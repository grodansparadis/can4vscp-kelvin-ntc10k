/* ******************************************************************************
 * 	VSCP (Very Simple Control Protocol) 
 * 	http://www.vscp.org
 *
 *  Kelvin NTC10KA Module
 *  =====================
 *
 *  Copyright (C) 2000-2020 Ake Hedman, Grodans Paradis AB
 *                          http://www.grodansparadis.com
 *                          <akhe@grodansparadis.com>
 *
 *  This work is licensed under the Creative Common 
 *  Attribution-NonCommercial-ShareAlike 3.0 Unported license. The full
 *  license is available in the top folder of this project (LICENSE) or here
 *  http://creativecommons.org/licenses/by-nc-sa/3.0/legalcode
 *  It is also available in a human readable form here 
 *  http://creativecommons.org/licenses/by-nc-sa/3.0/
 * 
 *	This file is part of VSCP - Very Simple Control Protocol 	
 *	http://www.vscp.org
 *
 * ******************************************************************************
 */


#include "vscp-compiler.h"
#include "vscp-projdefs.h"

#include <p18cxxx.h>
#include <timers.h>
#include <adc.h>
#include <math.h>
#include <vscp-inttypes.h>
#include <ECAN.h>
#include <vscp-firmware.h>
#include <vscp-class.h>
#include <vscp-type.h>
#include "main.h" 
#include "version.h"
#include "ntc.h"


#if defined(_18F2580) 


#if defined(RELEASE)

#pragma config WDT = ON, WDTPS = 128
#pragma config OSC = HSPLL
#pragma config BOREN = BOACTIVE
#pragma config STVREN = ON
#pragma config BORV = 0			// 4.6V
#pragma config LVP = ON
#pragma config CPB = ON
#pragma config BBSIZ = 2048
#pragma config WRTD  = OFF

#pragma config EBTR0 = OFF
#pragma config EBTR1 = OFF
#pragma config EBTR2 = OFF
#pragma config EBTR3 = OFF

#pragma config EBTRB = OFF

#else

#pragma config WDT = OFF
#pragma config OSC = HSPLL
#pragma config PWRT = ON
#pragma config BOREN = BOACTIVE
#pragma config STVREN = ON
#pragma config BORV = 0			// 4.6V
#pragma config LVP = OFF
#pragma config CPB = OFF
#pragma config WRTD  = OFF

#pragma config EBTR0 = OFF
#pragma config EBTR1 = OFF
#pragma config EBTR2 = OFF
#pragma config EBTR3 = OFF

#pragma config EBTRB = OFF

#endif

#else if defined(_18F25K80) || defined(_18F26K80) || defined(_18F45K80) || defined(_18F46K80) || defined(_18F65K80) || defined(_18F66K80)


// CONFIG1L
#pragma config SOSCSEL = DIG    // RC0/RC is I/O
#pragma config RETEN = OFF      // Ultra low-power regulator is Disabled (Controlled by REGSLP bit).
#pragma config INTOSCSEL = HIGH // LF-INTOSC in High-power mode during Sleep.
#pragma config XINST = OFF      // No extended instruction set

// CONFIG1H
#pragma config FOSC = HS2       // Crystal 10 MHz
#pragma config PLLCFG = ON      // 4 x PLL

// CONFIG2H
#pragma config WDTPS = 1048576  // Watchdog prescaler
#pragma config BOREN = SBORDIS  // Brown out enabled
#pragma config BORV  = 0        // 3V

// CONFIG3H
#pragma config CANMX = PORTB    // ECAN TX and RX pins are located on RB2 and RB3, respectively.
#pragma config MSSPMSK = MSK7   // 7 Bit address masking mode.
#pragma config MCLRE = ON       // MCLR Enabled, RE3 Disabled.

// CONFIG4L
#pragma config STVREN = ON      // Stack Overflow Reset enabled
#pragma config BBSIZ = BB2K     // Boot block size 2K

#ifdef DEBUG
#pragma config WDTEN = OFF      // WDT disabled in hardware; SWDTEN bit disabled.
#else
#pragma config WDTEN = ON       // WDT enabled in hardware; 
#endif


#endif


// The device URL (max 32 characters including null termination)
const uint8_t vscp_deviceURL[] = "www.eurosource.se/ntc10KA_3.xml";

// Global Variable Declarations
int16_t current_temp[6]; // Current temperature

uint8_t adc[NUMBER_OF_TEMP_SERIES * 12];    // Current ADC values
uint8_t adc_conversion_flags;               // Bits to flag new ADC values
uint8_t adc_series_counter;                 // Series counter

volatile uint16_t sendTimer;                // Timer for CAN send
volatile uint32_t measurement_clock;        // Clock for measurements
volatile uint32_t timeout_clock;            // Clock used for timeouts

uint8_t seconds;                // counter for seconds

uint8_t seconds_temp[6];        // timers for temp event

// Alarm flag bits
uint8_t low_alarm;
uint8_t high_alarm;

// Thermistor coefficients
double sh_coefficients[6*3];    // 6 sensors each with 3 32-bit constants

///////////////////////////////////////////////////////////////////////////////
// Isr() 	- Interrupt Service Routine
//      	- Services Timer0 Overflow
//      	- Services GP3 Pin Change
//////////////////////////////////////////////////////////////////////////////

