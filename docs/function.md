

## Configure the module

You configure a VSCP module by writing content into the modules
registers. You can do this manually or with the wizard available in
[VSCP Works+](https://grodansparadis.github.io/vscp-works-qt/#/).
Using the wizard is absolutely the easiest way to set up functionality.

## Zone/sub-zone

You should always plan your overall structure. The zone and the sub-zone registers found in the first two register positions can help you here. Think of a zone as a (logical) house,floor plan or similar and sub-zone as a room or a location.

Note that this is not an address. It\'s a way to group functionality together.

Each temperature sensor can belong to it\'s own sub-zone or to a common sub-zone.

## Logging temperature

The module can automatically log temperature data and send it to other modules or to a remote server for further processing. This is easy to setup, you just set the interval you want and temperature eventes will be delevered periodically from the module. See the [register section](./registers.md?id=report-interval-registers).

## Alarm when temperature is out of range
To protect sensitive equipment or to control heating or cooling you can set up alarms for high and low temperature. When a temperature sensor goes above or below a set value an alarm event is sent. You can set the alarm limits in the [low alarm registers](./registers.md?id=low-alarm-registers) and in the [high alarm registers](./registers.md?id=high-alarm-registers).

Now activating high or low or both in the [control register](http://localhost:3000/#/./registers?id=sensor-control-registers) for the sensor will make the module send out alarm events when the temperature goes above or below the set value.

The [hysteresis](./registers?id=sensor-hysteresis-registers) registers can be used to set a hysteresis value for the alarm. This is the value the temperature must go below or above the set value before the alarm is cleared.

The module can even send events that turn och heatingcontrollers instead of alarm events. Just set bit 5 of the [control register](./registers?id=sensor-control-registers) and CLASS1_CONTROL, Turn ON  will be sent when the low set point is reached (minus hysteresis) and CLASS1_CONTROL, Turn OFF events will be sent when the high setpoint is reached. Setting bit 7 will make theese events continious.

## Cooling

If you want to control a cooling system. Just set the cooling bit in the [control register](./registers?id=sensor-control-registers) and the module will send out [CLASS1_CONTROL, Turn ON events](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.control?id=type5) when the high setpoint is reached and [CLASS1_CONTROL, Turn OFF events](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.control?id=type6) when the low setpoint is reached. Setting bit 7 will make theese events continious, and events will be sent out every second.

Typically the sent events are feed to the [decision matrix](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_decision_matrix) of another module (or several) that do the action of turning on or off cooling equipment and thereby creating a self contained control system.

## Alarm

The module can send alarm events if the temperatur for a sensor goes above or below a set temperature value. When an alarm occurs a bit is set in the alarm register which is located in standard register position 128/0x80. You can read this register to see if the module have sent out any alarm events. When you read the register the alarm bits in the alarm register will be cleared.

  
[filename](./bottom-copyright.md ':include')
