![](./images/opensourcehw-100x82.png)

-----

This project is part of the <a href="http://www.vscp.org">VSCP (Very Simple Control Protocol) project</a>. 

<hr>

# Kelvin NTC10K

This module is part of the [VSCP project](https://www.vscp.org).  It is free to use, modify and sell. The only thing we kindly ask is that improvements and extensions are contributed back to the project (at your will). This to make the project better for everyone. All design files is licensed under the [MIT license](https://en.wikipedia.org/wiki/MIT_License).

The full documentation for the module can be found [here](http://grodansparadis.github.io/can4vscp-kelvin-ntc10k/#/).

![](./images/kelvin11.png)

In this repository you find all development files for the Kelvin NTC10K module. The project together with the [Paris Relay Module](https://github.com/grodansparadis/can4vscp-paris) is the main examples for a VSCP implemention on a Microchip 18F PIC device.

## Abstract

Kelvin NTC10K is a temperature module that can handle six temperature sensors. One internal and five external.

The Kelvin NTC10KA module measures temperature. It measures temperatures in a wide range (depending on what sensors are used) and can achieve 0.1C accuracy with calibration. It does so by allowing connection of up to five external NTC temperature sensors and by measuring the temperature on the surface of the module itself. The measured temperature can be reported to other CAN4VSCP modules or to one or several servers so it can be collected in a database or visualized and analyzed in a diagram. The accuracy of the sensors can be increase further by calibration.

Temperature data can be presented by the module in Kelvin, Celsius or Fahrenheit.

Temperatures is reported automatically and it is possible to set alarms for high/low levels. Absolute low and high temperatures are collected and it is easy to construct autonomous and reliable temperature regulation systems.

NTC sensors has the advantage of being low cost, possible to get high accuracy from and ease of use with long cable runs. No calibration is usually needed if +-1 degree accuracy is OK.

The module fully adopts to the CAN4VSCP specification and can be powered directly over the bus with a 9-28V DC power source. It has a rich register set for configuration and any information events defined. It also have a decision matrix for easy dynamic event handling.

VSCP CAN modules are designed to work on a CAN4CAN bus which use ordinary RJ-45 connectors and is powered with 9-28V DC over the same cable. This means there is no need for a separate power cable. All that is needed is a CAT5 or better twisted pair cable. Buss length can be a maximum of 500 meters with drops of maximum 24 meters length (up to a total of 120 meters). As for all CAN4VSCP modules the communication speed is fixed at 125 kbps.

All VSCP modules contains information of there own setup, manual, hardware version, manufacturer etc. You just ask the module for the information you need and you will get it. When they are started up they have a default functionality that often is all that is needed to get a working setup. If the module have something to report it will send you an event and if it is setup to react on a certain type of event it will do it's work when you send event(s) to it. 

---



## Project files

### User manual
  * [User Manual](https://grodansparadis.github.io/can4vscp-kelvin-ntc10k/#)

### Schematic, PCB, 3D files etc
  * [Schematics reversion A](https://raw.githubusercontent.com/grodansparadis/can4vscp_accra/refs/heads/master/eagle/accra_sch_rev_A.png)
 * Hardware design files is made in [KiCad](https://kicad.org) and can be found in the `kicad` directory. Valid from reversion B of the hardware.
   * Gerber files for PCB production can be found in the `gerber` directory (in the `kicad` folder).
 * Eagle schema and board files for reversion A and B can be found in the `eagle` directory. They are no longer actively updated.

 ### Firmware

 The firmware is developed in [MPLAB X IDE](https://www.microchip.com/mplab/mplab-x-ide) using the [XC8 compiler](https://www.microchip.com/mplab/compilers).

  * Binary release files is available [here](https://github.com/grodansparadis/can4vscp-kelvin-ntc10k/releases)

### MDF - Module Description File(s)
  * [MDF file version: 1.2.0 Release date: 2015-02-18](http://www.eurosource.se/ntc10KA_3.xml)
  * [MDF file version: 1.1.0 Release date: 2014-12-24](http://www.eurosource.se/ntc10KA_2.xml)
  * [MDF file version: 1.0.0 Release date: 2012-02-24](http://www.eurosource.se/ntc10KA_1.xml)

### Support
If you need support, please open an issue in the [GitHub repository](https://github.com/grodansparadis/can4vscp-kelvin-ntc10k/issues).

### Buy a ready made modules
You can buy a ready made module from [Grodans Paradis](http://www.grodansparadis.com).

### Project related links
  * [VSCP project](https://www.vscp.org)
  * [VSCP Documentation site](https://docs.vscp.org/)
  * [VSCP Wiki](https://github.com/grodansparadis/vscp/wiki)


## Steps you should go through to adopt this file for your own VSCP project.

  * The frequency for your system may be different. This will affect the CAN bit rate settings which are defined in the biginning of the can18f.c file. Here you also find the filter/mask  defines if you need to change these. Timer0 is used as a 1 ms timer for the VSCP functionality. You should adopt the timer reload value in the main.h file to your system.

  * The GUID (Glocal Unique ID) is defined in the beginning of vscp.c This number should be different for each piece of equiment you produce. Regard it is as the device serial number. It can be moved to EEPROM if that is found more convinient. In that case the vscp_rcv_readreg  procedure in vscp.c also has to be changed.

  * The device URL is defined in the beginning of the vscp.c file. This is an URL without the initial "http://" that should point to a XML file that give information about your piece of equipment.

  * The manufacturer id is also defined in the beginning of the vscp.c file. You can use this id in any way you like.

In addition to this you may use different pins for the INIT button (RC0 today) and the status LED (RC1) today. Changes affect the main interrupt vector.