void interrupt low_priority interrupt_at_low_vector( void )
{
    // Check timer
    if (INTCONbits.TMR0IF) { // If A Timer0 Interrupt, Then

        // Reload value for 1 ms resolution
        WriteTimer0(TIMER0_RELOAD_VALUE);

        vscp_timer++;
        vscp_configtimer++;
        measurement_clock++;
        timeout_clock++;
        sendTimer++;

        // Check for init. button
        if (!(PORTC & 0x01)) {
            // Active
            vscp_initbtncnt++;
        }
        else {
            vscp_initbtncnt = 0;
        }

        // Status LED
        vscp_statuscnt++;
        if ( ( VSCP_LED_BLINK1 ==
                 vscp_initledfunc) && (vscp_statuscnt > 100)) {
            if ( PORTC & 0x02 ) {
                PORTC &= ~0x02;
            }
            else {
                PORTC |= 0x02;
            }
            vscp_statuscnt = 0;
        }
        else if (VSCP_LED_ON == vscp_initledfunc) {
            PORTC |= 0x02;
            vscp_statuscnt = 0;
        }
        else if (VSCP_LED_OFF == vscp_initledfunc) {
            PORTC &= ~0x02;
            vscp_statuscnt = 0;
        }

        INTCONbits.TMR0IF = 0; // Clear Timer0 Interrupt Flag

    } // Timer & button

    // Check ADC
    if ( PIR1bits.ADIF ) {

#if defined(_18F2580)        
        switch (0x3C & ADCON0) {
#else
        switch (0x7C & ADCON0) {   
#endif            

            case SELECT_ADC_TEMP0:
                
                // Read conversion
                adc[(12 * adc_series_counter) + 0] = ADRESH;
                adc[(12 * adc_series_counter) + 1] = ADRESL;
                
                // Start new conversion
                ADCON0 = SELECT_ADC_TEMP1 + 1;

                // Mark that a new adc value is available if a full series
                // has been completed.
                if ((NUMBER_OF_TEMP_SERIES - 1) == adc_series_counter) {
                    adc_conversion_flags |= 1;
                }
                break;

            case SELECT_ADC_TEMP1:
                
                // Read conversion
                adc[(12 * adc_series_counter) + 2] = ADRESH;
                adc[(12 * adc_series_counter) + 3] = ADRESL;

                // Start new conversion
                ADCON0 = SELECT_ADC_TEMP2 + 1;

                // Mark that a new adc value is available if a full series
                // has been completed.
                if ((NUMBER_OF_TEMP_SERIES - 1) == adc_series_counter) {
                    adc_conversion_flags |= 1 << 1;
                }
                break;

            case SELECT_ADC_TEMP2:
                
                // Read conversion
                adc[(12 * adc_series_counter) + 4] = ADRESH;
                adc[(12 * adc_series_counter) + 5] = ADRESL;

                // Start new conversion
                ADCON0 = SELECT_ADC_TEMP3 + 1;

                // Mark that a new adc value is available if a full series
                // has been completed.
                if ((NUMBER_OF_TEMP_SERIES - 1) == adc_series_counter) {
                    adc_conversion_flags |= 1 << 2;
                }
                break;

            case SELECT_ADC_TEMP3:
                
                // Read conversion
                adc[(12 * adc_series_counter) + 6] = ADRESH;
                adc[(12 * adc_series_counter) + 7] = ADRESL;
                
                // Start new conversion
                ADCON0 = SELECT_ADC_TEMP4 + 1;

                // Mark that a new adc value is available if a full series
                // has been completed.
                if ((NUMBER_OF_TEMP_SERIES - 1) == adc_series_counter) {
                    adc_conversion_flags |= 1 << 3;
                }
                break;

            case SELECT_ADC_TEMP4:
                
                // Read conversion
                adc[(12 * adc_series_counter) + 8] = ADRESH;
                adc[(12 * adc_series_counter) + 9] = ADRESL;
                
                // Start new conversion
                ADCON0 = SELECT_ADC_TEMP5 + 1;

                // Mark that a new adc value is available if a full series
                // has been completed.
                if ((NUMBER_OF_TEMP_SERIES - 1) == adc_series_counter) {
                    adc_conversion_flags |= 1 << 4;
                }
                break;

            case SELECT_ADC_TEMP5:
                
                // Read conversion
                adc[(12 * adc_series_counter) + 10] = ADRESH;
                adc[(12 * adc_series_counter) + 11] = ADRESL;
                
                // Start new conversion
                ADCON0 = SELECT_ADC_TEMP0 + 1;

                // Mark that a new adc value is available if a full series
                // has been completed.
                if ((NUMBER_OF_TEMP_SERIES - 1) == adc_series_counter) {
                    adc_conversion_flags |= 1 << 5;
                }

                // Fill next series
                adc_series_counter++;
                if (adc_series_counter >= NUMBER_OF_TEMP_SERIES) {
                    adc_series_counter = 0;
                }
                break;

            default:
                
                // Start new conversion
                ADCON0 = SELECT_ADC_TEMP0 + 1;
                adc_series_counter = 0;
                break;
                
        }

        // Start conversion
        ConvertADC();

        PIR1bits.ADIF = 0; // Reset interrupt flag

    } // ADC

    return;
}


//***************************************************************************
// Main() - Main Routine
//***************************************************************************

void main()
{
    uint32_t i;
    
    init();
    
    // Check VSCP persistent storage and
    // restore if needed
    if ( !vscp_check_pstorage() ) {

        // Spoiled or not initialised - reinitialise
        init_app_eeprom();

    }

    vscp_init(); // Initialise the VSCP functionality

    while (TRUE) { // Loop Forever

        ClrWdt(); // Feed the dog

        if ( ( vscp_initbtncnt > 2500 ) &&
             ( VSCP_STATE_INIT != vscp_node_state ) ) {

            // Init. button pressed
            vscp_nickname = VSCP_ADDRESS_FREE;
            eeprom_write(VSCP_EEPROM_NICKNAME, VSCP_ADDRESS_FREE);
            vscp_init();

        }


        // Check for a valid  event
        vscp_imsg.flags = 0;
        vscp_getEvent();


        switch (vscp_node_state) {

            case VSCP_STATE_STARTUP: // Cold/warm reset

                // Get nickname from EEPROM
                if (VSCP_ADDRESS_FREE == vscp_nickname) {
                    // new on segment need a nickname
                    vscp_node_state = VSCP_STATE_INIT;
                }
                else {
                    // been here before - go on
                    vscp_node_state = VSCP_STATE_ACTIVE;
                    vscp_goActiveState();
                }
                break;

            case VSCP_STATE_INIT: // Assigning nickname
                vscp_handleProbeState();
                break;

            case VSCP_STATE_PREACTIVE: // Waiting for host initialization
                vscp_goActiveState();
                break;

            case VSCP_STATE_ACTIVE: // The normal state

                // Check for incoming message?
                if (vscp_imsg.flags & VSCP_VALID_MSG) {

                    if ( VSCP_CLASS1_PROTOCOL == vscp_imsg.vscp_class ) {

                        // Handle protocol event
                        vscp_handleProtocolEvent();

                    }
                    else if ( ( VSCP_CLASS1_CONTROL == vscp_imsg.vscp_class ) &&
                                ( VSCP_TYPE_CONTROL_SYNC == vscp_imsg.vscp_type ) ) {
                        handle_sync();
                    }

                }
                break;

            case VSCP_STATE_ERROR: // Everything is *very* *very* bad.
                vscp_error();
                break;

            default: // Should not be here...
                vscp_node_state = VSCP_STATE_STARTUP;
                break;

        }


        // do a measurement if needed
        if ( measurement_clock > 1000 ) {

            measurement_clock = 0;
            doOneSecondWork();
            seconds++;

            // Temperature report timers are only updated if in active
            // state
            if (VSCP_STATE_ACTIVE == vscp_node_state) {
                for (i = 0; i < 6; i++) {
                    seconds_temp[i]++;
                }
            }

            if (seconds > 59) {
                seconds = 0;
            }

            // Do VSCP one second jobs
            // Check if alarm events should be sent
            if (VSCP_STATE_ACTIVE == vscp_node_state) {
                vscp_doOneSecondWork();
            }

            // Also do some work
            doWork();

	}

    } // while
}


///////////////////////////////////////////////////////////////////////////////
// doWork
//
// The actual work is done here.
//

void doWork(void)
{
    uint8_t i, j;
    uint16_t B;
    double avarage;
    double resistance;
    double Rinf;
    double temp;
    double v;
    double calVoltage;

    calVoltage = (double)construct_unsigned16( eeprom_read(EEPROM_CALIBRATED_VOLTAGE_MSB),
                                        eeprom_read(EEPROM_CALIBRATED_VOLTAGE_LSB) ) / 10000.0;

    // Check if there are new adc values to
    // convert to temperatures

    for (i = 0; i < 6; i++) {

        if (adc_conversion_flags & 1 << i) {

            // Calculate mean value for this adc
            avarage = 0;
            for (j = 0; j < NUMBER_OF_TEMP_SERIES; j++) {
                avarage +=  construct_unsigned16( adc[12 * j + 2 * i], 
                                                    adc[12 * j + 2 * i + 1] );
            }
            avarage = avarage / NUMBER_OF_TEMP_SERIES;

            if (1) {

                // Use B-constant
                // ==============
                // http://en.wikipedia.org/wiki/Thermistor
                // R1 = (R2V - R2V2) / V2  R2= 10K, V = 5V,  V2 = adc * voltage/1024
                // T = B / ln(r/Rinf)
                // Rinf = R0 e (-B/T0), R0=10K, T0 = 273.15 + 25 = 298.15

                B = construct_unsigned16( eeprom_read(2 * i + EEPROM_B_CONSTANT0_MSB ),
                                        eeprom_read(2 * i + EEPROM_B_CONSTANT0_LSB) );

                
                Rinf = 10000.0 * exp(B/-298.15);
                
#if defined(_18F2580)   
                // V2 = adc * voltage/1024
                v = calVoltage * (double)avarage/1024;
#else  
                // V2 = adc * voltage/4096
                v = calVoltage * (double)avarage/4096;
#endif                
                // R1 = (R2V - R2V2) / V2  R2= 10K, V = 5V,  V2 = adc * voltage/1024
                resistance = ( 10000.0*(calVoltage - v) ) / v;
                
                //itemp = r;
                temp = ((double) B) / log(resistance / Rinf);
                //itemp = log(r/Rinf);
                temp -= 273.15; // Convert Kelvin to Celsius
                
                //avarage = testadc;
                /*  https://learn.adafruit.com/thermistor/using-a-thermistor
                avarage = (1023/avarage) - 1;
                avarage = 10000 / avarage;      // Resistance of termistor
                //temp = avarage/10000;           // (R/Ro)
                temp = 10000/avarage;
                temp = log(temp);               // ln(R/Ro)
                temp /= B;                      // 1/B * ln(R/Ro)
                temp += 1.0 / (25 + 273.15);    // + (1/To)
                temp = 1.0 / temp;              // Invert
                temp -= 273.15;
                */
                current_temp[ i ] = ( current_temp[ i ] + ( (long)(temp * 100) + getCalibrationValue( i ) ) ) / 2;

            }
            else {

                // Use S-H equation
                // ================

                // Assuming a 10k Thermistor.  Calculation is actually: Resistance = (1024/ADC)
#if defined(_18F2580)                 
                resistance = ((10240000 / adc[2 * i + 1]) - 10000);
#else 
                // Assuming a 10k Thermistor.  Calculation is actually: Resistance = (4096/ADC)
                resistance = ((40960000 / adc[2 * i + 1]) - 10000);     
#endif                

                /********************************************************************/
                /* Utilises the Steinhart-Hart Thermistor Equation:					*/
                /*    Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]^3}		*/
                /********************************************************************/
                temp = log(resistance);
                temp = 1 / (sh_coefficients[i * 3] + (sh_coefficients[i * 3 + 1] * temp) +
                        (sh_coefficients[i * 3 + 2] * temp * temp * temp));
                temp = temp - 273.15; // Convert Kelvin to Celsius
                current_temp[ i ] = (current_temp[ i ] + ((long) (temp * 100) + getCalibrationValue(i))) / 2;

            }

            // Reset flag
            adc_conversion_flags &= ~(1 << i);

        }
    }

}

///////////////////////////////////////////////////////////////////////////////
// doOneSecondWork
//

void doOneSecondWork(void) 
{
    uint8_t tmp;
    uint8_t i;
    int16_t setpoint;

    for (i = 0; i < 6; i++) {

        //*********************************************************************
        // Check if this is the lowest temperature ever
        //*********************************************************************
        
        if (current_temp[ i ] < construct_signed16( eeprom_read(EEPROM_ABSOLUT_LOW0_MSB + 2*i),
                                                    eeprom_read(EEPROM_ABSOLUT_LOW0_LSB + 2*i) ) ) {
            // Store new lowest value
            eeprom_write(EEPROM_ABSOLUT_LOW0_MSB + 2*i, ((uint16_t)current_temp[ i ]) >> 8);
            eeprom_write(EEPROM_ABSOLUT_LOW0_LSB + 2*i, ((uint16_t)current_temp[ i ]) & 0xff);
        }

        //*********************************************************************
        // Check if this is the highest temperature ever
        //*********************************************************************
        
        if (current_temp[ i ] > construct_signed16( eeprom_read(EEPROM_ABSOLUT_HIGH0_MSB + 2*i), 
                                                    eeprom_read(EEPROM_ABSOLUT_HIGH0_LSB + 2*i ) ) ) {
            // Store new lowest value
            eeprom_write(EEPROM_ABSOLUT_HIGH0_MSB + 2*i, ((uint16_t)current_temp[ i ]) >> 8);
            eeprom_write(EEPROM_ABSOLUT_HIGH0_LSB + 2*i,  ((uint16_t)current_temp[ i ]) & 0xff );
        }
        
        //*********************************************************************
        // Check if temperature report events should be sent
        //*********************************************************************
        tmp = eeprom_read(EEPROM_REPORT_INTERVAL0 + i);
        if (tmp && (seconds_temp[i] > tmp)) {

            // Send event
            if (sendTempEvent(i)) {
                seconds_temp[i] = 0;
            }

        }

        //*********************************************************************
        // Check for continuous alarm 
        //*********************************************************************
        if (MASK_CONTROL_CONTINUOUS & eeprom_read(EEPROM_CONTROLREG0 + i)) {

            // If low alarm active for sensor
            if (low_alarm & (1 << i)) {

                // Alarm must be enabled
                if (eeprom_read(EEPROM_CONTROLREG0 + i) & CONFIG_ENABLE_LOW_ALARM) {

                    vscp_omsg.priority = VSCP_PRIORITY_HIGH;
                    vscp_omsg.flags = VSCP_VALID_MSG + 3;

                    // Should ALARM or TURNON/TURNOFF events be sent
                    if (eeprom_read(EEPROM_CONTROLREG0 + i) & CONFIG_ENABLE_TURNX) {

                        if (eeprom_read(EEPROM_CONTROLREG0 + i) & CONFIG_ENABLE_TURNON_INVERT) {
                            vscp_omsg.vscp_class = VSCP_CLASS1_CONTROL;
                            vscp_omsg.vscp_type = VSCP_TYPE_CONTROL_TURNON;
                        } 
                        else {
                            vscp_omsg.vscp_class = VSCP_CLASS1_CONTROL;
                            vscp_omsg.vscp_type = VSCP_TYPE_CONTROL_TURNOFF;
                        }

                    } 
                    else {
                        // Alarm event should be sent
                        vscp_omsg.vscp_class = VSCP_CLASS1_ALARM;
                        vscp_omsg.vscp_type = VSCP_TYPE_ALARM_ALARM;
                    }

                    vscp_omsg.data[ 0 ] = i; // Index = sensor
                    vscp_omsg.data[ 1 ] =
                            eeprom_read(EEPROM_SENSOR0_ZONE + 2*i);      // Zone
                    vscp_omsg.data[ 2 ] =
                            eeprom_read(EEPROM_SENSOR0_SUBZONE + 2*i);   // Subzone

                    // Send event
                    // We allow for missing to send this event
                    // as it will be sent next second instead.
                    vscp_sendEvent();

                }
            }

            // If  high alarm active for sensor
            if (high_alarm & (1 << i)) {

                // Should ALARM or TURNON/TURNOFF events be sent
                if ((eeprom_read(EEPROM_CONTROLREG0 + i) & CONFIG_ENABLE_HIGH_ALARM)) {

                    vscp_omsg.priority = VSCP_PRIORITY_HIGH;
                    vscp_omsg.flags = VSCP_VALID_MSG + 3;

                    if (eeprom_read(EEPROM_CONTROLREG0 + i) & CONFIG_ENABLE_TURNX) {
                        
                        if (eeprom_read(EEPROM_CONTROLREG0 + i) & CONFIG_ENABLE_TURNON_INVERT) {
                            vscp_omsg.vscp_class = VSCP_CLASS1_CONTROL;
                            vscp_omsg.vscp_type = VSCP_TYPE_CONTROL_TURNOFF;
                        } 
                        else {
                            vscp_omsg.vscp_class = VSCP_CLASS1_CONTROL;
                            vscp_omsg.vscp_type = VSCP_TYPE_CONTROL_TURNON;
                        }
                    } 
                    else {
                        // Alarm event should be sent
                        vscp_omsg.vscp_class = VSCP_CLASS1_ALARM;
                        vscp_omsg.vscp_type = VSCP_TYPE_ALARM_ALARM;
                    }

                    vscp_omsg.data[ 0 ] = i;                            // Index = sensor
                    vscp_omsg.data[ 1 ] = 
                            eeprom_read(EEPROM_SENSOR0_ZONE + 2*i);      // Zone
                    vscp_omsg.data[ 2 ] = 
                            eeprom_read(EEPROM_SENSOR0_SUBZONE + 2*i);   // Sub zone

                    // Send event
                    // We allow for missing to send this event
                    // as it will be sent next second instead.
                    vscp_sendEvent();
                }
            }
        }       
        
        
        //*********************************************************************
        // Check if we have a low alarm condition 
        //*********************************************************************
        if (low_alarm & (1 << i)) {

            // We have an alarm condition already
            setpoint = construct_signed16(eeprom_read(EEPROM_LOW_ALARM0_MSB + 2*i),
                                            eeprom_read(EEPROM_LOW_ALARM0_LSB + 2*i)) +
                        (int8_t) eeprom_read(EEPROM_HYSTERESIS_SENSOR0 + i);

            // Check if it is no longer valid
            // that is under hysteresis so we can rest
            // alarm condition
            if (current_temp[ i ] > (setpoint * 100)) {

                // Reset alarm condition
                low_alarm &= ~(1 << i);

            }

        } 
        else {

            // We do not have a low alarm condition already
            // check if we should have
            setpoint = construct_signed16( eeprom_read(EEPROM_LOW_ALARM0_MSB + 2*i ),
                                            eeprom_read(EEPROM_LOW_ALARM0_LSB + 2*i ) );

            if (current_temp[ i ] < (setpoint * 100)) {

                // We have a low alarm condition
                low_alarm |= (1 << i);

                // Set module alarm flag
                // Note that this bit is set even if we are unable
                // to send an alarm event.
                vscp_alarmstatus |= MODULE_LOW_ALARM;

                // Should ALARM events be sent
                if ( eeprom_read(i + EEPROM_CONTROLREG0) & CONFIG_ENABLE_LOW_ALARM ) {

                    vscp_omsg.priority = VSCP_PRIORITY_HIGH;
                    vscp_omsg.flags = VSCP_VALID_MSG + 3;

                    // Should TurnOn/TurnOff events be sent
                    if (eeprom_read(EEPROM_CONTROLREG0 + i) & CONFIG_ENABLE_TURNX) {

                        if (eeprom_read(EEPROM_CONTROLREG0 + i) & CONFIG_ENABLE_TURNON_INVERT) {
                            vscp_omsg.vscp_class = VSCP_CLASS1_CONTROL;
                            vscp_omsg.vscp_type = VSCP_TYPE_CONTROL_TURNON;
                        } 
                        else {
                            vscp_omsg.vscp_class = VSCP_CLASS1_CONTROL;
                            vscp_omsg.vscp_type = VSCP_TYPE_CONTROL_TURNOFF;
                        }

                    }
                    else {
                        vscp_omsg.vscp_class = VSCP_CLASS1_ALARM;
                        vscp_omsg.vscp_type = VSCP_TYPE_ALARM_ALARM;
                    }

                    vscp_omsg.data[ 0 ] = i; // Index
                    vscp_omsg.data[ 1 ] = eeprom_read(EEPROM_SENSOR0_ZONE + 2 * i);    // Zone
                    vscp_omsg.data[ 2 ] = eeprom_read(EEPROM_SENSOR0_SUBZONE + 2 * i); // Sub zone

                    // Send event
                    if (!vscp_sendEvent()) {
                        // Could not send alarm event
                        // Reset alarm - we try again next round
                        low_alarm &= ~(1 << i);
                    }

                }
            }
        }

        //*********************************************************************
        // Check if we have a high alarm condition 
        //*********************************************************************
        if (high_alarm & (1 << i)) {

            // We have an alarm condition already

            setpoint = construct_signed16(eeprom_read(EEPROM_HIGH_ALARM0_MSB + 2 * i),
                                            eeprom_read(EEPROM_HIGH_ALARM0_LSB + 2 * i)) -
                            (int8_t)eeprom_read(EEPROM_HYSTERESIS_SENSOR0 + i);

            // Under hysteresis so we can reset condition
            if (current_temp[ i ] < (setpoint * 100) ) {

                // Reset alarm
                high_alarm &= ~(1 << i);

            }

        } 
        else {

            // We do not have an alarm condition
            // check for one

            setpoint = construct_signed16(eeprom_read(EEPROM_HIGH_ALARM0_MSB + 2 * i),
                                            eeprom_read(EEPROM_HIGH_ALARM0_LSB + 2 * i));

            if (current_temp[ i ] > (setpoint * 100)) {

                // We have a low alarm condition
                high_alarm |= (1 << i);

                // Set module alarm flag
                // Note that this bit is set even if we are unable
                // to send an alarm event.

                vscp_alarmstatus |= MODULE_HIGH_ALARM;

                // Should ALARM or TURNON/TURNOFF events be sent
                if (eeprom_read(EEPROM_CONTROLREG0 + i) & CONFIG_ENABLE_HIGH_ALARM) {

                    vscp_omsg.priority = VSCP_PRIORITY_HIGH;
                    vscp_omsg.flags = VSCP_VALID_MSG + 3;

                    if (eeprom_read(EEPROM_CONTROLREG0 + i) & CONFIG_ENABLE_TURNX) {

                        if (eeprom_read(EEPROM_CONTROLREG0 + i) & CONFIG_ENABLE_TURNON_INVERT) {
                            vscp_omsg.vscp_class = VSCP_CLASS1_CONTROL;
                            vscp_omsg.vscp_type = VSCP_TYPE_CONTROL_TURNOFF;
                        } 
                        else {
                            vscp_omsg.vscp_class = VSCP_CLASS1_CONTROL;
                            vscp_omsg.vscp_type = VSCP_TYPE_CONTROL_TURNON;
                        }
                    }
                    else {
                        vscp_omsg.vscp_class = VSCP_CLASS1_ALARM;
                        vscp_omsg.vscp_type = VSCP_TYPE_ALARM_ALARM;
                    }

                    vscp_omsg.data[ 0 ] = i; // Index
                    vscp_omsg.data[ 1 ] = eeprom_read(EEPROM_SENSOR0_ZONE + 2 * i);      // Zone
                    vscp_omsg.data[ 2 ] = eeprom_read(EEPROM_SENSOR0_SUBZONE + 2 * i);   // Sub zone

                    // Send event
                    if (!vscp_sendEvent()) {
                        // Could not send alarm event
                        // Reset alarm - we try again next round
                        high_alarm &= ~(1 << i);
                    }

                }
            }
        }
        
    }
}

///////////////////////////////////////////////////////////////////////////////
// sendTempEvent
//

int8_t sendTempEvent(uint8_t i)
{
    vscp_omsg.priority = VSCP_PRIORITY_MEDIUM;
    vscp_omsg.flags = VSCP_VALID_MSG + 4;
    vscp_omsg.vscp_class = VSCP_CLASS1_MEASUREMENT;
    vscp_omsg.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // Data format
    vscp_omsg.data[ 0 ] = 0x80 | // Normalized integer
            ((0x03 & eeprom_read(i + EEPROM_CONTROLREG0)) << 3) | // Unit
            i; // Sensor
    // Exponent 
    vscp_omsg.data[ 1 ] = 0x82;

    setEventData( current_temp[i],
            ( 0x03 & eeprom_read(i + EEPROM_CONTROLREG0 ) ) );

    // Send event
    if (!vscp_sendEvent()) {
        return FALSE;
    }

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// setEventData
//

void setEventData(int v, unsigned char unit)
{
    double newval;
    int ival;

    if (TEMP_UNIT_KELVIN == unit) {
        // Convert to Kelvin
        newval = Celsius2Kelvin(v);
    } else if (TEMP_UNIT_FAHRENHEIT == unit) {
        // Convert to Fahrenheit
        newval = Celsius2Fahrenheit(v);
    } else {
        // Defaults to Celsius
        newval = v;
    }

    ival = (int) newval;

    vscp_omsg.data[ 2 ] = ((ival & 0xff00) >> 8);
    vscp_omsg.data[ 3 ] = (ival & 0xff);
}

///////////////////////////////////////////////////////////////////////////////
// getCalibrationValue
//
// Get the calibration value for a specific sensor.
//

int16_t getCalibrationValue(uint8_t i)
{
    int16_t cal;

    cal = construct_signed16( eeprom_read(2 * i + EEPROM_CALIBRATION_SENSOR0_MSB),
				eeprom_read(2 * i + EEPROM_CALIBRATION_SENSOR0_LSB) );

    return cal;
}

///////////////////////////////////////////////////////////////////////////////
// Init - Initialization Routine
//  

void init()
{
    // Initialize data
    init_app_ram();

    // Initialize the uP

    // PORTA
    // RA0/AN0 - input
    // RA1/AN1 - input
    // RA2/AN2 - input
    // RA4 input (VCAP for PIC18F26K80))
    TRISA = 0b00010111;

    // PortB
    // RB7 - Not used.
    // RB& - Not used.
    // RB5 - Not used.
    // RB4/AN9 - input
    // RB3 CAN RX - input
    // RB2 CAN TX - output
    // RB1/AN8 - input
    // RB0/AN10 -input
    TRISB = 0b00011011; // 0x1B;

    // RC7 - Output - Not used.
    // RC6 - Output - Not used.
    // RC5 - Output - Not used.
    // RC3 - Output - Not used.
    // RC2 - Output - Not used.
    // RC1 - Output - Status LED - Default off
    // RC0 - Input  - Init. button

    TRISC = 0b00000001;
    PORTC = 0x00;
    
    OpenTimer0(TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_8);
    WriteTimer0(TIMER0_RELOAD_VALUE);

#if defined(_18F2580)     
    OpenADC(ADC_FOSC_32 & ADC_RIGHT_JUST & ADC_20_TAD,
                ADC_CH0 & ADC_INT_ON & ADC_11ANA &
                    ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS,
                15 );
#else if defined(_18F25K80) || defined(_18F26K80) || defined(_18F45K80) || defined(_18F46K80) || defined(_18F65K80) || defined(_18F66K80)
        
    // Disable comparators
    CM1CON = 0x00;
    CM2CON = 0x00;
    CVRCON = 0; 
    
    ANCON0bits.ANSEL0=1;
    ANCON0bits.ANSEL1=1;
    ANCON0bits.ANSEL2=1;
    ANCON1bits.ANSEL8=1;
    ANCON1bits.ANSEL9=1;
    ANCON1bits.ANSEL10=1;

    // OpenADC_Page16
    OpenADC( ADC_FOSC_32 & ADC_RIGHT_JUST & ADC_20_TAD,
                ADC_CH0 & ADC_INT_ON,
                ADC_NEG_CH0 & ADC_REF_VDD_VDD & ADC_REF_VDD_VSS );
#endif    
   

    // Initialize CAN
    ECANInitialize();

/* 
    {
    uint8_t msgdata[ 8 ];
    
    // Must be in Config. mode to change many of settings.
    //ECANSetOperationMode(ECAN_OP_MODE_CONFIG);

    // Return to Normal mode to communicate.
    //ECANSetOperationMode(ECAN_OP_MODE_NORMAL);

    msgdata[ 0 ] = 1;
    msgdata[ 1 ] = 2;
    msgdata[ 2 ] = 3;
    msgdata[ 3 ] = 4;

    sendCANFrame( 12345, 4, msgdata );
    }
*/

    // Enable global interrupt
    INTCONbits.GIE = 1;

    // Start ADC conversions
    ConvertADC();

    return;
}

///////////////////////////////////////////////////////////////////////////////
// writeCoeffs2Ram
//

void writeCoeffs2Ram(void)
{
    int i, j;
    uint8_t c[3];

    for (i = 0; i < 6; i++) {
        for (j = 2; j > 0; j--) {
            // Store Microchip order (Little endian)
            c[2 - j] = eeprom_read(EEPROM_COEFFICIENT_A_SENSOR0_0 + i * 3 + j);
        }
        sh_coefficients[i] = *((double*) c);
    }
}

///////////////////////////////////////////////////////////////////////////////
// init._app._ram
//

void init_app_ram(void)
{
    char i;

    measurement_clock = 0; // start a new measurement cycle
    seconds = 0;

    // no temp. reads yet
    for (i = 0; i < 6; i++) {
        seconds_temp[i] = 0;
        current_temp[i] = 0;
    }

    // No adc conversions done
    adc_conversion_flags = 0;

    // Start with series 0
    adc_series_counter = 0;

    // No low temperature alarms
    low_alarm = 0;

    // No high temperature alarms
    high_alarm = 0;

    // Write coefficients to ram
    //writeCoeffs2Ram();
}



///////////////////////////////////////////////////////////////////////////////
// init_app_eeprom
//

void init_app_eeprom(void)
{
    // Zone sub zone
    eeprom_write( EEPROM_ZONE, 0 );
    eeprom_write( EEPROM_ZONE, 0 );
    
    eeprom_write(EEPROM_CONTROLREG0, DEFAULT_CONTROL_REG);
    eeprom_write(EEPROM_CONTROLREG1, DEFAULT_CONTROL_REG);
    eeprom_write(EEPROM_CONTROLREG2, DEFAULT_CONTROL_REG);
    eeprom_write(EEPROM_CONTROLREG3, DEFAULT_CONTROL_REG);
    eeprom_write(EEPROM_CONTROLREG4, DEFAULT_CONTROL_REG);
    eeprom_write(EEPROM_CONTROLREG5, DEFAULT_CONTROL_REG);

    eeprom_write(EEPROM_REPORT_INTERVAL0, DEFAULT_REPORT_INTERVAL_SENSOR0);
    eeprom_write(EEPROM_REPORT_INTERVAL1, DEFAULT_REPORT_INTERVAL);
    eeprom_write(EEPROM_REPORT_INTERVAL2, DEFAULT_REPORT_INTERVAL);
    eeprom_write(EEPROM_REPORT_INTERVAL3, DEFAULT_REPORT_INTERVAL);
    eeprom_write(EEPROM_REPORT_INTERVAL4, DEFAULT_REPORT_INTERVAL);
    eeprom_write(EEPROM_REPORT_INTERVAL5, DEFAULT_REPORT_INTERVAL);

    // B Constants

    eeprom_write(EEPROM_B_CONSTANT0_MSB, DEFAULT_B_CONSTANT_SENSOR0_MSB);
    eeprom_write(EEPROM_B_CONSTANT0_LSB, DEFAULT_B_CONSTANT_SENSOR0_LSB);

    eeprom_write(EEPROM_B_CONSTANT1_MSB, DEFAULT_B_CONSTANT_MSB);
    eeprom_write(EEPROM_B_CONSTANT1_LSB, DEFAULT_B_CONSTANT_LSB);

    eeprom_write(EEPROM_B_CONSTANT2_MSB, DEFAULT_B_CONSTANT_MSB);
    eeprom_write(EEPROM_B_CONSTANT2_LSB, DEFAULT_B_CONSTANT_LSB);

    eeprom_write(EEPROM_B_CONSTANT3_MSB, DEFAULT_B_CONSTANT_MSB);
    eeprom_write(EEPROM_B_CONSTANT3_LSB, DEFAULT_B_CONSTANT_LSB);

    eeprom_write(EEPROM_B_CONSTANT4_MSB, DEFAULT_B_CONSTANT_MSB);
    eeprom_write(EEPROM_B_CONSTANT4_LSB, DEFAULT_B_CONSTANT_LSB);

    eeprom_write(EEPROM_B_CONSTANT5_MSB, DEFAULT_B_CONSTANT_MSB);
    eeprom_write(EEPROM_B_CONSTANT5_LSB, DEFAULT_B_CONSTANT_LSB);

    // Low alarms

    eeprom_write(EEPROM_LOW_ALARM0_MSB, DEFAULT_LOW_ALARM_MSB);
    eeprom_write(EEPROM_LOW_ALARM0_LSB, DEFAULT_LOW_ALARM_LSB);

    eeprom_write(EEPROM_LOW_ALARM1_MSB, DEFAULT_LOW_ALARM_MSB);
    eeprom_write(EEPROM_LOW_ALARM1_LSB, DEFAULT_LOW_ALARM_LSB);

    eeprom_write(EEPROM_LOW_ALARM2_MSB, DEFAULT_LOW_ALARM_MSB);
    eeprom_write(EEPROM_LOW_ALARM2_LSB, DEFAULT_LOW_ALARM_LSB);

    eeprom_write(EEPROM_LOW_ALARM3_MSB, DEFAULT_LOW_ALARM_MSB);
    eeprom_write(EEPROM_LOW_ALARM3_LSB, DEFAULT_LOW_ALARM_LSB);

    eeprom_write(EEPROM_LOW_ALARM4_MSB, DEFAULT_LOW_ALARM_MSB);
    eeprom_write(EEPROM_LOW_ALARM4_LSB, DEFAULT_LOW_ALARM_LSB);

    eeprom_write(EEPROM_LOW_ALARM5_MSB, DEFAULT_LOW_ALARM_MSB);
    eeprom_write(EEPROM_LOW_ALARM5_LSB, DEFAULT_LOW_ALARM_LSB);

    // High alarms

    eeprom_write(EEPROM_HIGH_ALARM0_MSB, DEFAULT_HIGH_ALARM_MSB);
    eeprom_write(EEPROM_HIGH_ALARM0_LSB, DEFAULT_HIGH_ALARM_LSB);

    eeprom_write(EEPROM_HIGH_ALARM1_MSB, DEFAULT_HIGH_ALARM_MSB);
    eeprom_write(EEPROM_HIGH_ALARM1_LSB, DEFAULT_HIGH_ALARM_LSB);

    eeprom_write(EEPROM_HIGH_ALARM2_MSB, DEFAULT_HIGH_ALARM_MSB);
    eeprom_write(EEPROM_HIGH_ALARM2_LSB, DEFAULT_HIGH_ALARM_LSB);

    eeprom_write(EEPROM_HIGH_ALARM3_MSB, DEFAULT_HIGH_ALARM_MSB);
    eeprom_write(EEPROM_HIGH_ALARM3_LSB, DEFAULT_HIGH_ALARM_LSB);

    eeprom_write(EEPROM_HIGH_ALARM4_MSB, DEFAULT_HIGH_ALARM_MSB);
    eeprom_write(EEPROM_HIGH_ALARM4_LSB, DEFAULT_HIGH_ALARM_LSB);

    eeprom_write(EEPROM_HIGH_ALARM5_MSB, DEFAULT_HIGH_ALARM_MSB);
    eeprom_write(EEPROM_HIGH_ALARM5_LSB, DEFAULT_HIGH_ALARM_LSB);

    // Sensor zone/subzone information

    eeprom_write(EEPROM_SENSOR0_ZONE, DEFAULT_SENSOR_ZONE);
    eeprom_write(EEPROM_SENSOR0_SUBZONE, DEFAULT_SENSOR_SUBZONE);

    eeprom_write(EEPROM_SENSOR1_ZONE, DEFAULT_SENSOR_ZONE);
    eeprom_write(EEPROM_SENSOR1_SUBZONE, DEFAULT_SENSOR_SUBZONE);

    eeprom_write(EEPROM_SENSOR2_ZONE, DEFAULT_SENSOR_ZONE);
    eeprom_write(EEPROM_SENSOR2_SUBZONE, DEFAULT_SENSOR_SUBZONE);

    eeprom_write(EEPROM_SENSOR3_ZONE, DEFAULT_SENSOR_ZONE);
    eeprom_write(EEPROM_SENSOR3_SUBZONE, DEFAULT_SENSOR_SUBZONE);

    eeprom_write(EEPROM_SENSOR4_ZONE, DEFAULT_SENSOR_ZONE);
    eeprom_write(EEPROM_SENSOR4_SUBZONE, DEFAULT_SENSOR_SUBZONE);

    eeprom_write(EEPROM_SENSOR5_ZONE, DEFAULT_SENSOR_ZONE);
    eeprom_write(EEPROM_SENSOR5_SUBZONE, DEFAULT_SENSOR_SUBZONE);

    // Absolut low temperatures

    eeprom_write(EEPROM_ABSOLUT_LOW0_MSB, DEFAULT_LOW_MSB);
    eeprom_write(EEPROM_ABSOLUT_LOW0_LSB, DEFAULT_LOW_LSB);

    eeprom_write(EEPROM_ABSOLUT_LOW1_MSB, DEFAULT_LOW_MSB);
    eeprom_write(EEPROM_ABSOLUT_LOW1_LSB, DEFAULT_LOW_LSB);

    eeprom_write(EEPROM_ABSOLUT_LOW2_MSB, DEFAULT_LOW_MSB);
    eeprom_write(EEPROM_ABSOLUT_LOW2_LSB, DEFAULT_LOW_LSB);

    eeprom_write(EEPROM_ABSOLUT_LOW3_MSB, DEFAULT_LOW_MSB);
    eeprom_write(EEPROM_ABSOLUT_LOW3_LSB, DEFAULT_LOW_LSB);

    eeprom_write(EEPROM_ABSOLUT_LOW4_MSB, DEFAULT_LOW_MSB);
    eeprom_write(EEPROM_ABSOLUT_LOW4_LSB, DEFAULT_LOW_LSB);

    eeprom_write(EEPROM_ABSOLUT_LOW5_MSB, DEFAULT_LOW_MSB);
    eeprom_write(EEPROM_ABSOLUT_LOW5_LSB, DEFAULT_LOW_LSB);

    // Absolute high temperatures

    eeprom_write(EEPROM_ABSOLUT_HIGH0_MSB, DEFAULT_HIGH_MSB);
    eeprom_write(EEPROM_ABSOLUT_HIGH0_LSB, DEFAULT_HIGH_LSB);

    eeprom_write(EEPROM_ABSOLUT_HIGH1_MSB, DEFAULT_HIGH_MSB);
    eeprom_write(EEPROM_ABSOLUT_HIGH1_LSB, DEFAULT_HIGH_LSB);

    eeprom_write(EEPROM_ABSOLUT_HIGH2_MSB, DEFAULT_HIGH_MSB);
    eeprom_write(EEPROM_ABSOLUT_HIGH2_LSB, DEFAULT_HIGH_LSB);

    eeprom_write(EEPROM_ABSOLUT_HIGH3_MSB, DEFAULT_HIGH_MSB);
    eeprom_write(EEPROM_ABSOLUT_HIGH3_LSB, DEFAULT_HIGH_LSB);

    eeprom_write(EEPROM_ABSOLUT_HIGH4_MSB, DEFAULT_HIGH_MSB);
    eeprom_write(EEPROM_ABSOLUT_HIGH4_LSB, DEFAULT_HIGH_LSB);

    eeprom_write(EEPROM_ABSOLUT_HIGH5_MSB, DEFAULT_HIGH_MSB);
    eeprom_write(EEPROM_ABSOLUT_HIGH5_LSB, DEFAULT_HIGH_LSB);

    // Sensor hysteresis

    eeprom_write(EEPROM_HYSTERESIS_SENSOR0, DEFAULT_HYSTERESIS);
    eeprom_write(EEPROM_HYSTERESIS_SENSOR1, DEFAULT_HYSTERESIS);
    eeprom_write(EEPROM_HYSTERESIS_SENSOR2, DEFAULT_HYSTERESIS);
    eeprom_write(EEPROM_HYSTERESIS_SENSOR3, DEFAULT_HYSTERESIS);
    eeprom_write(EEPROM_HYSTERESIS_SENSOR4, DEFAULT_HYSTERESIS);
    eeprom_write(EEPROM_HYSTERESIS_SENSOR5, DEFAULT_HYSTERESIS);

    // Calibrated voltage
    
    eeprom_write(EEPROM_CALIBRATED_VOLTAGE_MSB, DEFAULT_CALIBRATED_VOLTAGE_MSB);
    eeprom_write(EEPROM_CALIBRATED_VOLTAGE_LSB, DEFAULT_CALIBRATED_VOLTAGE_LSB);

    // Calibration

    eeprom_write(EEPROM_CALIBRATION_SENSOR0_MSB, 0);
    eeprom_write(EEPROM_CALIBRATION_SENSOR0_LSB, 0);
    eeprom_write(EEPROM_CALIBRATION_SENSOR1_MSB, 0);
    eeprom_write(EEPROM_CALIBRATION_SENSOR1_LSB, 0);
    eeprom_write(EEPROM_CALIBRATION_SENSOR2_MSB, 0);
    eeprom_write(EEPROM_CALIBRATION_SENSOR2_LSB, 0);
    eeprom_write(EEPROM_CALIBRATION_SENSOR3_MSB, 0);
    eeprom_write(EEPROM_CALIBRATION_SENSOR3_LSB, 0);
    eeprom_write(EEPROM_CALIBRATION_SENSOR4_MSB, 0);
    eeprom_write(EEPROM_CALIBRATION_SENSOR4_LSB, 0);
    eeprom_write(EEPROM_CALIBRATION_SENSOR5_MSB, 0);
    eeprom_write(EEPROM_CALIBRATION_SENSOR5_LSB, 0);

    // S-H Coefficients sensor 0

    /*
    eeprom_write(EEPROM_COEFFICIENT_A_SENSOR0_0, 0);
    eeprom_write(EEPROM_COEFFICIENT_A_SENSOR0_1, 0);
    eeprom_write(EEPROM_COEFFICIENT_A_SENSOR0_2, 0);
    eeprom_write(EEPROM_COEFFICIENT_A_SENSOR0_3, 0);
    eeprom_write(EEPROM_COEFFICIENT_B_SENSOR0_0, 0);
    eeprom_write(EEPROM_COEFFICIENT_B_SENSOR0_1, 0);
    eeprom_write(EEPROM_COEFFICIENT_B_SENSOR0_2, 0);
    eeprom_write(EEPROM_COEFFICIENT_B_SENSOR0_3, 0);
    eeprom_write(EEPROM_COEFFICIENT_C_SENSOR0_0, 0);
    eeprom_write(EEPROM_COEFFICIENT_C_SENSOR0_1, 0);
    eeprom_write(EEPROM_COEFFICIENT_C_SENSOR0_2, 0);
    eeprom_write(EEPROM_COEFFICIENT_C_SENSOR0_3, 0);

    // S-H Coefficients sensor 1-5
    
    for (uint8_t i = 0; i < 6; i++) {
        eeprom_write(EEPROM_COEFFICIENT_A_SENSOR1_0 + i * 12, 0);
        eeprom_write(EEPROM_COEFFICIENT_A_SENSOR1_1 + i * 12, 0);
        eeprom_write(EEPROM_COEFFICIENT_A_SENSOR1_2 + i * 12, 0);
        eeprom_write(EEPROM_COEFFICIENT_A_SENSOR1_3 + i * 12, 0);
        eeprom_write(EEPROM_COEFFICIENT_B_SENSOR1_0 + i * 12, 0);
        eeprom_write(EEPROM_COEFFICIENT_B_SENSOR1_1 + i * 12, 0);
        eeprom_write(EEPROM_COEFFICIENT_B_SENSOR1_2 + i * 12, 0);
        eeprom_write(EEPROM_COEFFICIENT_B_SENSOR1_3 + i * 12, 0);
        eeprom_write(EEPROM_COEFFICIENT_C_SENSOR1_0 + i * 12, 0);
        eeprom_write(EEPROM_COEFFICIENT_C_SENSOR1_1 + i * 12, 0);
        eeprom_write(EEPROM_COEFFICIENT_C_SENSOR1_2 + i * 12, 0);
        eeprom_write(EEPROM_COEFFICIENT_C_SENSOR1_3 + i * 12, 0);
    }
    */
    
}


///////////////////////////////////////////////////////////////////////////////
// convertTemperature
//

int8_t convertTemperature(int temp, unsigned char unit)
{
    int newval;

    if (TEMP_UNIT_KELVIN == unit) {
        // Convert to Kelvin
        newval = (int) Celsius2Kelvin(temp);
    } 
    else if (TEMP_UNIT_FAHRENHEIT == unit) {
        // Convert to Fahrenheit
        newval = (int) Celsius2Fahrenheit(temp);
    } 
    else {
        // Defaults to Celsius
        newval = (int) temp;
    }

    return newval;
}

///////////////////////////////////////////////////////////////////////////////
// handle_sync
//
// Report all temperature values
//

void handle_sync(void)
{
    uint8_t i;

    for (i = 0; i < 6; i++) {

        if ( ( ( 0xff == vscp_imsg.data[ 1 ] ) ||
                ( eeprom_read( EEPROM_SENSOR0_ZONE + 2*i ) == vscp_imsg.data[ 1 ] ) ) &&
                ( ( 0xff == vscp_imsg.data[ 2 ] ) ||
                ( eeprom_read( EEPROM_SENSOR0_SUBZONE + 2*i ) == vscp_imsg.data[ 2 ] ) ) ) {

            // We have a one second timeout
            timeout_clock = 0;
            while (!sendTempEvent(i)) {
                if (timeout_clock > 1000) break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// vscp_readAppReg
//

uint8_t vscp_readAppReg(unsigned char reg)
{
    uint8_t rv;
    //int tmpval;
    //uint8_t val, checksum;
    //uint8_t unit = (0x03 & eeprom_read(EEPROM_CONTROLREG0));

    if (0 == vscp_page_select) {

        switch (reg) {

            // Zone
            case 0x00:
                rv = eeprom_read( EEPROM_ZONE );
                break;

            // Sub zone
            case 0x01:
                rv = eeprom_read( EEPROM_SUBZONE );
                break;

            // Control register for sensor 0
            case 0x02:
                rv = eeprom_read( EEPROM_CONTROLREG0 );
                break;

            // Control register for sensor 1
            case 0x03:
                rv = eeprom_read (EEPROM_CONTROLREG1 );
                break;

            // Control register for sensor 2
            case 0x04:
                rv = eeprom_read( EEPROM_CONTROLREG2 );
                break;

            // Control register for sensor 3
            case 0x05:
                rv = eeprom_read( EEPROM_CONTROLREG3 );
                break;

            // Control register for sensor 4
            case 0x06:
                rv = eeprom_read( EEPROM_CONTROLREG4 );
                break;

            // Control register for sensor 5
            case 0x07:
                rv = eeprom_read( EEPROM_CONTROLREG5 );
                break;

            // MSB of current temperature for sensor 0
            case 0x08:
                rv = ((current_temp[0] & 0xff00) >> 8);
                break;

            // LSB of current temperature for sensor 0
            case 0x09:
                rv = (current_temp[0] & 0x00ff);
                break;

            // MSB of current temperature for sensor 1
            case 0x0A:
                rv = ((current_temp[1] & 0xff00) >> 8);
                break;

            // LSB of current temperature for sensor 1
            case 0x0B:
                rv = (current_temp[1] & 0x00ff);
                break;

            // MSB of current temperature for sensor 2
            case 0x0C:
                rv = ((current_temp[2] & 0xff00) >> 8);
                break;

            // LSB of current temperature for sensor 2
            case 0x0D:
                rv = (current_temp[2] & 0x00ff);
                break;

            // MSB of current temperature for sensor 3
            case 0x0E:
                rv = ((current_temp[3] & 0xff00) >> 8);
                break;

            // LSB of current temperature for sensor 3
            case 0x0F:
                rv = (current_temp[3] & 0x00ff);
                break;

            // MSB of current temperature for sensor 4
            case 0x10:
                rv = ((current_temp[4] & 0xff00) >> 8);
                break;

            // LSB of current temperature for sensor 4
            case 0x11:
                rv = (current_temp[4] & 0x00ff);
                break;

            // MSB of current temperature for sensor 4
            case 0x12:
                rv = ((current_temp[5] & 0xff00) >> 8);
                break;

            // LSB of current temperature for sensor 4
            case 0x13:
                rv = (current_temp[5] & 0x00ff);
                break;

            // Report interval register for sensor 0
            case 0x14:
                rv = eeprom_read(EEPROM_REPORT_INTERVAL0);
                break;

            // Report interval register for sensor 1
            case 0x15:
                rv = eeprom_read(EEPROM_REPORT_INTERVAL1);
                break;

            // Report interval register for sensor 2
            case 0x16:
                rv = eeprom_read(EEPROM_REPORT_INTERVAL2);
                break;

            // Report interval register for sensor 3
            case 0x17:
                rv = eeprom_read(EEPROM_REPORT_INTERVAL3);
                break;

            // Report interval register for sensor 4
            case 0x18:
                rv = eeprom_read(EEPROM_REPORT_INTERVAL4);
                break;

            // Report interval register for sensor 5
            case 0x19:
                rv = eeprom_read(EEPROM_REPORT_INTERVAL5);
                break;

            // B Constants

            // B constant MSB for sensor 0
            case 0x26:
                rv = eeprom_read(EEPROM_B_CONSTANT0_MSB);
                break;

            // B constant LSB for sensor 0
            case 0x27:
                rv = eeprom_read(EEPROM_B_CONSTANT0_LSB);
                break;

            // B constant MSB for sensor 1
            case 0x28:
                rv = eeprom_read(EEPROM_B_CONSTANT1_MSB);
                break;

            // B constant LSB for sensor 1
            case 0x29:
                rv = eeprom_read(EEPROM_B_CONSTANT1_LSB);
                break;

            // B constant MSB for sensor 2
            case 0x2A:
                rv = eeprom_read(EEPROM_B_CONSTANT2_MSB);
                break;

            // B constant LSB for sensor 2
            case 0x2B:
                rv = eeprom_read(EEPROM_B_CONSTANT2_LSB);
                break;

            // B constant MSB for sensor 3
            case 0x2C:
                rv = eeprom_read(EEPROM_B_CONSTANT3_MSB);
                break;

            // B constant LSB for sensor 3
            case 0x2D:
                rv = eeprom_read(EEPROM_B_CONSTANT3_LSB);
                break;

            // B constant MSB for sensor 4
            case 0x2E:
                rv = eeprom_read(EEPROM_B_CONSTANT4_MSB);
                break;

            // B constant LSB for sensor 4
            case 0x2F:
                rv = eeprom_read(EEPROM_B_CONSTANT4_LSB);
                break;

            // B constant MSB for sensor 5
            case 0x30:
                rv = eeprom_read(EEPROM_B_CONSTANT5_MSB);
                break;

            // B constant LSB for sensor 5
            case 0x31:
                rv = eeprom_read(EEPROM_B_CONSTANT5_LSB);
                break;


            // Low alarm registers


            // Low alarm registers MSB for sensor 0
            case 0x32:
                rv = eeprom_read(EEPROM_LOW_ALARM0_MSB);
                break;

            // Low alarm registers LSB for sensor 0
            case 0x33:
                rv = eeprom_read(EEPROM_LOW_ALARM0_LSB);
                break;

            // Low alarm registers MSB for sensor 1
            case 0x34:
                rv = eeprom_read(EEPROM_LOW_ALARM1_MSB);
                break;

            // Low alarm registers LSB for sensor 1
            case 0x35:
                rv = eeprom_read(EEPROM_LOW_ALARM1_LSB);
                break;

            // Low alarm registers MSB for sensor 2
            case 0x36:
                rv = eeprom_read(EEPROM_LOW_ALARM2_MSB);
                break;

            // Low alarm registers LSB for sensor 2
            case 0x37:
                rv = eeprom_read(EEPROM_LOW_ALARM2_LSB);
                break;

            // Low alarm registers MSB for sensor 3
            case 0x38:
                rv = eeprom_read(EEPROM_LOW_ALARM3_MSB);
                break;

            // Low alarm registers LSB for sensor 3
            case 0x39:
                rv = eeprom_read(EEPROM_LOW_ALARM3_LSB);
                break;

            // Low alarm registers MSB for sensor 4
            case 0x3A:
                rv = eeprom_read(EEPROM_LOW_ALARM4_MSB);
                break;

            // Low alarm registers LSB for sensor 4
            case 0x3B:
                rv = eeprom_read(EEPROM_LOW_ALARM4_LSB);
                break;

            // Low alarm registers MSB for sensor 5
            case 0x3C:
                rv = eeprom_read(EEPROM_LOW_ALARM5_MSB);
                break;

            // Low alarm registers LSB for sensor 5
            case 0x3D:
                rv = eeprom_read(EEPROM_LOW_ALARM5_LSB);
                break;



            // High alarm registers


            // High alarm registers MSB for sensor 0
            case 0x3E:
                rv = eeprom_read(EEPROM_HIGH_ALARM0_MSB);
                break;

            // High alarm registers LSB for sensor 0
            case 0x3F:
                rv = eeprom_read(EEPROM_HIGH_ALARM0_LSB);
                break;

            // High alarm registers MSB for sensor 1
            case 0x40:
                rv = eeprom_read(EEPROM_HIGH_ALARM1_MSB);
                break;

            // High alarm registers LSB for sensor 1
            case 0x41:
                rv = eeprom_read(EEPROM_HIGH_ALARM1_LSB);
                break;

            // High alarm registers MSB for sensor 2
            case 0x42:
                rv = eeprom_read(EEPROM_HIGH_ALARM2_MSB);
                break;

            // High alarm registers LSB for sensor 2
            case 0x43:
                rv = eeprom_read(EEPROM_HIGH_ALARM2_LSB);
                break;

            // High alarm registers MSB for sensor 3
            case 0x44:
                rv = eeprom_read(EEPROM_HIGH_ALARM3_MSB);
                break;

            // High alarm registers LSB for sensor 3
            case 0x45:
                rv = eeprom_read(EEPROM_HIGH_ALARM3_LSB);
                break;

            // High alarm registers MSB for sensor 4
            case 0x46:
                rv = eeprom_read(EEPROM_HIGH_ALARM4_MSB);
                break;

            // High alarm registers LSB for sensor 4
            case 0x47:
                rv = eeprom_read(EEPROM_HIGH_ALARM4_LSB);
                break;

            // High alarm registers MSB for sensor 5
            case 0x48:
                rv = eeprom_read(EEPROM_HIGH_ALARM5_MSB);
                break;

            // High alarm registers LSB for sensor 5
            case 0x49:
                rv = eeprom_read(EEPROM_HIGH_ALARM5_LSB);
                break;


            // Sensor zone information


            // Zone for sensor 0
            case 0x4A:
                rv = eeprom_read(EEPROM_SENSOR0_ZONE);
                break;

            // Sub zone for senor 0
            case 0x4B:
                rv = eeprom_read(EEPROM_SENSOR0_SUBZONE);
                break;

            // Zone for sensor 1
            case 0x4C:
                rv = eeprom_read(EEPROM_SENSOR1_ZONE);
                break;

            // Sub zone for senor 1
            case 0x4D:
                rv = eeprom_read(EEPROM_SENSOR1_SUBZONE);
                break;

            // Zone for sensor 2
            case 0x4E:
                rv = eeprom_read(EEPROM_SENSOR2_ZONE);
                break;

            // Sub zone for senor 2
            case 0x4F:
                rv = eeprom_read(EEPROM_SENSOR2_SUBZONE);
                break;

            // Zone for sensor 3
            case 0x50:
                rv = eeprom_read(EEPROM_SENSOR3_ZONE);
                break;

            // Sub zone for senor 3
            case 0x51:
                rv = eeprom_read(EEPROM_SENSOR3_SUBZONE);
                break;

            // Zone for sensor 4
            case 0x52:
                rv = eeprom_read(EEPROM_SENSOR4_ZONE);
                break;

            // Sub zone for senor 4
            case 0x53:
                rv = eeprom_read(EEPROM_SENSOR4_SUBZONE);
                break;

            // Zone for sensor 5
            case 0x54:
                rv = eeprom_read(EEPROM_SENSOR5_ZONE);
                break;

            // Sub zone for senor 5
            case 0x55:
                rv = eeprom_read(EEPROM_SENSOR5_SUBZONE);
                break;


            // Sensor absolute low


            // Absolute low registers MSB for sensor 0
            case 0x56:
                rv = eeprom_read(EEPROM_ABSOLUT_LOW0_MSB);
                break;

            // Absolute low  registers LSB for sensor 0
            case 0x57:
                rv = eeprom_read(EEPROM_ABSOLUT_LOW0_LSB);
                break;

            // Absolute low  registers MSB for sensor 1
            case 0x58:
                rv = eeprom_read(EEPROM_ABSOLUT_LOW1_MSB);
                break;

            // Absolute low  registers LSB for sensor 1
            case 0x59:
                rv = eeprom_read(EEPROM_ABSOLUT_LOW1_LSB);
                break;

            // Absolute low  registers MSB for sensor 2
            case 0x5A:
                rv = eeprom_read(EEPROM_ABSOLUT_LOW2_MSB);
                break;

            // Absolute low  registers LSB for sensor 2
            case 0x5B:
                rv = eeprom_read(EEPROM_ABSOLUT_LOW2_LSB);
                break;

            // Absolute low  registers MSB for sensor 3
            case 0x5C:
                rv = eeprom_read(EEPROM_ABSOLUT_LOW3_MSB);
                break;

            // Absolute low  registers LSB for sensor 3
            case 0x5D:
                rv = eeprom_read(EEPROM_ABSOLUT_LOW3_LSB);
                break;

            // Absolute low  registers MSB for sensor 4
            case 0x5E:
                rv = eeprom_read(EEPROM_ABSOLUT_LOW4_MSB);
                break;

            // Absolute low  registers LSB for sensor 4
            case 0x5F:
                rv = eeprom_read(EEPROM_ABSOLUT_LOW4_LSB);
                break;

            // Absolute low  registers MSB for sensor 5
            case 0x60:
                rv = eeprom_read(EEPROM_ABSOLUT_LOW5_MSB);
                break;

            // Absolute low  registers LSB for sensor 5
            case 0x61:
                rv = eeprom_read(EEPROM_ABSOLUT_LOW5_LSB);
                break;


            // Sensor absolute high


            // Absolute high registers MSB for sensor 0
            case 0x62:
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH0_MSB);
                break;

            // Absolute high  registers LSB for sensor 0
            case 0x63:
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH0_LSB);
                break;

            // Absolute high  registers MSB for sensor 1
            case 0x64:
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH1_MSB);
                break;

            // Absolute high  registers LSB for sensor 1
            case 0x65:
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH1_LSB);
                break;

            // Absolute high  registers MSB for sensor 2
            case 0x66:
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH2_MSB);
                break;

            // Absolute high  registers LSB for sensor 2
            case 0x67:
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH2_LSB);
                break;

            // Absolute high  registers MSB for sensor 3
            case 0x68:
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH3_MSB);
                break;

            // Absolute high  registers LSB for sensor 3
            case 0x69:
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH3_LSB);
                break;

            // Absolute high  registers MSB for sensor 4
            case 0x6A:
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH4_MSB);
                break;

            // Absolute high  registers LSB for sensor 4
            case 0x6B:
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH4_LSB);
                break;

            // Absolute high  registers MSB for sensor 5
            case 0x6C:
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH5_MSB);
                break;

            // Absolute high  registers LSB for sensor 5
            case 0x6D:
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH5_LSB);
                break;


            // Hysteresis



            // hysteresis for sensor 0
            case 0x6E:
                rv = eeprom_read(EEPROM_HYSTERESIS_SENSOR0);
                break;

            // hysteresis for sensor 1
            case 0x6F:
                rv = eeprom_read(EEPROM_HYSTERESIS_SENSOR1);
                break;

            // hysteresis for sensor 2
            case 0x70:
                rv = eeprom_read(EEPROM_HYSTERESIS_SENSOR2);
                break;

            // hysteresis for sensor 3
            case 0x71:
                rv = eeprom_read(EEPROM_HYSTERESIS_SENSOR3);
                break;

            // hysteresis for sensor 4
            case 0x72:
                rv = eeprom_read(EEPROM_HYSTERESIS_SENSOR4);
                break;

            // hysteresis for sensor 5
            case 0x73:
                rv = eeprom_read(EEPROM_HYSTERESIS_SENSOR5);
                break;

            // Reserved
            case 0x74:
                rv = 0;
                break;

            // Reserved
            case 0x75:
                rv = 0;
                break;

            // Calibration voltage MSB    
            case 0x76:
                rv = eeprom_read(EEPROM_CALIBRATED_VOLTAGE_MSB);
                break;

            // Calibration voltage LSB    
            case 0x77:
                rv = eeprom_read(EEPROM_CALIBRATED_VOLTAGE_LSB);
                break;

            default:
                rv = 0;
                break;
        }
    }
    else if (1 == vscp_page_select) {
        
        // SH Coefficients
        if (reg < 72) {
            rv = eeprom_read(EEPROM_COEFFICIENT_A_SENSOR0_0 + reg ) ;
        }
        // Raw A/D values
        else if (reg < 84) {
            uint8_t pos = reg - 72;
            rv = adc[ pos ];
        }
        // Sensor calibration values
        else if (reg < 98) {
            rv = eeprom_read( EEPROM_CALIBRATION_SENSOR0_MSB + reg - 84 ) ;
        }
    }

    return rv;

}

