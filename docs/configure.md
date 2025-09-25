# Configure the module

You configure a VSCP module by writing content into the modules registers. You can do this manually or with the wizard available in VSCP works. Using the wizard is absolutely the easiest way to accomplish different things.

## Sensor calibration

Thermistors are very accurate but non linear devices. Still you may find that Kelvin NTC10K show you the wrong value of the measured temperature. This is because the sensor often need calibration or that registers need to be set to correct values. With calibration you can get very accurate results. For the best result you typically calibrate at three points, in ice + water mix, at 25 degrees Celsius and in boiling water. For most cases one calibration point will do but it all depends on what needs you have.

There is several registers that affect the reported temperature value. First of all there is the(Beta value registers)[registers.md?id=B2_constant_registers]. The value in these registers should be the Beta value for the sensor (and the range you use). Alternatively a value you get form calibration as of above.

The next value is the [[registers#voltage_calibration_registers|voltage calibration value]]. This value is ten thousand times the voltage of the Kelvin NTC board. This is thus normally 50000 (for 5V) which is 0xC350 in hexadecimal. If the voltage of your board is not 5.00 volts you can enter the correct value here. This value seldom need a change.

The last and most important place are the  [[registers#sensor_calibration_registers|sensor calibration registers]] which can be used to set up a calibration value for each sensor that will be added or subtracted from the read value before it is reported. 

Values are stored as [twos complement numbers](https://www.cs.cornell.edu/~tomf/notes/cps104/twoscomp.html) and is as usual 100 times the actual value.

For example your sensor shows a temperature that is 1.4 degrees to low. 1.4 time 100 is 140 which is 0x008C in hexadecimal. So store 0x00 in the MSB byte and 0x8C in the LSB byte of the calibration value for that sensor. Now 1.4 degrees will be added to the measured value before it is reported and used in calculations.

Another sample is if your sensor measure a value that is 1.9 degrees to high. We therefore need to subtract 1.9. 100 * -1.9 = -190. This is  0xFF42 as a twos complement number  (190 =   0x00BE (0000 0000 1011 1110), Invert which give 0xFF41 (1111 1111 0100 0001), add one which gives 0cxFF42).

![](./images/calibration.jpg)

A good method to use to calibrate sensors is to mix ice and water and put the sensor in the mix. This will be zero degrees Celsius. To get another point boil some water and measure the temperature in the middle of the container without touching the walls.

A good tool for calibration is available on [this page](http://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm) where you can calculate the Beta value for a sensor from two calibration points.


## Zones and sub zones

Zones and sub zones are used to group things together. Normally you don't address events to a certain receiver in VSCP ([CLASS1.PROTOCOL](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.protocol) being the exception that prove this rule). Instead you report/send things to a group of modules defined by a zone and a sub zone. The sender have no knowledge of how many units this zone/sub zone consist of. Well it does not even care.

Zone and sub zone for the module can be set in the  (module zone registers)[./registers?id=module-zone-registers]. But more useful than the module zone sub zone is the zone information for each individual sensor that is available in the (sensor zone registers)[./registers?id=sensor-zone-information-registers]. Here you can specify the zone/sub zone units you think is interested in this event. You can also set them both (or one of them) to 0xff meaning all zones, sub zones.

Think of a zone as a house, floor plan or similar and sub-zone as a room or a location.  Or let the MSB bits of the zone be the floor plan, the LSB bits of the zone be the room on that floor plan and the sub zone be a group of functionality for that room. Note again that this is not an address. It's just a way to group functionality together.

It is good to think this through before you start to configure a system with VSCP modules.

## β constants 

In registers (β constants)[./registers?id=β-constant-registers] you have registers that hold β (beta) values for each sensor. To get correct temperature readings it is important that you enter the correct value here for the sensor you use. You find this information in the data sheet for the sensor.

## Sensor Control Registers 
In registers [[registers#description_of_control_register_bits|0:2 - 0:7]] you can configure how sensor data is reported and acted up on. You can set the unit you want temperatures to be reported in. You can enable alarms. You can define that TurnOn/TurnOff events should be sent when levels have been reached or data has been found to be below a certain level. And other things. 

## Temperature measurements 
If you have gone through the steps above you can now read temperature data in [[registers#temperature_registers|registers 0:8 - 0:19]]. The temperature is shown in the selected unit as an integer which is temperature * 100. 

The temperature registers is located in pairs (Most significant byte followed my the least significant byte) and they can be read to get the current temperature for each sensor. The temperature is stored as a 16-bit two complement of a normalized temperature. This means you have to divide the read temperature by 100 to get the temperature value. 

To get the temperature and calculate the temperature for a specific sensor. 

* Read MSB register. Read LSB register.
* Calculate the 16-bit twos complement as MSB reading * 256 + LSB reading.
* If the most significant bit is not set (Equal or less then 32767) this is a positive temperature. Divide by 100 and you have the temperature. 
* If the most significant bit is set (Greater than 32767) this is a negative temperature. Now invert the result (the bits are inverted; 0 becomes 1, and 1 becomes 0) and add one to the result. Dive by 100 and you have the temperature.

However usually this is not the way to get temperature readings. Instead you set a report interval for a sensor expressed in seconds in the [report interval register](./registers#report_interval_registers) (or set to zero to turn of reporting) and get temperature readings automatically delivered/reported.

After entering a value here [temperature events](./events#temperature_event) will be sent with the configured interval. The receiver of this information can be another module, a routine that writes them to a database or a websocket widget that diagram them live on a screen or on a web page.

## Alarms

In the [low alarm register](registers#low_alarm_registers) and in the [high alarm registers](registers#high_alarm_registers) registers you can set the levels at which [alarm events](events#alarm_event) are sent.

An activated low alarm event (activated by setting bit 3 in the [control register](registers#description_of_control_register_bits)) is sent when the temperature is going below the low alarm value. When this happens the low alarm bit in the `standard alarm register` is set. This bit will only be rested by reading the alarm register. The alarm is considered active as long as the temperature is below the low alarm set point + the hysteresis value set in [sensor hysteresis registers](registers#sensor_hysteresis_registers). So if temperature moves above this point a new alarm event will be sent again when temperature again is moving below the set point.

An activated high alarm event (activated by setting bit 4 in the [control register](registers#description_of_control_register_bits)) is sent when the temperature is going above the high alarm value. When this happens the high alarm bit in the `standard alarm register` is set. This bit will only be rested by reading the alarm register. The alarm is considered active as long as the temperature is above the low alarm set point - the hysteresis value set in [sensor hysteresis registers](registers#sensor_hysteresis_registers). So if temperature moves below this point a new alarm event will be sent again when temperature again is moving above the set point.

It is possible to have continuous alarms events sent by setting bit 7 in the [control register](registers#description_of_control_register_bits). An alarm event will be sent every second while the alarm condition persist.

## Controlling things 

Instead of alarms [CLASS1.CONTROL, TurnOn](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.control?id=type5) and [CLASS1.CONTROL, TurnOff](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.control?id=type6) events can be sent.  This functionality is activated by setting **bit 5** in the [control registers](./registers?id=sensor-control-registers). 

Another bit, bit 6, in the control register decides when the TurnOff/TurnOn events are sent. If set to zero TurnOff is sent when the high point is reached and TurnOn when the low point is reached. Suitable for control of a heating system for example. If the bit is set the TurnOn event is sent at the high set point and the TurnOff is sent on the low set point. Suitable for a cooling system.

![](./images/limits.png)

The picture shows a real world example where a Kelvin NTC10K module measures temperature in a fridge which have its compressor controlled by a [[http://www.grodansparadis.com/paris/paris.html|Paris relay module]]. The [Paris decision matrix](http://grodansparadis.github.io/can4vscp-paris/#/./decisionmatrix) is programmed to turn on the compressor when TurnOn events is received from the the zone/subzone programmed into the Kelvin NTC10K module sensor that measure the fridge temperature.

So here TurnOff events are sent by the Kelvin NTC10K module when the temperature go below -22 degrees Celsius. This event is received by the Paris module and it turn of the relay that controls the fridge compressor. Continuous mode is activated so the TurnOff events are sent until the temperature reach -20 degrees Celsius. A function to protect against events not being delivered.

When the temperature goes up above -20 degrees Celsius no events will be sent until the temperature go above -17 degrees Celsius. At this point TurnOn events will be sent which are received by the Paris module which turn on the relay that control the Fridge compressor. TurnOn events are sent continuously until the temperature go down to -19 degrees Celsius and again there will be a dead zone until the temperature falls to -22 degrees Celsius.

## Some samples

<html>
<iframe width="420" height="315" src="https://www.youtube.com/embed/37PQomplUVE" frameborder="0" allowfullscreen></iframe>
</html>

<html>
<iframe width="420" height="315" src="https://www.youtube.com/embed/L17d1SSyBc0" frameborder="0" allowfullscreen></iframe>
</html>

<html>
<iframe width="560" height="315" src="https://www.youtube.com/embed/qV41vWt5toE" frameborder="0" allowfullscreen></iframe>
</html>

  
[filename](./bottom-copyright.md ':include')