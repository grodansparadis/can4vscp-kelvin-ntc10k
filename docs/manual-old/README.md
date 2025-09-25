# CAN4VSCP Kelvin NTC10K module

Document reversion: 2020-05-04
Copyright © 2000-2020 Grodans Paradis AB, <info@grodansparadis.com>

![Kelvin NTC10K](images/kelvin14.png)

This documentation is for the [Kelvin NTC10K module](https://www.grodansparadis.com/kelvinntc10k/kelvin_ntc10ka.html).

Disclaimer: © 2010-2020 Grodans Paradis AB. All rights reserved. Grodans Paradis AB®, logo and combinations thereof, are registered trademarks of Grodans Paradis AB. Other terms and product names may be trademarks of others. The information in this document is provided in connection with Grodans Paradis AB products. No license, express or implied or otherwise, to any intellectual property right is granted by this document or in connection with the sale of Grodans Paradis AB products. Neither the whole nor any part of the information contained in or the product described in this document may be adapted or reproduced in any material from except with the prior written permission of the copyright holder. The product described in this document is subject to continuous development and improvements. All particulars of the product and its use contained in this document are given by Grodans Paradis AB in good faith. However all warranties implied or expressed including but not limited to implied warranties of merchantability or fitness for purpose are excluded. This document is intended only to assist the reader in the use of the product. Grodans Paradis AB. shall not be liable for any loss or damage arising from the use of any information in this document or any error or omission in such information or any incorrect use of the product. 

# Introduction

![Kelvin NTC10K](./images/kelvin13.png)

 Kelvin NTC10K is a temperature module that can handle six temperature sensing elements of which one is internal. The module can be attached to a standard DIN Rail or be mounted directly on a wall (ordered as separate versions).

The module fully adopts to the CAN4VSCP specification and can be powered directly over the bus with a 9-30V DC power source. It has a rich register set for configuration and any information events defined. It also have a decision matrix for easy dynamic event handling.

VSCP CAN modules are designed to work on a CAN4VSCP bus which use ordinary RJ-45 connectors and is powered with a 9 - 28VDC power source where power is provided over the communication cable. This means there is no need for a separate power cable. All that is needed is a CAT5 or better twisted pair cable. Buss length can be a maximum of 500 meters with drops of maximum 24 meters length (up to a total of 120 meters). As for all CAN4VSCP modules the communication speed is fixed at 125 kbps.

All VSCP modules contains information of there own setup, manual, hardware version, manufacturer etc. You just ask the module for the information you need and you will get it. When they are started up they have a default functionality that often is all that is needed to get a working setup. If the module have something to report it will send you an event and if it is setup to react on a certain type of event it will do it's work when you send event(s) to it.

The module comes with a GUID (global unique id) and is programmed with a bootloader and VSCP module functionality that works right out of the box.

You can control things with the module, such as intelligent relay control for heating and cooling that our Paris module have, or set channel individual high and low alarm levels to safely monitor temperatures, possibly in remote locations.

# Geting started

 You need

* RJ-45 patch cables or similar.
* A stable +9V - +28V DC power source.
* A host computer with a serial or USB interface.

The Kelvin NTC10K module is powered from the CAN4VSCP bus. This article have information on how the bus is powered and set up. Follow it for a reliable setup. Don't forget the two 120 ohm terminators at each end of the bus. They are important! You can activate the on board terminator on the module if it is located at one end of your bus (see the [hardware section](#Hardware)).

When you power the module by inserting the RJ-45 cable the red LED on the board should light up to indicate it is powered. This is just a power indicator LED. When you power the board for the first time the green led starts to blink to indicate that the module search for a nickname. During normal operation this LED should light steady.

Remember that there need to be at least two CAN4VSCP (or other CAN device set to 125kbps) devices (one additional if you have the [Frankfurt RS-232](http://www.grodansparadis.com/frankfurt/rs232/frankfurt-rs232.html)) on the bus to be able to communicate. A single device can't talk by itself on a bus. It's an error. 

![Kelvin NTC10K](./images/odessa_inter_module_connector.png)

As an alternative to power the Kelvin NTC module from the CAN4VSCP bus you can power it over the daisy chain connector. The [CAN4VSCP Power Injector](http://www.grodansparadis.com/powerinjector/can4vscp_big/powerinjector.html) is needed (or similar) for this. 

## Installing the module

Connect the module to the CAN4VSCP bus. The red led led on the module should light up indicating that the device is powered. If this is the first time you start up the module the green lamp next to the initializing button will start to blink. This means that the module is trying to negotiate a nickname address with the rest of the modules on the bus. When it found a free nickname the green led will light steady. If the green led does not start to blink press the initialization button until it does. Now your module is ready to use.

If the green led never stops blinking you have a problem with your bus as the typically mean that the module dos not find any other module on the bus. Remember there must always be at least two active modules. Also check cables and that you have 120 ohm terminators installed on both ends of the bus.

When your module is installed it's time to configure it.

## Connecting temperature sensor(s)

![Thermistor](./images/termistor.jpg)

To just test that communication works you don't need to install any external temperature sensors. You can work with the internal sensor. By default it will send the board temperature in degrees Celsius every 30 seconds. You can view VSCP traffic on the CAN4VSCP bus with all CAN tools set to 125kbit/s but VSCP Works makes handling, configuring, diagnosing VSCP stuff a lot more easy. So this may be the time to get to know this piece of software that works on both Linux and Window. 

The internal temperature sensor is heated by the processor and other things on the PCB so it will always give a bit of a high value. You can calibrate away this error if you want. See the [configuration section](#Configuration). 

 Eventually you probably want to connect external temperature sensors to the module. You can connect any 10 Kohm NTC temperature sensor to the Kelvin NTC10K module. It helps if you know the B value of it for the temperature range you want to work in. You can get sensors and sensors on cables of different length from the FrogShop.

Five external NTC sensor can be connected to the Kelvin NTC10KA module. It is recommended to use NTC thermistors with 10K @ 25 °C types for best result. There are sensors available for all kinds of temperature measurement needs. As noted above you need the B value from the sensors data sheet or to be able to use it with the Kelvin NTC10KA module.

The main advantage for a NTC sensor is that it is low cost. You can get it from anywhere and you can get it in any type and dimension. NTC resistor is also not sensitive to cable length. Using a CAT-5e cable (4 pairs with AWG 24) we will experience something like 9.38 ohms/100m which will not effect a 10K NTC thermistor that much.

Connect the thermistor to the Kelvin NTC10KA by connecting it to +5VDC and to one of the temperature inputs. A thermistor is not subject to polarity considerations so you can connect it in any way you want. You need to setup the B value for the sensor to get correct temperature measurements. The pin out for the terminal block is in the hardware section. 

## To measure temperature

![Temperature measurements](./images/tempdiff.jpg)

 If you want to measure a temperature, for a room for example you will soon realize that temperature measurements has more to it then just hook up a sensor and measure the temperature. Look at the picture above.

Each of the sensors shows a different temperature even when the sensor are on the same desk. If you walk around in the room things will be even more complex. Three sensors of the same type but on different height will show different results.

The readings from the sensors of the Kelvin NTC module is given with two decimals. Does that mean we have 0.01°C precision? Unfortunately no! The thermistor has error and the analog reading circuitry (ADC) has error.

We can approximate the expected error by first taking into account the thermistor resistance error. The thermistor is correct to 1%, which means that at 25°C it can read 10,100 to 9900 ohms. At around 25°C a difference of 450 ohms represents 1°C so 1% error means about +-0.25°C (you may be able to calibrate this away by determining the resistance of the thermistor in a 0°C ice bath and removing any offset). You can also spring for a 0.1% thermistor which will reduce the possible resistance error down to +-0.03°C

Then there is the error of the ADC, for every bit that it is wrong the resistance (around 25°C) can be off by about 50 ohms. This isn't too bad, and is a smaller error than the thermistor error itself +-(0.1°C) but there is no way to calibrate it 'away' - just a higher precision ADC (12-16 bits instead of 10) will give you more precise readings

In general, we think thermistors are higher precision than thermocouples, or most low cost digital sensors, but you will not get better than +-0.1°C accuracy on with a 1% thermistor and we would suggest assuming no better than +-0.5°C.

You will in addition to this notice that the on-oard sensor will show a temperature that is higher then what you should expect, The same is if you connect a sensor directly to the board. This is due to the fact that the electronics on the board generate heat. This heat flows very well through the copper of the board giving faulty readings. The solution is to take away a degree or two from the reading. 

# Hardware

## Most current information
You can find the most current information about the Kelvin NTC10K module at
https://www.grodansparadis.com/products.html. On the site you can also find links to the latest firmware, drivers and schematics etc for its use. 

##   The raw facts
| Parameter | Value | 
| --------- | ----- | 
 | Current hardware reversion | B | 
 | Current firmware version | See [firmware release page](https///github.com/grodansparadis/can4vscp_kelvinntc10k/releases) | 
 | Supply voltage | +9-+28 VDC | 
 | PCB Size | 42mm x 72mm | 
 | Mounting | DIN or wall | 
 | Power requirements | 0.1W | 
 | Communication: CAN4VSCP (CAN) | 125 kbps | 
 | Number of sensors supported  | 6 | 

## Schematics

![Schema for Kelvin NTC10K](images/schema_kelvin_ntc_rev_b.png)

Schematics: Rev B (click picture to enlarge) 

## Components

### Internal temperature sensor

The internal temperature sensor used on the Kelvin NTC10KA is the Murata [NCP21XV103J03RA](https///www.google.se/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&cad=rja&uact=8&ved=0ahUKEwiu-ujjlrnJAhWDjiwKHY9aCQQQFggiMAA&url=http%3A%2F%2Fwww.murata.com%2F~%2Fmedia%2Fwebrenewal%2Fsupport%2Flibrary%2Fcatalog%2Fproducts%2Fthermistor%2Fntc%2Fr44e.ashx&usg=AFQjCNE0U3R9tCH8Mm3JbRtiqhAN5yqOaQ&sig2=3Ga5yx9Fc6e15Gvr6YVbPg). Data for this sensor is as follows

 | Description | Value | 
 | ----------- | ----- | 
 | Time constant | 10 s | 
 | B[25/100]- value | 3944K | 
 | Temperature range | -40...+125ºC  | 
 | Resistance tolerance  | 5% | 
 | Resistance @ 25ºC | 10 Kohm | 
 | Max effect | 210 @ 25ºC mW | 
 | B[25/50]- value | 3900 | 
 | B[25/85]- value | 3934 | 
 | B[25/100]- value| 3944K 3% | 
 | Max error (B[25/100]) | 2.8 (%/C) | 

![](./images/kelvin_parts_a.png)

![](./images/kelvin_parts_b.png)



## Connectors

![](./images/kelvin_connectors.png)


### Terminal block

![](./images/kelvin_12_pin.png)

 | Pin | Description  | 
 | --- | -----------  | 
 | 1   | Ground (GND) | 
 | 2   | I/O 9        | 
 | 3   | I/O 8        | 
 | 4   | I/O 7        | 
 | 5   | I/O 6        | 
 | 6   | I/O 5        | 
 | 7   | I/O 4        | 
 | 8   | I/O 3        | 
 | 9   | I/O 2        | 
 | 10  | I/O 1        | 
 | 11  | I/O 0        | 
 | 12  | +5V (VCC)    | 



### RJ-XX pin-out

The unit is powered over the CAN4VSCP bus. The CAN4VSCP normally uses CAT5 or better twisted pair cable. You can use other cables if you which. The important thing is that the CANH and CANL signals uses a twisted cable. For connectors you can use RJ10, RJ11, RJ12 or the most common RJ45 connectors.

Recommended connector is RJ-34/RJ-12 or RJ-11 with pin out as in this table.

 | Pin   | Use  | RJ-11 | RJ-12 | RJ-45 | Patch Cable wire color T568B | 
 | ---   | ---  | ----- | ----- | ----- | ---------------------------- | 
 | 1     | +9-28V DC |       |       | RJ-45 | Orange/White                 | 
 | 2 1   | +9-28V DC |       | RJ-12 | RJ-45 | Orange                       | 
 | 3 2 1 | +9-28V DC | RJ-11 | RJ-12 | RJ-45 | Green/White                  | 
 | 4 3 2 | CANH      | RJ-11 | RJ-12 | RJ-45 | Blue                         | 
 | 5 4 3 | CANL      | RJ-11 | RJ-12 | RJ-45 | Blue/White                   | 
 | 6 5 4 | GND       | RJ-11 | RJ-12 | RJ-45 | Green                        | 
 | 7 6   | GND       |       | RJ-12 | RJ-45 | Brown/White                  | 
 | 8     | GND       |       |       | RJ-45 | Brown                        | 

![RJ-11/12/45 pin-out](./images/rj45.jpg)

**RJ-11/12/45 pin-out**

:!: Always use a pair of twisted wires for CANH/CANL for best noise immunity. If the EIA/TIA 56B standard is used this condition will be satisfied. This is good as most Ethernet networks already is wired this way. 



### Inter module connector

The inter module connector can be used to connect modules that are physically close to each other together in an easy way. Remember that the minimum length of a connection cable is 30 cm.

 | Pin | Description | 
 | --- | ----------- | 
 | 1   | Power from CAN4VSCP bus | 
 | 2   | CANH | 
 | 3   | CANL | 
 | 4   | GND  | 


![Inter module connector](./images/odessa_inter_module_connector.png)


### PIC programming Connector

 | pin | Description | 
 | --- | ----------- | 
 | 1   | Reset       | 
 | 2   | VCC         | 
 | 3   | GND         | 
 | 4   | PGD (RX of second serial port is here to) | 
 | 5   | PGC (TX of second serial port is here to) | 
 | 6   | LWPGM  | 


![Programming connector](./images/odessa_programming_connector.png)


### Functionality of the status LED

The LED is used to indicate the status of the module. It will light steady when the firmware is running and will blink when the module is in the nickname discovery process.

 | LED | Description | 
 | --- | ----------- | 
 | Steady  | No error. Firmware running. | 
 | Blinking | Module is going through the [nickname discovery process](http://www.vscp.org/docs/vscpspec/doku.php?id=vscp_level_i_specifics#address_or_nickname_assignment_for_level_i_nodes). | 



### CAN

CAN4VSCP is a CAN based bus running at 125 kbps with the addition of DC power. If you are interested in how CAN works you have a pretty good intro [here](http://www.eeherald.com/section/design-guide/esmod9.html).

CAN is known to be robust and is there for used in vehicles and in the industry.

# Cabling

##   Cable length

CAN4VSCP always communicate with 125kbps. This means that if you use a good quality cable it can be up to a maximum of 500 meters using AWG24 or similar (CAT5) . Actual length depend on the environment and other parameters. Drops with a maximum length of 24 meters can be taken from this cable and the sum of all drops must not exceed a total of 120 meters. 

![](./images/bus_drops.jpg)

CAN4VSCP bus with drops and terminations

## Termination

The CAN4VSCP bus, as all CAN based networks, should be terminated with a 120 ohms resistor between CANH and CANL at both ends of the cable. 

![](./images/can_bus.png)


If you use CAT5 this termination should be placed between the blue - blue/white cables at both end of your bus.

On the board there is a jumper for an on-board terminator.


**Why is terminators required?**

Terminators are needed to cancel signal echos in the cable. In short you get less noise in the cable if you use them. It is recommended to use them even if at 125 kbit it is possible that your bus will work anyway as CAN is pretty forgiving at this transmission speed.

## Power the module

You normally power the module through the RJ45 connector over the CAN4VSCP bus. The voltage range is +9VDC - 28VDC.

It is possible to power the module on the TTL connector or to take power from this connector if power is available on the CAN4VSCP bus.  

## Best practice

Even if it is possible to have cable lengths up to 500 meters it is better to stay at shorter distances to have some margins. Here are some guidelines for a reliable set up


* Total cable length of 300m, stub cables count double their length in that total.

* Nodes count as 6m in that total. 

* Never less than 30cm of cable between nodes, nor between a stub connection and a node. 

* No more than 50 nodes connected.

Following this best practice you will have a setup that will work reliable even in harsh environments.

# Updating the firmware

New firmware is released from time to time for all our modules. We expect this to be true also for the Odessa module as it is an active project. The process of updating the firmware of the module is a simple one. Just follow the steps outlined in the [VSCP Works documentation](http://www.vscp.org/docs/vscpworks/doku.php?id=start).


## Where is the source for the firmware?

You can find the latest source for the firmware [here](https///github.com/grodansparadis/can4vscp-kelvin-ntc10k/releases). **You should use the relocated version of the firmware.**


## Step by step update process

Enter the bootloader in the menu

![](./images/boot1.png)

Select the interface the device is on

![](./images/boot2.png)

![](./images/boot3.png)

Select the nickname for the device to update firmware on. Make sure this is the correct id.  If this is the first time you update a node and only the bootloader is present use **0xfe**

![](./images/boot4.png)

On a node with working firmware the bootloader will be discovered automatically. If not select the algorithm (Microchip pic 1) for the device,


![](./images/boot5.png)


Load the firmware file using "local file". **You should use the relocated version of the firmware.**


![](./images/boot6.png)


Program the device


![](./images/boot7.png)

![](./images/boot8.png)


Due to a bug in VSCP Works restart VSCP works after the bootloader process is ready. 

Your device should now be available with the same id as you entered previously.

# Configure the module

You configure a VSCP module by writing content into the modules registers. You can do this manually or with the wizard available in VSCP works. Using the wizard is absolutely the easiest way to accomplish different things.

## Sensor calibration

Thermistors are very accurate but non linear devices. Still you may find that Kelvin NTC10K show you the wrong value of the measured temperature. This is because the sensor often need calibration or that registers need to be set to correct values. With calibration you can get very accurate results. For the best result you typically calibrate at three points, in ice + water mix, at 25 degrees Celsius and in boiling water. For most cases one calibration point will do but it all depends on what needs you have.

There is several registers that affect the reported temperature value. First of all there is the [Beta value registers](registers#%CE%B2_constant_registers). The value in these registers should be the Beta value for the sensor (and the range you use). Alternatively a value you get form calibration as of above.

The next value is the [voltage calibration value](registers#voltage_calibration_registers). This value is ten thousand times the voltage of the Kelvin NTC board. This is thus normally 50000 (for 5V) which is 0xC350 in hexadecimal. If the voltage of your board is not 5.00 volts you can enter the correct value here. This value seldom need a change.

The last and most important place are the  [sensor calibration registers](registers#sensor_calibration_registers) which can be used to set up a calibration value for each sensor that will be added or subtracted from the read value before it is reported. 

Values are stored as [twos complement numbers](https///www.cs.cornell.edu/~tomf/notes/cps104/twoscomp.html) and is as usual 100 times the actual value.

For example your sensor shows a temperature that is 1.4 degrees to low. 1.4 time 100 is 140 which is 0x008C in hexadecimal. So store 0x00 in the MSB byte and 0x8C in the LSB byte of the calibration value for that sensor. Now 1.4 degrees will be added to the measured value before it is reported and used in calculations.

Another sample is if your sensor measure a value that is 1.9 degrees to high. We therefore need to subtract 1.9. 100 * -1.9 = -190. This is  0xFF42 as a twos complement number  (190 =   0x00BE (0000 0000 1011 1110), Invert which give 0xFF41 (1111 1111 0100 0001), add one which gives 0cxFF42).

![](./images/calibration.jpg)

A good method to use to calibrate sensors is to mix ice and water and put the sensor in the mix. This will be zero degrees Celsius. To get another point boil some water and measure the temperature in the middle of the container without touching the walls.

A good tool for calibration is 

[http://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm](http://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm)


## Zones and sub zones

Zones and sub zones are used to group things together. Normally you don't address events to a certain receiver in VSCP ([CLASS1.PROTOCOL](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.protocol) being the exception that prove this rule). Instead you report/send things to a group of modules defined by a zone and a sub zone. The sender have no knowledge of how many units this zone/sub zone consist of. Well it does not even care.

Zone and sub zone for the module can be set in register [0:0 (page:register) and register 0:1](http://www.grodansparadis.com/kelvinntc10k/manual/doku.php?id=registers#module_zone_registers). But more useful than the module zone sub zone is the zone information for each individual sensor that is available in register [0:74 - 0:85](http://www.grodansparadis.com/kelvinntc10k/manual/doku.php?id=registers#sensor_zone_information_registers). Here you can specify the zone/sub zone units you think is interested in this event. You can also set them both (or one of them) to 0xff meaning all zones or all sub zones.

Think of a zone as a house, floor plan or similar and sub-zone as a room or a location. Note again that this is not an address. It's just a way to group functionality together. Or let the MSB bits of the zone be the floor plan, the LSB bits of the zone be the room on that floor plan and the sub zone be a group of functionality for that room.

It is good to think this through before you start to configure a system with VSCP modules.

## β values

In registers [0:38 - 0:49](http://www.grodansparadis.com/kelvinntc10k/manual/doku.php?id=registers#%CE%B2_constant_registers) you have registers that hold β (beta) values for each sensor. To get correct temperature readings it is important that you enter the correct value here for the sensor you use. You find this information in the data sheet for the sensor.

## Sensor Control Registers

In registers [0:2 - 0:7](registers#description_of_control_register_bits) you can configure how sensor data is reported and acted up on. You can set the unit you want temperatures to be reported in. You can enable alarms. You can define that TurnOn/TurnOff events should be sent when levels have been reached or data has been found to be below a certain level. And other things. 

## Temperature measurements

If you have gone through the steps above you can now read temperature data in [registers 0:8 - 0:19](registers#temperature_registers). The temperature is shown in the selected unit as an integer which is temperature * 100. 

The temperature registers is located in pairs (Most significant byte followed my the least significant byte) and they can be read to get the current temperature for each sensor. The temperature is stored as a 16-bit two complement of a normalized temperature. This means you have to divide the read temperature by 100 to get the temperature value.

To get the temperature and calculate the temperature for a specific sensor.

 1.  Read MSB register. Read LSB register.
 2.  Calculate the 16-bit twos complement as MSB reading * 256 + LSB reading.
 3.  If the most significant bit is not set (Equal or less then 32767) this is a positive temperature. Divide by 100 and you have the temperature. 
 4.  If the most significant bit is set (Greater than 32767) this is a negative temperature. Now invert the result (the bits are inverted; 0 becomes 1, and 1 becomes 0) and add one to the result. Dive by 100 and you have the temperature.

However usually this is not the way to get temperature readings. Instead you set a report interval for a sensor expressed in seconds in [register 0:20 - 0:25](registers#report_interval_registers) (or set to zero to turn of reporting) and get temperature readings automatically delivered/reported.

After entering a value here [temperature events](events#temperature_event) will be sent with the configured interval. The receiver of this information can be another module, a routine that writes them to a database or a websocket widget that diagram them live on a screen or on a web page.

Some samples

* https://www.youtube.com/embed/37PQomplUVE`

* https://www.youtube.com/embed/L17d1SSyBc0`

* https://www.youtube.com/embed/qV41vWt5toE


## Alarms

In the [Low Alarm](registers#low_alarm_registers) and in the [High Alarm](registers#high_alarm_registers) registers you can set the level at which [alarm events](events#alarm_event) are sent.

An activated low alarm event (activated by setting bit 3 in the [control register](registers#description_of_control_register_bits)) is sent when the temperature is going below the low alarm value. When this happens the low alarm bit in the [standard alarm register](alarms) is set. This bit will only be rested by reading the alarm register. The alarm is considered active as long as the temperature is below the low alarm set point + the hysteresis value set in [register 0:110 - 0:115](registers#sensor_hysteresis_registers). So if temperature moves above this point a new alarm event will be sent again when temperature again is moving below the set point.

An activated high alarm event (activated by setting bit 4 in the [control register](registers#description_of_control_register_bits)) is sent when the temperature is going above the high alarm value. When this happens the high alarm bit in the [standard alarm register](alarms) is set. This bit will only be rested by reading the alarm register. The alarm is considered active as long as the temperature is above the low alarm set point - the hysteresis value set in [register 0:110 - 0:115](registers#sensor_hysteresis_registers). So if temperature moves below this point a new alarm event will be sent again when temperature again is moving above the set point.

It is possible to have continuous alarms events sent by setting bit 7 in the [control register](registers#description_of_control_register_bits). An alarm event will be sent every second while the alarm condition persist.

## Controlling things

Instead of alarms [CLASS1.CONTROL, Type=5, TurnOn events](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.control#type_5_0x05_turnon) and [CLASS1.CONTROL, Type=6, TurnOn](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.control#type_6_0x06_turnoff) events can be sent.  This functionality is activated by setting **bit 5** in the [control register](http://www.grodansparadis.com/kelvinntc10k/manual/doku.php?id=registers#description_of_control_register_bits). 

Another bit, bit 6, in the control register decides when the TurnOff/TurnOn events are sent. If set to zero TurnOff is sent when the high point is reached and TurnOn when the low point is reached. Suitable for control of a heating system for example. If the bit is set the TurnOn event is sent at the high set point and the TurnOff is sent on the low set point. Suitable for a cooling system.

![](./images/limits.png)

The picture shows a real world example where a Kelvin NTC10K module measures temperature in a fridge which have its compressor controlled by a [Paris relay module](http://www.grodansparadis.com/paris/paris.html). The Paris [decision matrix](http://www.vscp.org/docs/vscpspec/doku.php?id=decision_matrix) is programmed to turn on the compressor when TurnOn events is received from the the zone/subzone programmed into the Kelvin NTC10K module sensor that measure the fridge temperature.

So here TurnOff events are sent by the Kelvin NTC10K module when the temperature go below -22 degrees Celsius. This event is received by the Paris module and it turn of the relay that controls the fridge compressor. Continuous mode is activated so the TurnOff events are sent until the temperature reach -20 degrees Celsius. A function to protect against events not being delivered.

When the temperature goes up above -20 degrees Celsius no events will be sent until the temperature go above -17 degrees Celsius. At this point TurnOn events will be sent which are received by the Paris module which turn on the relay that control the Fridge compressor. TurnOn events are sent continuously until the temperature go down to -19 degrees Celsius and again there will be a dead zone until the temperature falls to -22 degrees Celsius.

# Registers

All VSCP modules have a set of 8-bit registers defined to export it's functionality in a common way. Some of them (register 128-255) are predefined and the information in them are the same for all VSCP modules. See the [VSCP specification](http://www.vscp.org/docs/vscpspec/doku.php?id=register_abstraction_model) for a description of their content. 

The lower 128 register space is used for module specific registers. It is also here you find registers with which you configure your module. Also registers where you typically can read status information such as measurement data from the module is here.

The lower register space is further expanded in register pages.

Below is a description of the registers of the Kelvin NTC10KA module.

## Page 0

### Module zone registers

 | Register | Page | Description | 
 | -------- | ---- | ----------- | 
 | 0        | 0    | The zone this module belongs to. | 
 | 1        | 0    | The sub zone this module belongs to. | 

### Sensor Control Registers

Configure which data is reported here and how it is reported.


 | Register | Page | Description | 
 | -------- | ---- | ----------- | 
 | 2        | 0    | Control byte for sensor 0. | 
 | 3        | 0    | Control byte for sensor 1. | 
 | 4        | 0    | Control byte for sensor 2. | 
 | 5        | 0    | Control byte for sensor 3. | 
 | 6        | 0    | Control byte for sensor 4. | 
 | 7        | 0    | Control byte for sensor 5. | 

#### Description of control register bits

 | Bit | Description | 
 | --- | ----------- | 
 | 0,1 | Temperature unit \\ \\ **00** - Kelvin. \\ **01** - Celsius (default). \\  **10** - Fahrenheit. \\ **11** - Reserved. | 
 | 2   | Reserved. | 
 | 3   | Enable low alarm if set to one. | 
 | 4   | Enable high alarm if set to one. | 
 | 5   | Send **TurnOn/TurnOff** events instead of Alarm if set to one. | 
 | 6   | Controls when **TurnOn/TurnOff** events will be sent if activated. **0** - **TurnOn** event is sent when low temperature is reached as set in low temperature alarm register (see [sub:Low-alarm-registers]). **TurnOff** event is sent when high temperature is reached as set in high temperature alarm register (see [sub:High-alarm-registers]). **1** - **TurnOff** event is sent when low temperature is reached as set in low temperature alarm register (see [Registers]([registers)]). **TurnOn** event is sent when high temperature is reached as set in high temperature alarm register (see [sub:High-alarm-registers]). The hysteresis value (see [sub:Sensor-hysteresis-registers]) will effect **TurnOn/TurnOff** events The temperature must go above or below the hysteresis value before a new event will be sent out. | 
 | 7   | If this bit is set Alarm events will be sent continuous with a one second interval until the alarm register is read or the temperature goes to a non alarm state (with hysteresis taken into account see [[sub:Low-temperature-alarm]). **TurnOn/TurnOff** events will not be affected by a cleared alarm register and will be sent until this bit is cleared or the temperature goes to a non alarm state (with hysteresis taken into account. | 


###  Temperature registers

 | Register | Page | Description | 
 | -------- | ---- | -----------  | 
 | 8        | 0    | Temperature sensor 0 MSB (on board sensor). | 
 | 9        | 0    | Temperature sensor 0 LSB (on board sensor). | 
 | 10       | 0    | Temperature sensor 1 MSB.                   | 
 | 11       | 0    | Temperature sensor 1 LSB.                   | 
 | 12       | 0    | Temperature sensor 2 MSB.                   | 
 | 13       | 0    | Temperature sensor 2 LSB.                   | 
 | 14       | 0    | Temperature sensor 3 MSB.                   | 
 | 15       | 0    | Temperature sensor 3 LSB.                   | 
 | 16       | 0    | Temperature sensor 4 MSB.                   | 
 | 17       | 0    | Temperature sensor 4 LSB.                   | 
 | 18       | 0    | Temperature sensor 5 MSB.                   | 
 | 19       | 0    | Temperature sensor 5 LSB.                   | 

All values stored as a twos complement number as temperature * 100.

###  Report interval registers

 | Register | Page | Description | 
 | -------- | ---- | ----------- | 
 | 20       | 0    | Report interval for sensor 0 in seconds (Set to zero for no report). | 
 | 21       | 0    | Report interval for sensor 1 in seconds (Set to zero for no report). | 
 | 22       | 0    | Report interval for sensor 2 in seconds (Set to zero for no report). | 
 | 23       | 0    | Report interval for sensor 3 in seconds (Set to zero for no report). | 
 | 24       | 0    | Report interval for sensor 4 in seconds (Set to zero for no report). | 
 | 25       | 0    | Report interval for sensor 5 in seconds (Set to zero for no report). | 


###  β constant registers

 | Register | Page | Description | 
 | -------- | ---- | ----------- | 
 | 38       | 0    | B constant for sensor 0 MSB. | 
 | 39       | 0    | B constant for sensor 0 LSB. | 
 | 40       | 1    | B constant for sensor 1 MSB. | 
 | 41       | 1    | B constant for sensor 1 LSB. | 
 | 42       | 2    | B constant for sensor 2 MSB. | 
 | 43       | 2    | B constant for sensor 2 LSB. | 
 | 44       | 3    | B constant for sensor 3 MSB. | 
 | 45       | 3    | B constant for sensor 3 LSB. | 
 | 46       | 4    | B constant for sensor 4 MSB. | 
 | 47       | 4    | B constant for sensor 4 LSB. | 
 | 48       | 5    | B constant for sensor 5 MSB. | 
 | 49       | 5    | B constant for sensor 5 LSB. | 

You find the B value for a specific NTC temperature sensor in it's datasheet. The correct B value set here is essential for the temperature calculations to be correct.

You can us [this webpage](http://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm) for a 10K thermistor with an unknown beta value.


###  Low alarm registers

 | Register | Page | Description | 
 | -------- | ---- | ----------- | 
 | 50       | 0    | Low alarm set point for sensor 0 MSB. | 
 | 51       | 0    | Low alarm set point for sensor 0 LSB. | 
 | 52       | 0    | Low alarm set point for sensor 1 MSB. | 
 | 53       | 0    | Low alarm set point for sensor 1 LSB. | 
 | 54       | 0    | Low alarm set point for sensor 2 MSB. | 
 | 55       | 0    | Low alarm set point for sensor 2 LSB. | 
 | 56       | 0    | Low alarm set point for sensor 3 MSB. | 
 | 57       | 0    | Low alarm set point for sensor 3 LSB. | 
 | 58       | 0    | Low alarm set point for sensor 4 MSB. | 
 | 59       | 0    | Low alarm set point for sensor 4 LSB. | 
 | 60       | 0    | Low alarm set point for sensor 5 MSB. | 
 | 61       | 0    | Low alarm set point for sensor 5 LSB. | 

Writing any of these registers will reset high/low alarm conditions for that sensor.

The value is stored as a twos complement number representing the temperature. **Note note that the value is not *100**.

The hysteresis value affects the low alarm after a low alarm condition has been set. The temperature must increase with *hysteresis* degrees before the low alarm condition is resetted.


###  High alarm registers

 | Register | Page | Description | 
 | -------- | ---- | ----------- | 
 | 62       | 0    | High alarm set point for sensor 0 MSB. | 
 | 63       | 0    | High alarm set point for sensor 0 LSB. | 
 | 64       | 0    | High alarm set point for sensor 1 MSB. | 
 | 65       | 0    | High alarm set point for sensor 1 LSB. | 
 | 66       | 0    | High alarm set point for sensor 2 MSB. | 
 | 67       | 0    | High alarm set point for sensor 2 LSB. | 
 | 68       | 0    | High alarm set point for sensor 3 MSB. | 
 | 69       | 0    | High alarm set point for sensor 3 LSB. | 
 | 70       | 0    | High alarm set point for sensor 4 MSB. | 
 | 71       | 0    | High alarm set point for sensor 4 LSB. | 
 | 72       | 0    | High alarm set point for sensor 5 MSB. | 
 | 73       | 0    | High alarm set point for sensor 5 LSB. | 

Writing any of these registers will reset high/low alarm conditions for that sensor.

The value is stored as a twos complement number representing the temperature. **Note note that the value is not *100**.

The hysteresis value affects the high alarm after a high alarm condition has been set. The temperature must decrease with *hysteresis* degrees before the high alarm condition is resetted.

###  Sensor zone information registers

 | Register | Page | Description | 
 | -------- | ---- | ----------- | 
 | 74       | 0    | Zone for sensor 0 alarms.     | 
 | 75       | 0    | Sub zone for sensor 0 alarms. | 
 | 76       | 0    | Zone for sensor 1 alarms.     | 
 | 77       | 0    | Sub zone for sensor 1 alarms. | 
 | 78       | 0    | Zone for sensor 2 alarms.     | 
 | 79       | 0    | Sub zone for sensor 2 alarms. | 
 | 80       | 0    | Zone for sensor 3 alarms.     | 
 | 81       | 0    | Sub zone for sensor 3 alarms. | 
 | 82       | 0    | Zone for sensor 4 alarms.     | 
 | 83       | 0    | Sub zone for sensor 4 alarms. | 
 | 84       | 0    | Zone for sensor 5 alarms.     | 
 | 85       | 0    | Sub zone for sensor 5 alarms. | 

###  Sensor absolute low temperature registers

 | Register | Page | Description | 
 | -------- | ---- | ----------- | 
 | 86       | 0    | Absolute low for sensor 0 MSB (Write to reset). | 
 | 87       | 0    | Absolute low for sensor 0 LSB (Write to reset). | 
 | 88       | 0    | Absolute low for sensor 1 MSB (Write to reset). | 
 | 89       | 0    | Absolute low for sensor 1 LSB (Write to reset). | 
 | 90       | 0    | Absolute low for sensor 2 MSB (Write to reset). | 
 | 91       | 0    | Absolute low for sensor 2 LSB (Write to reset). | 
 | 92       | 0    | Absolute low for sensor 3 MSB (Write to reset). | 
 | 93       | 0    | Absolute low for sensor 3 LSB (Write to reset). | 
 | 94       | 0    | Absolute low for sensor 4 MSB (Write to reset). | 
 | 95       | 0    | Absolute low for sensor 4 LSB (Write to reset). | 
 | 96       | 0    | Absolute low for sensor 5 MSB (Write to reset). | 
 | 97       | 0    | Absolute low for sensor 5 LSB (Write to reset). | 

The value is stored as a twos complement number as temperature * 100.  Writing any of the registers will rest the value.

###  Sensor absolute high temperature registers

 | Register | Page | Description | 
 | -------- | ---- | ----------- | 
 | 98       | 0    | Absolute high for sensor 0 MSB (Write to reset). | 
 | 99       | 0    | Absolute high for sensor 0 LSB (Write to reset). | 
 | 100      | 0    | Absolute high for sensor 1 MSB (Write to reset). | 
 | 101      | 0    | Absolute high for sensor 1 LSB (Write to reset). | 
 | 102      | 0    | Absolute high for sensor 2 MSB (Write to reset). | 
 | 103      | 0    | Absolute high for sensor 2 LSB (Write to reset). | 
 | 104      | 0    | Absolute high for sensor 3 MSB (Write to reset). | 
 | 105      | 0    | Absolute high for sensor 3 LSB (Write to reset). | 
 | 106      | 0    | Absolute high for sensor 4 MSB (Write to reset). | 
 | 107      | 0    | Absolute high for sensor 4 LSB (Write to reset). | 
 | 108      | 0    | Absolute high for sensor 5 MSB (Write to reset). | 
 | 109      | 0    | Absolute high for sensor 5 LSB (Write to reset). | 

The value is stored as a twos complement number as temperature * 100.  Writing any of the registers will rest the value.

###  Sensor hysteresis registers

 | Register | Page | Description | 
 | -------- | ---- | ----------- | 
 | 110      | 0    | Hysteresis in degrees Celsius for sensor 0 (default is 5 degrees). | 
 | 111      | 0    | Hysteresis in degrees Celsius for sensor 1 (default is 5 degrees). | 
 | 112      | 0    | Hysteresis in degrees Celsius for sensor 2 (default is 5 degrees). | 
 | 113      | 0    | Hysteresis in degrees Celsius for sensor 3 (default is 5 degrees). | 
 | 114      | 0    | Hysteresis in degrees Celsius for sensor 4 (default is 5 degrees). | 
 | 115      | 0    | Hysteresis in degrees Celsius for sensor 5 (default is 5 degrees). | 

The value here will affect the high and low alarms 
### Voltage calibration registers

The Voltage used to calculate the temperature for a sensor have effect on the error. The voltage is theoretically 5V but will in reality be something else but still close to 5V. These register can be used to set a calibrated voltage obtained from actual measurements. The value stored in the two registers is the voltage times 10000 so 5V for example should be stored as 50000.

 | Register | Page | Description | 
 | -------- | ---- | ----------- | 
 | 118      | 0    | MSB of Voltage (Default is 0xC3). | 
 | 119      | 0    | MSB of Voltage (Default is 0x50). | 


## Page 1


### Raw A/D values

Raw A/D values from the temperature sensor network can be read if a user wants to. This can be useful for special nonstandard setups. The A/D converter is a 10-bit converter so the value can be 0-1023 and is stored MSB first in the registers (not in the converter) as all values are stored in VSCP. The registers can only be read, a write is ignored.

 | Register | Page | Description | 
 | -------- | ---- | ----------- | 
 | 72       | 1    | MSB of A/D converter value for sensor 0. | 
 | 73       | 1    | LSB of A/D converter value for sensor 0. | 
 | 74       | 1    | MSB of A/D converter value for sensor 1. | 
 | 75       | 1    | LSB of A/D converter value for sensor 1. | 
 | 76       | 1    | MSB of A/D converter value for sensor 2. | 
 | 77       | 1    | LSB of A/D converter value for sensor 2. | 
 | 78       | 1    | MSB of A/D converter value for sensor 3. | 
 | 79       | 1    | LSB of A/D converter value for sensor 3. | 
 | 80       | 1    | MSB of A/D converter value for sensor 4. | 
 | 81       | 1    | LSB of A/D converter value for sensor 4. | 
 | 82       | 1    | MSB of A/D converter value for sensor 5. | 
 | 83       | 1    | LSB of A/D converter value for sensor 5. | 

###  Sensor calibration registers

The value in the calibration register is added to the measured sensor value before it's reported. The calibration value is stored as a two's complement temperature * 100 to allow for both positive and negative numbers. So 24.23 degrees is stored as 2423 and so on. This means that 100 will add one degree to the measured value while 65436 (0xFF9c) is -100 and will subtract one degree form the measured value. If you are unsecyre on how two complement umbers work there is an introduction here https://www.cs.cornell.edu/~tomf/notes/cps104/twoscomp.html

 | Register | Page | Description | 
 | -------- | ---- | ----------- | 
 | 84       | 1    | MSB of calibration value for sensor 0. | 
 | 85       | 1    | LSB of calibration value for sensor 0. | 
 | 86       | 1    | MSB of calibration value for sensor 1. | 
 | 87       | 1    | LSB of calibration value for sensor 1. | 
 | 88       | 1    | MSB of calibration value for sensor 2. | 
 | 89       | 1    | LSB of calibration value for sensor 2. | 
 | 90       | 1    | MSB of calibration value for sensor 3. | 
 | 91       | 1    | LSB of calibration value for sensor 3. | 
 | 92       | 1    | MSB of calibration value for sensor 4. | 
 | 93       | 1    | LSB of calibration value for sensor 4. | 
 | 94       | 1    | MSB of calibration value for sensor 5. | 
 | 95       | 1    | LSB of calibration value for sensor 5. | 

# Alarms

 | Bit | Description | 
 | --- | ----------- | 
 | 0   | Low Alarm from one or more sensor.  | 
 | 1   | High Alarm from one or more sensor. | 
 | 2   | Reserved.                           | 
 | 3   | Reserved                            | 
 | 4   | Reserved                            | 
 | 5   | Reserved                            | 
 | 6   | Reserved                            | 
 | 7   | Reserved                            | 

Remember that alarm bits are cleared when read. So read the register to clear alarm bits. Alarm bits will be set until this register is cleared even if the actual alarm condition is not active anymore.

# Decision Matrix

This module have no decision matrix.

# Events

Here the events sent by the module is described.

## Temperature Event

 | Event | Description | 
 | ----- | ----------- | 
 | [CLASS1.MEASUREMENT (10) Type=6, Temperature](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.measurement#type_6_0x06_temperature) | If enabled this event is sent periodically for each sensor. The temperature is reported as a normalized integer value by this event. The event frequency is set in register 20-25 (default is one event per 30 seconds) and must be set to a non zero value for it to be sent.\\ \\ Temperature can be reported in one of three units. Kelvin, Celsius (default) or Fahrenheit as set in register 2-7. \\ \\ __**Data**__\\ **Byte 0** - Data coding byte. See description below.\\ **Byte 1** - Always 130 (0x82). Decimal point should be shifted two steps to the left = divide with hundred.\\ **Byte 2** - MSB of normalized integer. Two complement number.\\ **Byte 3** - LSB of normalized integer. Two complement number.\\ \\ | 

The data coding byte is the measurement data coding format described in [the VSCP specification](http://www.vscp.org/docs/vscpspec/doku.php?id=data_coding).

The normalized integer is stored as a two complement 16-bit integer. To convert

1. Calculate the 16-bit twos complement as MSB byte * 256 + LSB byte.

2. If the most significant bit is not set (Equal or less then 32767) this is a positive temperature. Divide by 100 and you have the temperature. 

3. If the most significant bit is set (Greater than 32767) this is a negative temperature. Now invert the result (the bits are inverted; 0 becomes 1, and 1 becomes 0) and add one to the result. Dive by 100 and you have the temperature.

### Example: A negative temperature in Celsius from sensor 1

The data part of the event will be

 | Byte   | Description | 
 | ----   | ----------- | 
 | Byte 0 | 130 (0x82)  | 
 | Byte 1 | 2 (0x02)    | 
 | Byte 2 | 255 (0xF0)  | 
 | Byte 3 | 216 (0x60)  | 

 1.  **Byte 0**: Normalized integer, Celsius, Sensor 1
 2.  **Byte 1**: Decimal point should be shifted two steps to the left.
 3.  **Byte 2/3**: Bit 16 is a one meaning it's a negative number. Temperature is a two complement number. 0xF060 = 0b1111000001100000. Invert which give 0b0000111110011111 add one which give 0b0000111110100000 = 4000. Divide by 100 gives 40.00 The read temperature is -40.00 ºC.

### Example: A positive temperature in Celsius from sensor 1

The data part of the event will be

 | Byte   | Description | 
 | ----   | ----------- | 
 | Byte 0 | 130 (0x82)  | 
 | Byte 1 | 2 (0x02)    | 
 | Byte 2 | 46 (0x2E)   | 
 | Byte 3 | 224 (0xE0)  | 

 1.  **Byte 0**: Normalized integer, Celsius, Sensor 1
 2.  **Byte 1**: Decimal point should be shifted two steps to the left.
 3.  **Byte 2/3**: Bit 16 is zero meaning it's a positive number. 0x2EE0 = 12000. Divide by 100 gives 120.00. The temperature is 120.00 ºC. 

### Example: A negative temperature in Fahrenheit from sensor 5

The data part of the event will be

 | Byte   | Description | 
 | ----   | ----------- | 
 | Byte 0 | 249 (0xF9)  | 
 | Byte 1 | 2 (0x02)    | 
 | Byte 2 | 255 (0xFF)  | 
 | Byte 3 | 118 (0x76)  | 

 1.  **Byte 0**: Normalized integer, Fahrenheit, Sensor 5
 2.  **Byte 1**: Decimal point should be shifted two steps to the left.
 3.  **Byte 2/3**: Bit 16 is a one meaning it's a negative number. Temperature is a two complement number. 0xFF76 = 0b1111111101110110. Invert which give 0b0000000010001001 add one which give 0b0000000010001010 = 138. Divide by 100 gives 1.38 The read temperature is -1.38 ºF.

 

### Data coding byte for Kelvin

 | Bits  | Description | 
 | ----  | ----------- | 
 | 5,6,7 | Always set to 4 (0b100) - Normalized integer format. | 
 | 3,4   | __**Unit**:__\\ **0 (0b00)** - Kelvin.\\  **1 (0b01)** – Celsius.\\  **2 (0b10)** – Fahrenheit. | 
 | 0,1,2 | __**Sensor Index**:__\\ **0 (0b000)** - Sensor 0.\\ **1 (0b001)** - Sensor 1.\\ **2 (0b010)** - Sensor 2.\\ **3 (0b011)** - Sensor 3.\\ **4 (0b100)** - Sensor 4.\\ **5 (0b101)** - Sensor 5. | 

To help to interpret data the three tables below list the datacoding bytes for Kelvin, Celsius and Fahrenheit temperature presentation.

#### Kelvin data coding table

 | Sensor   | Value for data coding byte | 
 | ------   | -------------------------- | 
 | Sensor 0 | 129 (0x81)                 | 
 | Sensor 1 | 130(0x82)                  | 
 | Sensor 2 | 131(0x83)                  | 
 | Sensor 3 | 132(0x84)                  | 
 | Sensor 4 | 133(0x85)                  | 
 | Sensor 5 | 134(0x86)                  | 


#### Celsius data coding table

 | Sensor   | Value for data coding byte | 
 | ------   | -------------------------- | 
 | Sensor 0 | 136(0x88)                  | 
 | Sensor 1 | 137(0x89)                  | 
 | Sensor 2 | 138(0x8A)                  | 
 | Sensor 3 | 139(0x8B)                  | 
 | Sensor 4 | 140(0x8C)                  | 
 | Sensor 5 | 141(0x8D)                  | 


#### Fahrenheit Vdata coding table

 | Sensor   | Value for data coding byte | 
 | ------   | -------------------------- | 
 | Sensor 0 | 244 (0xF4)                 | 
 | Sensor 1 | 245 (0xF5)                 | 
 | Sensor 2 | 246 (0xF6)                 | 
 | Sensor 3 | 247 (0xF7)                 | 
 | Sensor 4 | 248 (0xF8)                 | 
 | Sensor 5 | 249 (0xF9)                 | 


## Alarm Event

If enabled the event is sent when a temperature sensor goes below a low alarm set point (see [Low-alarm-registers](#registers)) or above a high alarm set point (see [High-alarm-registers](#registers)). The hysteresis registers is used so that after an alarm event has been sent a new alarm event is not sent until the temperature goes below/above the value set in the set point plus(low)/minus(high) signed content of hysteresis register. 

A read of the alarm register will reset the alarm status and alarm events will not be sent out again until the the temperature changed with the hysteresis amount.

 | Event | Description | 
 | -----  | ----------- | 
 | [CLASS1.ALARM, Type=2, Alarm occurred](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.alarm#type_2_0x02_alarm_occurred) | __**Data**__\\ **Byte 0** - Index. Index is 0 for Sensor 0. 1 for Sensor 1, 2 for Sensor 2. 3 for Sensor 3. 4 for Sensor 4. 5 for Sensor 5. \\ **Byte 1** - Zone.\\ **Byte 2** - Sub zone. | 

Zone and sub-zone are the modules settings for the sensor generating the alarm. See [register 74-85](#registers)

## TurnOn/TurnOff Event

If enabled, the event [CLASS1.CONTROL, Type=5, TurnOn](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.control#type_5_0x05_turnon) or [CLASS1.CONTROL, Type=6, TurnOff](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.control#type_6_0x06_turnoff) (Bit 5 in control register must be set (see [Control-registers](registers#sensor_control_ registers)])) is sent when the temperature goes below or above the low(see [Low-alarm-registers](registers#low_ alarm_registers)/high(see [High-alarm-registers](registers#high_ alarm_registers) alarm set points. Settings in the control register bit 6 (see [sub:Control-registers]) decides which of the **TurnOn/TurnOff** event that is sent. The hysteresis setting (see [sub:Sensor-hysteresis-registers]) tells how much a temperature must raise or fall below the alarm set point before a new event will be sent.

 | Event | Description | 
 | ----- | ----------- | 
 | [CLASS1.CONTROL, Type=5, TurnOn](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.control#type_5_0x05_turnon) | __**Data**:__\\ **0** Index,\\ **1** Zone.\\ **2** Sub zone. | 

 | Event | Description | 
 | ----- | ----------- | 
 | [CLASS1.CONTROL, Type=6, TurnOff](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.control#type_6_0x06_turnoff) | __**Data**:__\\ **0** Index,\\ **1** Zone.\\ **2** Sub zone. | 

__**Index**__ is 0 for Sensor 0, 1 for Sensor 1, 2 for Sensor 2. 3 for Sensor 3. 4 for Sensor 4. 5 for Sensor 5. 

__**Zone**__ information is fetched from sensor zone register (see [sensor zone information registers](http://www.grodansparadis.com/kelvinntc10k/manual/doku.php?id=registers#sensor_zone_information_registers)). 

__**Sub zone**__ information is fetched from sensor sub-zone register (see [sensor zone information registers](http://www.grodansparadis.com/kelvinntc10k/manual/doku.php?id=registers#sensor_zone_information_registers)).

## Sync. Event (incoming)

If a SYNC event is received by the module it will check the zone/subzone parameters of the event and send out [temperature measurement event(s)](http://www.grodansparadis.com/kelvinntc10k/manual/doku.php?id=events#temperature_event) for all sensors that match. This can be a handy feature to use of one want synchronized data from several sources.


 | Event | Description | 
 | ----- | ----------- | 
 | [CLASS1.CONTROL, Type=26, Sync](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.control#type_26_0x1a_sync) | __**Data**:__\\ **0** Index,\\ **1** Zone.\\ **2** Sub zone. | 

 # Recipes


*  **The story about an old Fridge**: [http://grodansparadis.com/wordpress/?p=1384](https://www.grodansparadis.com/wordpress/?p=1384)

# faq - Frequently Asked Questions

*Not many of those yet!*