///////////////////////////////////////////////////////////////////////////////
// VSCP_writeAppReg
//

uint8_t vscp_writeAppReg(unsigned char reg, unsigned char val)
{
    uint8_t rv;
    //int tmpval;
    //uint8_t checksum;
    //uint8_t unit = (0x03 & eeprom_read(EEPROM_CONTROLREG0));
    //uint16_t page = eeprom_read(EEPROM_CONTROLREG0) +
    //                    eeprom_read(EEPROM_CONTROLREG0)* 256;

    rv = ~val; // error return

    if (0 == vscp_page_select) {

        switch (reg) {

            // Zone
            case 0x00:
                eeprom_write(EEPROM_ZONE, val);
                rv = eeprom_read(EEPROM_ZONE);
                break;

            // SubZone
            case 0x01:
                eeprom_write(EEPROM_SUBZONE, val);
                rv = eeprom_read(EEPROM_SUBZONE);
                break;


            // Control registers


            // Control register sensor 0
            case 0x02:
                eeprom_write(EEPROM_CONTROLREG0, val);
                rv = eeprom_read(EEPROM_CONTROLREG0);
                break;

            // Control register sensor 1
            case 0x03:
                eeprom_write(EEPROM_CONTROLREG1, val);
                rv = eeprom_read(EEPROM_CONTROLREG1);
                break;

            // Control register sensor 2
            case 0x04:
                eeprom_write(EEPROM_CONTROLREG2, val);
                rv = eeprom_read(EEPROM_CONTROLREG2);
                break;

            // Control register sensor 3
            case 0x05:
                eeprom_write(EEPROM_CONTROLREG3, val);
                rv = eeprom_read(EEPROM_CONTROLREG3);
                break;

            // Control register sensor 4
            case 0x06:
                eeprom_write(EEPROM_CONTROLREG4, val);
                rv = eeprom_read(EEPROM_CONTROLREG4);
                break;

            // Control register sensor 5
            case 0x07:
                eeprom_write(EEPROM_CONTROLREG5, val);
                rv = eeprom_read(EEPROM_CONTROLREG5);
                break;


            // Report interval


            // Report interval register sensor 0
            case 0x14:
                eeprom_write(EEPROM_REPORT_INTERVAL0, val);
                rv = eeprom_read(EEPROM_REPORT_INTERVAL0);
                break;

            // Report interval register sensor 1
            case 0x15:
                eeprom_write(EEPROM_REPORT_INTERVAL1, val);
                rv = eeprom_read(EEPROM_REPORT_INTERVAL1);
                break;

            // Report interval register sensor 2
            case 0x16:
                eeprom_write(EEPROM_REPORT_INTERVAL2, val);
                rv = eeprom_read(EEPROM_REPORT_INTERVAL2);
                break;

            // Report interval register sensor 3
            case 0x17:
                eeprom_write(EEPROM_REPORT_INTERVAL3, val);
                rv = eeprom_read(EEPROM_REPORT_INTERVAL3);
                break;

            // Report interval register sensor 4
            case 0x18:
                eeprom_write(EEPROM_REPORT_INTERVAL4, val);
                rv = eeprom_read(EEPROM_REPORT_INTERVAL4);
                break;

            // Report interval register sensor 5
            case 0x19:
                eeprom_write(EEPROM_REPORT_INTERVAL5, val);
                rv = eeprom_read(EEPROM_REPORT_INTERVAL5);
                break;


             // B constant registers


            // B constant register MSB for sensor 0
            case 0x26:
                eeprom_write(EEPROM_B_CONSTANT0_MSB, val);
                rv = eeprom_read(EEPROM_B_CONSTANT0_MSB);
                break;

            // B constant register LSB for sensor 0
            case 0x27:
                eeprom_write(EEPROM_B_CONSTANT0_LSB, val);
                rv = eeprom_read(EEPROM_B_CONSTANT0_LSB);
                break;

            // B constant register MSB for sensor 1
            case 0x28:
                eeprom_write(EEPROM_B_CONSTANT1_MSB, val);
                rv = eeprom_read(EEPROM_B_CONSTANT1_MSB);
                break;

            // B constant register LSB for sensor 1
            case 0x29:
                eeprom_write(EEPROM_B_CONSTANT1_LSB, val);
                rv = eeprom_read(EEPROM_B_CONSTANT1_LSB);
                break;

            // B constant register MSB for sensor 2
            case 0x2A:
                eeprom_write(EEPROM_B_CONSTANT2_MSB, val);
                rv = eeprom_read(EEPROM_B_CONSTANT2_MSB);
                break;

            // B constant register LSB for sensor 2
            case 0x2B:
                eeprom_write(EEPROM_B_CONSTANT2_LSB, val);
                rv = eeprom_read(EEPROM_B_CONSTANT2_LSB);
                break;

            // B constant register MSB for sensor 3
            case 0x2C:
                eeprom_write(EEPROM_B_CONSTANT3_MSB, val);
                rv = eeprom_read(EEPROM_B_CONSTANT3_MSB);
                break;

            // B constant register LSB for sensor 3
            case 0x2D:
                eeprom_write(EEPROM_B_CONSTANT3_LSB, val);
                rv = eeprom_read(EEPROM_B_CONSTANT3_LSB);
                break;

            // B constant register MSB for sensor 4
            case 0x2E:
                eeprom_write(EEPROM_B_CONSTANT4_MSB, val);
                rv = eeprom_read(EEPROM_B_CONSTANT4_MSB);
                break;

            // B constant register LSB for sensor 4
            case 0x2F:
                eeprom_write(EEPROM_B_CONSTANT4_LSB, val);
                rv = eeprom_read(EEPROM_B_CONSTANT4_LSB);
                break;

            // B constant register MSB for sensor 5
            case 0x30:
                eeprom_write(EEPROM_B_CONSTANT5_MSB, val);
                rv = eeprom_read(EEPROM_B_CONSTANT5_MSB);
                break;

            // B constant register LSB for sensor 5
            case 0x31:
                eeprom_write(EEPROM_B_CONSTANT5_LSB, val);
                rv = eeprom_read(EEPROM_B_CONSTANT5_LSB);
                break;


            // Low alarm registers


            // Low alarm point register MSB for sensor 0
            case 0x32:
                eeprom_write(EEPROM_LOW_ALARM0_MSB, val);
                rv = eeprom_read(EEPROM_LOW_ALARM0_MSB);
                break;

            // Low alarm point register LSB for sensor 0
            case 0x33:
                eeprom_write(EEPROM_LOW_ALARM0_LSB, val);
                rv = eeprom_read(EEPROM_LOW_ALARM0_LSB);
                break;

            // Low alarm point register MSB for sensor 1
            case 0x34:
                eeprom_write(EEPROM_LOW_ALARM1_MSB, val);
                rv = eeprom_read(EEPROM_LOW_ALARM1_MSB);
                break;

            // Low alarm point register LSB for sensor 1
            case 0x35:
                eeprom_write(EEPROM_LOW_ALARM1_LSB, val);
                rv = eeprom_read(EEPROM_LOW_ALARM1_LSB);
                break;

            // Low alarm point register MSB for sensor 2
            case 0x36:
                eeprom_write(EEPROM_LOW_ALARM2_MSB, val);
                rv = eeprom_read(EEPROM_LOW_ALARM2_MSB);
                break;

            // Low alarm point register LSB for sensor 2
            case 0x37:
                eeprom_write(EEPROM_LOW_ALARM2_LSB, val);
                rv = eeprom_read(EEPROM_LOW_ALARM2_LSB);
                break;

            // Low alarm point register MSB for sensor 3
            case 0x38:
                eeprom_write(EEPROM_LOW_ALARM3_MSB, val);
                rv = eeprom_read(EEPROM_LOW_ALARM3_MSB);
                break;

            // Low alarm point register LSB for sensor 3
            case 0x39:
                eeprom_write(EEPROM_LOW_ALARM3_LSB, val);
                rv = eeprom_read(EEPROM_LOW_ALARM3_LSB);
                break;

            // Low alarm point register MSB for sensor 4
            case 0x3A:
                eeprom_write(EEPROM_LOW_ALARM4_MSB, val);
                rv = eeprom_read(EEPROM_LOW_ALARM4_MSB);
                break;

            // Low alarm point register LSB for sensor 4
            case 0x3B:
                eeprom_write(EEPROM_LOW_ALARM4_LSB, val);
                rv = eeprom_read(EEPROM_LOW_ALARM4_LSB);
                break;

            // Low alarm point register MSB for sensor 5
            case 0x3C:
                eeprom_write(EEPROM_LOW_ALARM5_MSB, val);
                rv = eeprom_read(EEPROM_LOW_ALARM5_MSB);
                break;

            // Low alarm point register LSB for sensor 5
            case 0x3D:
                eeprom_write(EEPROM_LOW_ALARM5_LSB, val);
                rv = eeprom_read(EEPROM_LOW_ALARM5_LSB);
                break;


            // High alarm registers


            // High alarm point register MSB for sensor 0
            case 0x3E:
                eeprom_write(EEPROM_HIGH_ALARM0_MSB, val);
                rv = eeprom_read(EEPROM_HIGH_ALARM0_MSB);
                break;

            // High alarm point register LSB for sensor 0
            case 0x3F:
                eeprom_write(EEPROM_HIGH_ALARM0_LSB, val);
                rv = eeprom_read(EEPROM_HIGH_ALARM0_LSB);
                break;

            // High alarm point register MSB for sensor 1
            case 0x40:
                eeprom_write(EEPROM_HIGH_ALARM1_MSB, val);
                rv = eeprom_read(EEPROM_HIGH_ALARM1_MSB);
                break;

            // High alarm point register LSB for sensor 1
            case 0x41:
                eeprom_write(EEPROM_HIGH_ALARM1_LSB, val);
                rv = eeprom_read(EEPROM_HIGH_ALARM1_LSB);
                break;

            // High alarm point register MSB for sensor 2
            case 0x42:
                eeprom_write(EEPROM_HIGH_ALARM2_MSB, val);
                rv = eeprom_read(EEPROM_HIGH_ALARM2_MSB);
                break;

            // High alarm point register LSB for sensor 2
            case 0x43:
                eeprom_write(EEPROM_HIGH_ALARM2_LSB, val);
                rv = eeprom_read(EEPROM_HIGH_ALARM2_LSB);
                break;

            // High alarm point register MSB for sensor 3
            case 0x44:
                eeprom_write(EEPROM_HIGH_ALARM3_MSB, val);
                rv = eeprom_read(EEPROM_HIGH_ALARM3_MSB);
                break;

            // High alarm point register LSB for sensor 3
            case 0x45:
                eeprom_write(EEPROM_HIGH_ALARM3_LSB, val);
                rv = eeprom_read(EEPROM_HIGH_ALARM3_LSB);
                break;

            // High alarm point register MSB for sensor 4
            case 0x46:
                eeprom_write(EEPROM_HIGH_ALARM4_MSB, val);
                rv = eeprom_read(EEPROM_HIGH_ALARM4_MSB);
                break;

            // High alarm point register LSB for sensor 4
            case 0x47:
                eeprom_write(EEPROM_HIGH_ALARM4_LSB, val);
                rv = eeprom_read(EEPROM_HIGH_ALARM4_LSB);
                break;

            // High alarm point register MSB for sensor 5
            case 0x48:
                eeprom_write(EEPROM_HIGH_ALARM5_MSB, val);
                rv = eeprom_read(EEPROM_HIGH_ALARM5_MSB);
                break;

            // High alarm point register LSB for sensor 5
            case 0x49:
                eeprom_write(EEPROM_HIGH_ALARM5_LSB, val);
                rv = eeprom_read(EEPROM_HIGH_ALARM5_LSB);
                break;


            // Sensor zone information registers


            // Zone for sensor 0
            case 0x4A:
                eeprom_write(EEPROM_SENSOR0_ZONE, val);
                rv = eeprom_read(EEPROM_SENSOR0_ZONE);
                break;

            // Sub zone for sensor 0
            case 0x4B:
                eeprom_write(EEPROM_SENSOR0_SUBZONE, val);
                rv = eeprom_read(EEPROM_SENSOR0_SUBZONE);
                break;

            // Zone for sensor 1
            case 0x4C:
                eeprom_write(EEPROM_SENSOR1_ZONE, val);
                rv = eeprom_read(EEPROM_SENSOR1_ZONE);
                break;

            // Subzone for sensor 1
            case 0x4D:
                eeprom_write(EEPROM_SENSOR1_SUBZONE, val);
                rv = eeprom_read(EEPROM_SENSOR1_SUBZONE);
                break;

            // Zone for sensor 2
            case 0x4E:
                eeprom_write(EEPROM_SENSOR2_ZONE, val);
                rv = eeprom_read(EEPROM_SENSOR2_ZONE);
                break;

            // Sub zone for sensor 2
            case 0x4F:
                eeprom_write(EEPROM_SENSOR2_SUBZONE, val);
                rv = eeprom_read(EEPROM_SENSOR2_SUBZONE);
                break;

            // Zone for sensor 3
            case 0x50:
                eeprom_write(EEPROM_SENSOR3_ZONE, val);
                rv = eeprom_read(EEPROM_SENSOR3_ZONE);
                break;

            // Sub zone for sensor 3
            case 0x51:
                eeprom_write(EEPROM_SENSOR3_SUBZONE, val);
                rv = eeprom_read(EEPROM_SENSOR3_SUBZONE);
                break;

            // Zone for sensor 4
            case 0x52:
                eeprom_write(EEPROM_SENSOR4_ZONE, val);
                rv = eeprom_read(EEPROM_SENSOR4_ZONE);
                break;

            // Sub zone for sensor 4
            case 0x53:
                eeprom_write(EEPROM_SENSOR4_SUBZONE, val);
                rv = eeprom_read(EEPROM_SENSOR4_SUBZONE);
                break;

            // Zone for sensor 5
            case 0x54:
                eeprom_write(EEPROM_SENSOR5_ZONE, val);
                rv = eeprom_read(EEPROM_SENSOR5_ZONE);
                break;

            // Sub zone for sensor 5
            case 0x55:
                eeprom_write(EEPROM_SENSOR5_SUBZONE, val);
                rv = eeprom_read(EEPROM_SENSOR5_SUBZONE);
                break;


            // Sensor absolute low temperature registers


            // Sensor absolute low temperature register MSB for sensor 0
            case 0x56:
                eeprom_write(EEPROM_ABSOLUT_LOW0_MSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_LOW0_MSB);
                break;

            // Sensor absolute low temperature register LSB for sensor 0
            case 0x57:
                eeprom_write(EEPROM_ABSOLUT_LOW0_LSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_LOW0_LSB);
                break;

            // Sensor absolute low temperature register MSB for sensor 1
            case 0x58:
                eeprom_write(EEPROM_ABSOLUT_LOW1_MSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_LOW1_MSB);
                break;

            // Sensor absolute low temperature register LSB for sensor 1
            case 0x59:
                eeprom_write(EEPROM_ABSOLUT_LOW1_LSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_LOW1_LSB);
                break;

            // Sensor absolute low temperature register MSB for sensor 2
            case 0x5A:
                eeprom_write(EEPROM_ABSOLUT_LOW2_MSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_LOW2_MSB);
                break;

            // Sensor absolute low temperature register LSB for sensor 2
            case 0x5B:
                eeprom_write(EEPROM_ABSOLUT_LOW2_LSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_LOW2_LSB);
                break;

            // Sensor absolute low temperature register MSB for sensor 3
            case 0x5C:
                eeprom_write(EEPROM_ABSOLUT_LOW3_MSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_LOW3_MSB);
                break;

            // Sensor absolute low temperature register LSB for sensor 3
            case 0x5D:
                eeprom_write(EEPROM_ABSOLUT_LOW3_LSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_LOW3_LSB);
                break;

            // Sensor absolute low temperature register MSB for sensor 4
            case 0x5E:
                eeprom_write(EEPROM_ABSOLUT_LOW4_MSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_LOW4_MSB);
                break;

            // Sensor absolute low temperature register LSB for sensor 4
            case 0x5F:
                eeprom_write(EEPROM_ABSOLUT_LOW4_LSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_LOW4_LSB);
                break;

            // Sensor absolute low temperature register MSB for sensor 5
            case 0x60:
                eeprom_write(EEPROM_ABSOLUT_LOW5_MSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_LOW5_MSB);
                break;

            // Sensor absolute low temperature register LSB for sensor 5
            case 0x61:
                eeprom_write(EEPROM_ABSOLUT_LOW5_LSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_LOW5_LSB);
                break;


            // Sensor absolute high temperature registers


            // Sensor absolute high temperature register MSB for sensor 0
            case 0x62:
                eeprom_write(EEPROM_ABSOLUT_HIGH0_MSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH0_MSB);
                break;

            // Sensor absolute high temperature register LSB for sensor 0
            case 0x63:
                eeprom_write(EEPROM_ABSOLUT_HIGH0_LSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH0_LSB);
                break;

            // Sensor absolute high temperature register MSB for sensor 1
            case 0x64:
                eeprom_write(EEPROM_ABSOLUT_HIGH1_MSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH1_MSB);
                break;

            // Sensor absolute high temperature register LSB for sensor 1
            case 0x65:
                eeprom_write(EEPROM_ABSOLUT_HIGH1_LSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH1_LSB);
                break;

            // Sensor absolute high temperature register MSB for sensor 2
            case 0x66:
                eeprom_write(EEPROM_ABSOLUT_HIGH2_MSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH2_MSB);
                break;

            // Sensor absolute high temperature register LSB for sensor 2
            case 0x67:
                eeprom_write(EEPROM_ABSOLUT_HIGH2_LSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH2_LSB);
                break;

            // Sensor absolute high temperature register MSB for sensor 3
            case 0x68:
                eeprom_write(EEPROM_ABSOLUT_HIGH3_MSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH3_MSB);
                break;

            // Sensor absolute high temperature register LSB for sensor 3
            case 0x69:
                eeprom_write(EEPROM_ABSOLUT_HIGH3_LSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH3_LSB);
                break;

            // Sensor absolute high temperature register MSB for sensor 4
            case 0x6A:
                eeprom_write(EEPROM_ABSOLUT_HIGH4_MSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH4_MSB);
                break;

            // Sensor absolute high temperature register LSB for sensor 4
            case 0x6B:
                eeprom_write(EEPROM_ABSOLUT_HIGH4_LSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH4_LSB);
                break;

            // Sensor absolute high temperature register MSB for sensor 5
            case 0x6C:
                eeprom_write(EEPROM_ABSOLUT_HIGH5_MSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH5_MSB);
                break;

            // Sensor absolute high temperature register LSB for sensor 5
            case 0x6D:
                eeprom_write(EEPROM_ABSOLUT_HIGH5_LSB, val );
                rv = eeprom_read(EEPROM_ABSOLUT_HIGH5_LSB);
                break;


            // Sensor hysteresis


            // Hysteresis for sensor 0
            case 0x6E:
                eeprom_write(EEPROM_HYSTERESIS_SENSOR0, val);
                rv = eeprom_read(EEPROM_HYSTERESIS_SENSOR0);
                break;

            // Hysteresis for sensor 1
            case 0x6F:
                eeprom_write(EEPROM_HYSTERESIS_SENSOR1, val);
                rv = eeprom_read(EEPROM_HYSTERESIS_SENSOR1);
                break;

            // Hysteresis for sensor 2
            case 0x70:
                eeprom_write(EEPROM_HYSTERESIS_SENSOR2, val);
                rv = eeprom_read(EEPROM_HYSTERESIS_SENSOR2);

            // Hysteresis for sensor 3
            case 0x71:
                eeprom_write(EEPROM_HYSTERESIS_SENSOR3, val);
                rv = eeprom_read(EEPROM_HYSTERESIS_SENSOR3);
                break;

            // Hysteresis for sensor 4
            case 0x72:
                eeprom_write(EEPROM_HYSTERESIS_SENSOR4, val);
                rv = eeprom_read(EEPROM_HYSTERESIS_SENSOR4);
                break;

            // Hysteresis for sensor 5
            case 0x73:
                eeprom_write(EEPROM_HYSTERESIS_SENSOR5, val);
                rv = eeprom_read(EEPROM_HYSTERESIS_SENSOR5);
                break;

            // reserved
            case 0x74:
                rv = 0;
                break;

            // Reserved
            case 0x75:
                rv = 0;
                break;

            // Voltage calibration values MSB                
            case 0x76:
                eeprom_write(EEPROM_CALIBRATED_VOLTAGE_MSB, val);
                rv = eeprom_read(EEPROM_CALIBRATED_VOLTAGE_MSB);
                break;

            // Voltage calibration values LSB    
            case 0x77:
                eeprom_write(EEPROM_CALIBRATED_VOLTAGE_LSB, val);
                rv = eeprom_read(EEPROM_CALIBRATED_VOLTAGE_LSB);
                break;

            default:
                rv = ~val; // error return
                break;
        }
    }
    else if (1 == vscp_page_select) {

        // Coefficients
        if (reg < 72) {
            eeprom_write(EEPROM_COEFFICIENT_A_SENSOR0_0 + reg, val);
            rv = eeprom_read(EEPROM_COEFFICIENT_A_SENSOR0_0 + reg );
            writeCoeffs2Ram();
        }
        // Raw A/D values is NOT writable
        else if (reg < 84) {
            uint8_t pos = reg - 72;
            rv = adc[ pos ];
        }
        // Sensor calibration values
        else if (reg < 98) {
            eeprom_write( EEPROM_CALIBRATION_SENSOR0_MSB + reg - 84, val );
            rv = eeprom_read( EEPROM_CALIBRATION_SENSOR0_MSB + reg - 84 ) ;
        }
    }

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
//                        VSCP Required Methods
//////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// Get Major version number for this hardware module
//

unsigned char vscp_getMajorVersion()
{
    return FIRMWARE_MAJOR_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// Get Minor version number for this hardware module
//

unsigned char vscp_getMinorVersion()
{
    return FIRMWARE_MINOR_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// Get Sub minor version number for this hardware module
//

unsigned char vscp_getSubMinorVersion()
{
    return FIRMWARE_SUB_MINOR_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// getVSCP_GUID
//
// Get GUID from EEPROM
//

uint8_t vscp_getGUID(uint8_t idx)
{
    return eeprom_read(VSCP_EEPROM_REG_GUID + idx);
}

///////////////////////////////////////////////////////////////////////////////
// VSCP_setGUID
//

#ifdef ENABLE_WRITE_2PROTECTED_LOCATIONS
void vscp_setGUID( uint8_t idx, uint8_t data )
{
    if ( idx>15 ) return;
    eeprom_write( VSCP_EEPROM_REG_GUID + idx, data );
}
#endif

///////////////////////////////////////////////////////////////////////////////
// getDeviceURL
//
// Get device URL from EEPROM
//

uint8_t vscp_getMDF_URL(uint8_t idx)
{
    return vscp_deviceURL[ idx ];
}

///////////////////////////////////////////////////////////////////////////////
// Get Manufacturer id and subid from EEPROM
//

uint8_t vscp_getUserID(uint8_t idx)
{
    return eeprom_read(VSCP_EEPROM_REG_USERID + idx);
}

///////////////////////////////////////////////////////////////////////////////
//  setVSCPUserID
//

void vscp_setUserID(uint8_t idx, uint8_t data)
{
    eeprom_write(idx + VSCP_EEPROM_REG_USERID, data);
}

///////////////////////////////////////////////////////////////////////////////
// getVSCPManufacturerId
// 
// Get Manufacturer id and subid from EEPROM
//

uint8_t vscp_getManufacturerId(uint8_t idx)
{
    return eeprom_read(VSCP_EEPROM_REG_MANUFACTUR_ID0 + idx);
}

///////////////////////////////////////////////////////////////////////////////
// getVSCPManufacturerId
// 
// Get Manufacturer id and subid from EEPROM
//

void vscp_setManufacturerId(uint8_t idx, uint8_t data)
{
    eeprom_write(VSCP_EEPROM_REG_MANUFACTUR_ID0 + idx, data);
}

///////////////////////////////////////////////////////////////////////////////
// Get the bootloader algorithm code
//

uint8_t vscp_getBootLoaderAlgorithm(void)
{
    return VSCP_BOOTLOADER_PIC1;
}

///////////////////////////////////////////////////////////////////////////////
// Get the buffer size
//

uint8_t vscp_getBufferSize(void)
{
    return 8; // Standard CAN frame
}

///////////////////////////////////////////////////////////////////////////////
//  VSCP_getMatrixInfo
//

void vscp_getMatrixInfo(char *pData)
{
    uint8_t i;

    // We don't implement a DM for this module
    // So we just set the data to zero
    for (i = 0; i < 8; i++) {
        pData[ i ] = 0;
    }

}

///////////////////////////////////////////////////////////////////////////////
//  vscp_getEmbeddedMdfInfo
//

void vscp_getEmbeddedMdfInfo(void)
{
    // No embedded DM so we respond with info about that

    vscp_omsg.priority = VSCP_PRIORITY_NORMAL;
    vscp_omsg.flags = VSCP_VALID_MSG + 3;
    vscp_omsg.vscp_class = VSCP_CLASS1_PROTOCOL;
    vscp_omsg.vscp_type = VSCP_TYPE_PROTOCOL_RW_RESPONSE;

    vscp_omsg.data[ 0 ] = 0;
    vscp_omsg.data[ 1 ] = 0;
    vscp_omsg.data[ 2 ] = 0;

    // Send the event
    vscp_sendEvent();
}


///////////////////////////////////////////////////////////////////////////////
// vscp_getRegisterPagesUsed
//

uint8_t vscp_getRegisterPagesUsed(void)
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getZone
//

uint8_t vscp_getZone(void)
{
    return eeprom_read(EEPROM_ZONE);
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getSubzone
//

uint8_t vscp_getSubzone(void)
{
    return eeprom_read(EEPROM_SUBZONE);
}

///////////////////////////////////////////////////////////////////////////////
// vscp_goBootloaderMode
//

void vscp_goBootloaderMode( uint8_t algorithm )
{
    if ( VSCP_BOOTLOADER_PIC1 != algorithm  ) return;

    // OK, We should enter boot loader mode
    // 	First, activate bootloader mode
    eeprom_write(VSCP_EEPROM_BOOTLOADER_FLAG, VSCP_BOOT_FLAG);
    
    // Turn off CAN
    ECANSetOperationMode( ECAN_OP_MODE_CONFIG ); 
    
    // Reset the "thingi"
    asm("reset");
}

///////////////////////////////////////////////////////////////////////////////
//  getNickname
//

uint8_t vscp_readNicknamePermanent(void)
{
    return eeprom_read( VSCP_EEPROM_NICKNAME );
}

///////////////////////////////////////////////////////////////////////////////
//  setNickname
//

void vscp_writeNicknamePermanent(uint8_t nickname)
{
    eeprom_write( VSCP_EEPROM_NICKNAME, nickname );
}

///////////////////////////////////////////////////////////////////////////////
//  setVSCPControlByte
//

void vscp_setControlByte( uint8_t idx, uint8_t ctrl )
{
    if ( idx > 1 ) return;
    eeprom_write( VSCP_EEPROM_CONTROL1 + idx, ctrl );
}


///////////////////////////////////////////////////////////////////////////////
//  getVSCPControlByte
//

uint8_t vscp_getControlByte( uint8_t idx )
{
    if ( idx > 1 ) return 0;
    return eeprom_read( VSCP_EEPROM_CONTROL1 + idx );
}

///////////////////////////////////////////////////////////////////////////////
//  vscp_init_pstorage(
//

void vscp_init_pstorage( void )
{
    init_app_eeprom();
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getFamilyCode
//

uint32_t vscp_getFamilyCode()
{
    return 0L;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_getFamilyType
//

uint32_t vscp_getFamilyType()
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_restoreDefaults
//

void vscp_restoreDefaults()
{
    init_app_eeprom();
    init_app_ram();
}

///////////////////////////////////////////////////////////////////////////////
// sendVSCPFrame
//

int8_t sendVSCPFrame(uint16_t vscpclass,
        uint8_t vscptype,
        uint8_t nodeid,
        uint8_t priority,
        uint8_t size,
        uint8_t *pData)
{
    uint32_t id = ((uint32_t) priority << 26) |
            ((uint32_t) vscpclass << 16) |
            ((uint32_t) vscptype << 8) |
            nodeid; // nodeaddress (our address)

    if (!sendCANFrame(id, size, pData)) {
        // Failed to send message
        vscp_errorcnt++;
        return FALSE;
    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// getVSCPFrame
//

int8_t getVSCPFrame( uint16_t *pvscpclass,
                        uint8_t *pvscptype,
                        uint8_t *pNodeId,
                        uint8_t *pPriority,
                        uint8_t *pSize,
                        uint8_t *pData)
{
    uint32_t id;

    if (!getCANFrame(&id, pSize, pData)) {
        return FALSE;
    }

    *pNodeId = id & 0x0ff;
    *pvscptype = (id >> 8) & 0xff;
    *pvscpclass = (id >> 16) & 0x1ff;
    *pPriority = (uint16_t) (0x07 & (id >> 26));

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// sendCANFrame
//

int8_t sendCANFrame(uint32_t id, uint8_t dlc, uint8_t *pdata)
{
    uint8_t rv = FALSE;
    
    sendTimer = 0;
    while ( sendTimer < 1000 ) {
        if ( ECANSendMessage( id, pdata, dlc, ECAN_TX_XTD_FRAME ) ) {
            rv = TRUE;
            break;
        }
    }

    vscp_omsg.flags = 0;

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// getCANFrame
//

int8_t getCANFrame(uint32_t *pid, uint8_t *pdlc, uint8_t *pdata)
{
    ECAN_RX_MSG_FLAGS flags;

    // Dont read in new message if there already is a message
    // in the input buffer
    if (vscp_imsg.flags & VSCP_VALID_MSG) return FALSE;

    if (ECANReceiveMessage((unsigned long *) pid, (BYTE*) pdata, (BYTE*) pdlc, &flags)) {
        // RTR not interesting
        if (flags & ECAN_RX_RTR_FRAME) return FALSE;

        // Must be extended frame
        if (!(flags & ECAN_RX_XTD_FRAME)) return FALSE;

        return TRUE;
    }

    return FALSE;
}
