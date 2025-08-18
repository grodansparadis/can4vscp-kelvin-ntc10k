# Events

Here the events sent by the module is described.

 ====== Events ======

Here the events sent by the module is described.

===== Temperature Event =====

^ Event ^ Description ^
|  [[http://www.vscp.org/docs/vscpspec/doku.php?id=class1.measurement#type_6_0x06_temperature|CLASS1.MEASUREMENT (10) Type=6, Temperature]]  | If enabled this event is sent periodically for each sensor. The temperature is reported as a normalized integer value by this event. The event frequency is set in register 20-25 (default is one event per 30 seconds) and must be set to a non zero value for it to be sent.\\ \\ Temperature can be reported in one of three units. Kelvin, Celsius (default) or Fahrenheit as set in register 2-7. \\ \\ __**Data**__\\ **Byte 0** - Data coding byte. See description below.\\ **Byte 1** - Always 130 (0x82). Decimal point should be shifted two steps to the left = divide with hundred.\\ **Byte 2** - MSB of normalized integer. Two complement number.\\ **Byte 3** - LSB of normalized integer. Two complement number.\\ \\  |

The data coding byte is the measurement data coding format described in [[http://www.vscp.org/docs/vscpspec/doku.php?id=data_coding|the VSCP specification]].

The normalized integer is stored as a two complement 16-bit integer. To convert

1. Calculate the 16-bit twos complement as MSB byte * 256 + LSB byte.

2. If the most significant bit is not set (Equal or less then 32767) this is a positive temperature. Divide by 100 and you have the temperature. 

3. If the most significant bit is set (Greater than 32767) this is a negative temperature. Now invert the result (the bits are inverted; 0 becomes 1, and 1 becomes 0) and add one to the result. Dive by 100 and you have the temperature.

==== Example: A negative temperature in Celsius from sensor 1 ====

The data part of the event will be

^ Byte ^ Description ^
| Byte 0 | 130 (0x82) |
| Byte 1 | 2 (0x02) |
| Byte 2 | 255 (0xF0) |
| Byte 3 | 216 (0x60) |

  - **Byte 0**: Normalized integer, Celsius, Sensor 1
  - **Byte 1**: Decimal point should be shifted two steps to the left.
  - **Byte 2/3**: Bit 16 is a one meaning it's a negative number. Temperature is a two complement number. 0xF060 = 0b1111000001100000. Invert which give 0b0000111110011111 add one which give 0b0000111110100000 = 4000. Divide by 100 gives 40.00 The read temperature is -40.00 ºC.

==== Example: A positive temperature in Celsius from sensor 1 ====

The data part of the event will be

^ Byte ^ Description ^
| Byte 0 | 130 (0x82) |
| Byte 1 | 2 (0x02) |
| Byte 2 | 46 (0x2E) |
| Byte 3 | 224 (0xE0) |

  - **Byte 0**: Normalized integer, Celsius, Sensor 1
  - **Byte 1**: Decimal point should be shifted two steps to the left.
  - **Byte 2/3**: Bit 16 is zero meaning it's a positive number. 0x2EE0 = 12000. Divide by 100 gives 120.00. The temperature is 120.00 ºC. 

==== Example: A negative temperature in Fahrenheit from sensor 5 ====

The data part of the event will be

^ Byte ^ Description ^
| Byte 0 | 249 (0xF9) |
| Byte 1 | 2 (0x02) |
| Byte 2 | 255 (0xFF) |
| Byte 3 | 118 (0x76) |

  - **Byte 0**: Normalized integer, Fahrenheit, Sensor 5
  - **Byte 1**: Decimal point should be shifted two steps to the left.
  - **Byte 2/3**: Bit 16 is a one meaning it's a negative number. Temperature is a two complement number. 0xFF76 = 0b1111111101110110. Invert which give 0b0000000010001001 add one which give 0b0000000010001010 = 138. Divide by 100 gives 1.38 The read temperature is -1.38 ºF.

 

==== Data coding byte for Kelvin ====

^ Bits ^ Description ^
| 5,6,7	| Always set to 4 (0b100) - Normalized integer format. |
| 3,4 | __**Unit**:__\\ **0 (0b00)** - Kelvin.\\  **1 (0b01)** – Celsius.\\  **2 (0b10)** – Fahrenheit. |
| 0,1,2	| __**Sensor Index**:__\\ **0 (0b000)** - Sensor 0.\\ **1 (0b001)** - Sensor 1.\\ **2 (0b010)** - Sensor 2.\\ **3 (0b011)** - Sensor 3.\\ **4 (0b100)** - Sensor 4.\\ **5 (0b101)** - Sensor 5. |

To help to interpret data the three tables below list the datacoding bytes for Kelvin, Celsius and Fahrenheit temperature presentation.

=== Kelvin data coding table ===

^ Sensor ^ Value for data coding byte ^
| Sensor 0 | 129 (0x81) |
| Sensor 1 | 130(0x82) |
| Sensor 2 | 131(0x83) |
| Sensor 3 | 132(0x84) |
| Sensor 4 | 133(0x85) |
| Sensor 5 | 134(0x86) |


=== Celsius data coding table ===

^ Sensor ^ Value for data coding byte ^
| Sensor 0 | 136(0x88) |
| Sensor 1 | 137(0x89) |
| Sensor 2 | 138(0x8A) |
| Sensor 3 | 139(0x8B) |
| Sensor 4 | 140(0x8C) |
| Sensor 5 | 141(0x8D) |


=== Fahrenheit Vdata coding table ===
^ Sensor ^ Value for data coding byte ^
| Sensor 0 | 244 (0xF4) |
| Sensor 1 | 245 (0xF5) |
| Sensor 2 | 246 (0xF6) |
| Sensor 3 | 247 (0xF7) |
| Sensor 4 | 248 (0xF8) |
| Sensor 5 | 249 (0xF9) |


===== Alarm Event =====

If enabled the event is sent when a temperature sensor goes below a low alarm set point (see [[http://www.grodansparadis.com/kelvinntc10k/manual/doku.php?id=registers|Low-alarm-registers]]) or above a high alarm set point (see [[http://www.grodansparadis.com/kelvinntc10k/manual/doku.php?id=registers|High-alarm-registers]]). The hysteresis registers is used so that after an alarm event has been sent a new alarm event is not sent until the temperature goes below/above the value set in the set point plus(low)/minus(high) signed content of hysteresis register. 

A read of the alarm register will reset the alarm status and alarm events will not be sent out again until the the temperature changed with the hysteresis amount.

^ Event ^ Description ^
|  [[http://www.vscp.org/docs/vscpspec/doku.php?id=class1.alarm#type_2_0x02_alarm_occurred|CLASS1.ALARM, Type=2, Alarm occurred]]  | __**Data**__\\ **Byte 0** - Index. Index is 0 for Sensor 0. 1 for Sensor 1, 2 for Sensor 2. 3 for Sensor 3. 4 for Sensor 4. 5 for Sensor 5. \\ **Byte 1** - Zone.\\ **Byte 2** - Sub zone. |

Zone and sub-zone are the modules settings for the sensor generating the alarm. See [[http://www.grodansparadis.com/kelvinntc10k/manual/doku.php?id=registers|register 74-85]]

===== TurnOn/TurnOff Event =====

If enabled, the event [[http://www.vscp.org/docs/vscpspec/doku.php?id=class1.control#type_5_0x05_turnon|CLASS1.CONTROL, Type=5, TurnOn]] or [[http://www.vscp.org/docs/vscpspec/doku.php?id=class1.control#type_6_0x06_turnoff|CLASS1.CONTROL, Type=6, TurnOff]] (Bit 5 in control register must be set (see [[registers#sensor_control_ registers|Control-registers]]])) is sent when the temperature goes below or above the low(see [[registers#low_ alarm_registers|Low-alarm-registers]]/high(see [[registers#high_ alarm_registers|High-alarm-registers]] alarm set points. Settings in the control register bit 6 (see [sub:Control-registers]) decides which of the **TurnOn/TurnOff** event that is sent. The hysteresis setting (see [sub:Sensor-hysteresis-registers]) tells how much a temperature must raise or fall below the alarm set point before a new event will be sent.

^ Event ^ Description ^
|  [[http://www.vscp.org/docs/vscpspec/doku.php?id=class1.control#type_5_0x05_turnon|CLASS1.CONTROL, Type=5, TurnOn]]  | __**Data**:__\\ **0** Index,\\ **1** Zone.\\ **2** Sub zone. |

^ Event ^ Description ^
|  [[http://www.vscp.org/docs/vscpspec/doku.php?id=class1.control#type_6_0x06_turnoff|CLASS1.CONTROL, Type=6, TurnOff]] | __**Data**:__\\ **0** Index,\\ **1** Zone.\\ **2** Sub zone. |

__**Index**__ is 0 for Sensor 0, 1 for Sensor 1, 2 for Sensor 2. 3 for Sensor 3. 4 for Sensor 4. 5 for Sensor 5. 

__**Zone**__ information is fetched from sensor zone register (see [[http://www.grodansparadis.com/kelvinntc10k/manual/doku.php?id=registers#sensor_zone_information_registers|sensor zone information registers]]). 

__**Sub zone**__ information is fetched from sensor sub-zone register (see [[http://www.grodansparadis.com/kelvinntc10k/manual/doku.php?id=registers#sensor_zone_information_registers|sensor zone information registers]]).

===== Sync. Event (incoming) =====

If a SYNC event is received by the module it will check the zone/subzone parameters of the event and send out [[http://www.grodansparadis.com/kelvinntc10k/manual/doku.php?id=events#temperature_event|temperature measurement event(s)]] for all sensors that match. This can be a handy feature to use of one want synchronized data from several sources.


^ Event ^ Description ^
|  [[http://www.vscp.org/docs/vscpspec/doku.php?id=class1.control#type_26_0x1a_sync|CLASS1.CONTROL, Type=26, Sync]]  | __**Data**:__\\ **0** Index,\\ **1** Zone.\\ **2** Sub zone. |


  
[filename](./bottom-copyright.md ':include')
