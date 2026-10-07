# IRM-V838M3 DDP examples

| Example | Purpose |
|---|---|
| `ir/readNec` | Verify Device ID `0x0111` and print the NEC address/command of every key; a held key prints `repeat`. |
| `ir/irEvents` | Low-level view: carrier level, status flags and the 4 raw NEC bytes (`addr ~addr cmd ~cmd`). |
| `i2c/changeAddress` | Scan the bus and change the I2C address of an IRM-V838M3 node. |
| `i2c/i2c_scanner` | List every address that acknowledges on the bus. |

The module decodes NEC only. The factory I2C address is `0x30`. Reading the
status byte clears its event bits, so use one reader per module.

Pins: ESP32 SDA GPIO6 / SCL GPIO7; RP2040/RP2350 `Wire1` GPIO12/13 (GPIO24/25
on Pulsar); STM32 and AVR use the board default pins.
