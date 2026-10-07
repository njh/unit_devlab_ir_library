# DevLab_IR

Arduino library for the IRM-V838M3 infrared receiver module (38 kHz) using the
DevLab Device Protocol (DDP) over I2C.

The module is a PY32F003 that decodes NEC frames in firmware and exposes them
through I2C. This library wraps the shared
[`DevLabDDP`](https://github.com/UNIT-Electronics-MX/unit_devlab_ddp_library)
master and the
[`DevLab_Interface`](https://github.com/UNIT-Electronics-MX/unit_devlab_interface_library)
`DevLab_I2C_Orchestrator` bus class into one `DevLab_IR` object.
Firmware: `unit_firmware_i2c_irm_v838m3_py32`.

Compatible with ESP32, RP2040/RP2350, STM32 and AVR.

# Features

- Device identification (Device ID `0x0111`) before any command
- `poll()` returns address, command, repeat and error flags in one call
- Low-level `readCarrier()`, `readStatus()` and `readRawFrame()`
- I2C address scan and reassignment (`0x08` to `0x77`), default address `0x30`
- Bus recovery (`beginRecovered`) for a slave left mid-transaction

# Quick Start

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <DevLab_IR.h>

DevLab_IR ir(Wire, DevLab_IR::DEFAULT_ADDRESS, 400000);

void setup() {
  Serial.begin(115200);
  if (!ir.beginRecovered(SDA, SCL)) {
    Serial.println("IRM-V838M3 not found");
    while (1);
  }
}

void loop() {
  DevLab_IR_Frame f;
  if (ir.poll(f) && f.newFrame && f.valid) {
    Serial.printf("addr=0x%02X cmd=0x%02X\n", f.address, f.command);
  }
  delay(20);
}
```

# Command map (block 0x80)

| Command | Method | Response |
|---:|---|---|
| `0x80` | `readCarrier()` | 1 byte, bit0 = carrier present (output low) |
| `0x81` | `readStatus()` | 1 byte of flags; clears event bits |
| `0x82` | `readRawFrame()` | 4 bytes `addr, ~addr, cmd, ~cmd`; clears `NEW_FRAME` |

Status bits: `0x01` new frame, `0x02` repeat, `0x04` checksum OK, `0x08`
carrier now, `0x10` timing error.

# Notes

- NEC only; RC5/Sony are not decoded by the firmware.
- `DevLabDDP::deviceName()` reports `unknown` for ID `0x0111` until the DDP
  library adds it; this does not affect identification.
- The firmware is not yet validated on hardware (see its README).

# Folder Structure

```text
DevLab_IR/
├── examples/
│   ├── i2c/{changeAddress,i2c_scanner}
│   └── ir/{readNec,irEvents}
├── src/DevLab_IR.{h,cpp}
├── library.properties
├── keywords.txt
└── LICENSE
```

# Author

UNIT Electronics MX - Jonathan Mejorado

# License

MIT License
