# ESPHome - FCHAO off-grid Inverter

ESPHome integration to monitor and switch ON//OFF a FCHAO inverter via RS485/RJ-45 port.

The inverter RS485 port consist of a communication part (RS485) and a pulled up dry contact to switch ON/OFF the inverter.

A MAX485 module and a level shifter are required.

Exposed components:
- ON/OFF Switch
- Power
- AC Voltage
- DC Voltage
- Temperature
- Overload protection triggered

Choose your GPIO pins and write them in the yaml. ```inverter_flow_pin``` is needed if your MAX485 module doesn't automatically manage TX/RX (i.e. it exposes DE/RE pins).

```yaml
substitutions:
  inverter_tx_pin: GPIO26
  inverter_rx_pin: GPIO25
  inverter_switch_pin: GPIO13
  inverter_flow_pin: GPIO14
```

Complete the WiFi and ESPHome configuration parameters.

Tested on the 3000W 24V off-grid FCHAO model.

This integration emulate the request made by the external display, which sends always the same packet. The request packet could vary depending on the inverter variant.

This integration is meant to work as a replace of the external native display. The integrated display will work properly. See suggestions below to adapt this integration to work without removing the external display (not implemented).

The ON/OFF switch requires a circuit with a transistor. However, the user can use the integration ignoring the switch, control switching manually ON/OFF via the hardware button in the inverter case.

This project is licensed under the MIT License. This project is provided as-is, with no guarantees of any kind. Use of this repository and its contents is entirely at your own risk.

## Wiring

Inverter RS485/RJ-45 port pins:
- 1, 2: GND
- 3, 5: dry contact - switch ON/OFF - (pull up 24V in 24V variant)
- 4, 6: not used by the integration (probably a power supply for the native external display)
- 7: RS485 A
- 8: RS485 B


```
    Inverter port
       ┌─────┐
     ┌─┘     └─┐
 ┌───┘         └───┐ 
 │                 │ 
 │ 1 2 3 4 5 6 7 8 │ 
 │ | | | | | | | | │ 
 └─────────────────┘
```

- **Sensor monitor**: 

The RS485 A and RS485 B pins should be connect to a MAX485 module converter, and then to level shifter, connected to RX/TX ESP32 pins.

I tested both MAX485 with and without RE/DE pins. In my experience MAX485 with DE/RE pins is in general more reliable, however, the second variant seems to work too. If you use the module variant with DE/RE pins, your should connect together these pins, and connect them through the level shifter to the chosen ESP32 ```inverter_flow_pin```. If you use the module variant without DE/RE pins, just choose a random ```inverter_flow_pin```, or remove it everywhere.

The level shifter is necessary because MAX485 is 5V rated, while ESP32 is 3.3V rated. If the inverter and the ESP32 has common ground (example: battery supplies power to the inverter and to the ESP32 through a buck converter), the inverter GND communication side connection is not necessary, however the remaining GND connections in the schema below are required.

```
                                               ^
┌──────────┐                 ┌──────────┐      |      ┌─────────┐                ┌─────────┐ 
│          │ <-- RS485 A --> │          │ <-- 5V ---> │         │ <-- 3.3V ----> │         │
│ INVERTER │ <-- RS485 B --> │  MAX485  │ <-- TX ---> │  Level  │ <--- TX -----> │  ESP32  │
│          │                 │          │ <-- RX ---> │ shifter │ <--- RX -----> │         │
│          │                 │          │ <- DE/RE -> │         │ <- flow pin -> │         │
│          │ <---- GND ----> │          │ <-- GND --> │         │ <-- GND -----> │         │
└──────────┘        │        └──────────┘      │      └─────────┘      │         └─────────┘
                    └───────────────────────── └───────────────────────┘
```

My experiments showed that trying to use MAX485 with a 3.3V supply in order to remove the level shifter leads to invalid packets. The MAX3485 variant should be 3.3V rated, however I didn't test it.

- **Switch ON/OFF control**:

Use a NPN transistor in order to control the pulled up dry contact with an ESP32 GPIO pin.

In the 24V inverter variant the dry contact is pulled up at 24V. Probably the pull up voltage always corresponds to the DC inverter input. Choose a proper transistor, rated for this collector (C) voltage.

```
┌──────────┐                     ┌─────────┐                         ┌─────────┐ 
│          │ Dry contact <---> C │         │ B <- 10 kOhm ----> GPIO │         │
│ INVERTER │                     │   NPN   │                |        │  ESP32  │
│          │                     │         │             10 kOhm     │         │ 
│          │                     │         │                |        │         │ 
│          │ GND <-----------> E │         │ E <---------------> GND │         │ 
└──────────┘                     └─────────┘                         └─────────┘
```

## Docs

Status request packet

```0xAE 0x01 0x01 0x03 0x05 0xEE```

Status response packet

```
0xAE 0x01 0x12 0x83 | 0x02 0x31 | 0x34 0x37 | 0x02 0x54 |  0x00 0x27  | 0x00 |  0x42 |  0x07 | 0x50 | 0xEE
───────────────────  ─────────── ─────────── ───────────  ───────────  ──────  ──────  ────── ────── ───────────
                    |   AC      |           |    DC     |             |      |       | BATT. |  ?   | DELIMITER
       HEADER       | VOLTAGE   |  POWER    |  VOLTAGE  | TEMPERATURE |      | FAULT | GAUGE |      |  
                    | (231V)    | (3437W)   |  (25.4V)  |   (27°C)    |      |       |       |      |         
```

FAULT: 0x04 if overload

BATTERY GAUGE: to show bars in the battery icon (depending on DC voltage)

## Using the integration with the external display

The user may edit the code in order to use the integration without removing the external display. To archive this the user should remove the lambda
```yaml
- lambda: |-  # Request
    uint8_t req[] = {0xAE, 0x01, 0x01, 0x03, 0x05, 0xEE};
    id(uart_inverter).write_array(req, sizeof(req));
```
which is the request packet. Then, in the decoding lambda
```yaml
- lambda: |- # Decoding
    ...
```
the user should implement a selection of the inverter packet, ignoring the external display request packet ```{0xAE, 0x01, 0x01, 0x03, 0x05, 0xEE}```.

Contributions are welcome!