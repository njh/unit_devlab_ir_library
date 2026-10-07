/**
 * @file readNec.ino
 * @brief Prints the NEC address and command of every key received by the IRM-V838M3 module.
 *
 * Compatible with ESP32, RP2040/RP2350, STM32 and AVR.
 *
 * @author Jonathan Mejorado
 * @organization UNIT Electronics MX
 */

#include <Arduino.h>
#include <Wire.h>
#include <DevLab_IR.h>

#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_RP2350)
  #define I2C_BUS Wire1
  #if defined(PIN_WIRE1_SDA) && defined(PIN_WIRE1_SCL) && \
      PIN_WIRE1_SDA == 24U && PIN_WIRE1_SCL == 25U
    // Pulsar connector pins.
    constexpr int SDA_PIN = 24;
    constexpr int SCL_PIN = 25;
  #else
    // Generic RP2040/RP2350 wiring.
    constexpr int SDA_PIN = 12;
    constexpr int SCL_PIN = 13;
  #endif
  constexpr uint32_t I2C_FREQ = 400000UL;
#elif defined(ARDUINO_ARCH_ESP32)
  #define I2C_BUS Wire
  constexpr int SDA_PIN = 6;
  constexpr int SCL_PIN = 7;
  constexpr uint32_t I2C_FREQ = 400000UL;
#elif defined(ARDUINO_ARCH_STM32)
  // STM32duino: default I2C pins of the selected board.
  #define I2C_BUS Wire
  constexpr int SDA_PIN = SDA;
  constexpr int SCL_PIN = SCL;
  constexpr uint32_t I2C_FREQ = 400000UL;
#elif defined(ARDUINO_ARCH_AVR)
  // AVR has fixed I2C pins (Uno/Nano: A4/A5, Mega: 20/21, Leonardo: 2/3);
  // SDA/SCL come from the board variant and begin() ignores the pin numbers.
  #define I2C_BUS Wire
  constexpr int SDA_PIN = SDA;
  constexpr int SCL_PIN = SCL;
  constexpr uint32_t I2C_FREQ = 100000UL;  // 400 kHz falla con el level shifter en UNO
#else
#error "Use an ESP32, RP2040, RP2350, STM32, or AVR master"
#endif

DevLab_IR ir(I2C_BUS, DevLab_IR::DEFAULT_ADDRESS, I2C_FREQ);

void setup() {
  Serial.begin(115200);
  delay(500);
  if (!ir.beginRecovered(SDA_PIN, SCL_PIN)) {
    Serial.println("IRM-V838M3 module not found; check wiring, power and firmware");
    return;
  }
  ir.printInfo(Serial);
  Serial.println("Point a NEC remote at the receiver");
}

void loop() {
  if (!ir.isConnected()) return;

  DevLab_IR_Frame frame;
  if (!ir.poll(frame)) {
    Serial.println("ERROR: IR read failed");
    delay(500U);
    return;
  }

  if (frame.newFrame) {
    Serial.print("NEC addr=0x");
    Serial.print(frame.address, HEX);
    Serial.print(" cmd=0x");
    Serial.print(frame.command, HEX);
    Serial.println(frame.valid ? "" : "  (invalid checksum)");
  }
  if (frame.repeat) Serial.println("  repeat");
  if (frame.timingError) Serial.println("  timing error");

  delay(20U);
}
