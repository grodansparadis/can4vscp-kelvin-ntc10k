
# Manual for the Kelvin NTC10K module

**Document version:** ${/var/document-version} - ${/var/creation-time}
[HISTORY](./history.md)


![Paris](/images/kelvin10.png)

The Kelvin NTC10KA module measures temperature. It measures temperatures in a wide range (depending on what sensors are used) and can achieve 0.1C accuracy with calibration. It does so by allowing connection of up to five external NTC temperature sensors and by measuring the temperature on the surface of the module itself. The measured temperature can be reported to other CAN4VSCP modules or to one or several servers so it can be collected in a database or visualized and analyzed in a diagram. The accuracy of the sensors can be increase further by calibration.

Temperature data can be presented by the module in Kelvin, Celsius or Fahrenheit.

Temperatures is reported automatically and it is possible to set alarms for high/low levels. Absolute low and high temperatures are collected and it is easy to construct autonomous and reliable temperature regulation systems.

NTC sensors has the advantage of being low cost, possible to get high accuracy from and ease of use with long cable runs. No calibration is usually needed if +-1 degree accuracy is OK.

The module fully adopts to the CAN4VSCP specification and can be powered directly over the bus with a 9-28V DC power source. It has a rich register set for configuration and any information events defined. It also have a decision matrix for easy dynamic event handling.

VSCP CAN modules are designed to work on a CAN4CAN bus which use ordinary RJ-45 connectors and is powered with 9-28V DC over the same cable. This means there is no need for a separate power cable. All that is needed is a CAT5 or better twisted pair cable. Buss length can be a maximum of 500 meters with drops of maximum 24 meters length (up to a total of 120 meters). As for all CAN4VSCP modules the communication speed is fixed at 125 kbps.

All VSCP modules contains information of there own setup, manual, hardware version, manufacturer etc. You just ask the module for the information you need and you will get it. When they are started up they have a default functionality that often is all that is needed to get a working setup. If the module have something to report it will send you an event and if it is setup to react on a certain type of event it will do it's work when you send event(s) to it. 

  * [GitHub repository for the module](https://github.com/grodansparadis/can4vscp_kelvin_ntc10k)
  * This manual is available [here](https://grodansparadis.github.io/can4vscp_kelvin_ntc10k/)
  * Latest KiCad schema for the module is available [here](https://github.com/grodansparadis/can4vscp_kelvin_ntc10k/tree/master/kicad)
  * Latest firmware for the module is available [here](https://github.com/grodansparadis/can4vscp_kelvin_ntc10k/tree/master/firmware)
  * [MDF for the module](https://github.com/grodansparadis/can4vscp_kelvin_ntc10k/tree/master/mdf)

## VSCP

![VSCP logo](./images/logo_100.png)

VSCP is a free and open automation protocol for IoT and m2m devices. Visit [the VSCP site](https://www.vscp.org) for more information.

**VSCP is free.** Placed in the **public domain**. Free to use. Free to change. Free to do whatever you want to do with it. VSCP is not owned by anyone. VSCP will stay free and gratis forever.

The specification for the VSCP protocol is [here](https://docs.vscp.org) 

VSCP documentation for various parts can be found [here](https://docs.vscp.org/).

If you use VSCP please consider contributing resources or time to the project ([https://github.com/sponsors/grodansparadis](https://github.com/sponsors/grodansparadis)).

## Document license

This document is licensed under [Creative Commons BY 4.0](https://creativecommons.org/licenses/by/4.0/) and can be freely copied, redistributed, remixed, transformed, built upon as long as you give credits to the author.



[filename](./bottom-copyright.md ':include')
