/**
 * @file i2c_scanner.ino
 * @brief Scans the I2C bus with DevLab_I2C_Orchestrator and prints the
 *        7-bit address of every device that responds over Serial.
 *
 * A detected address does not by itself identify the device or its
 * measurement protocol; it only confirms that something acknowledged on
 * that address. Per-architecture pin/bus setup lives inside the
 * orchestrator's begin(); this sketch only supplies the wiring.
 *
 * @author Jonathan Mejorado
 * @organization UNIT Electronics MX
 */

#include <Arduino.h>
#include <Wire.h>
#include <DevLab_I2C_Orchestrator.h>

#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_RP2350)
  #define I2C_BUS Wire1
  constexpr int SDA_PIN = 12;
  constexpr int SCL_PIN = 13;
#elif defined(ARDUINO_ARCH_ESP32)
  #define I2C_BUS Wire
  constexpr int SDA_PIN = 6;
  constexpr int SCL_PIN = 7;
#elif defined(ARDUINO_ARCH_STM32)
  // STM32duino: default I2C pins of the selected board.
  #define I2C_BUS Wire
  constexpr int SDA_PIN = SDA;
  constexpr int SCL_PIN = SCL;
#elif defined(ARDUINO_ARCH_AVR)
  // AVR has fixed I2C pins (Uno/Nano: A4/A5, Mega: 20/21, Leonardo: 2/3);
  // SDA/SCL come from the board variant, and the orchestrator's begin()
  // ignores the pin numbers on this architecture.
  #define I2C_BUS Wire
  constexpr int SDA_PIN = SDA;
  constexpr int SCL_PIN = SCL;
#else
#error "Use an ESP32, RP2040, RP2350, STM32, or AVR master"
#endif

constexpr uint32_t I2C_FREQ = 100000UL;
constexpr uint32_t SCAN_INTERVAL_MS = 2000U;

DevLab_I2C_Orchestrator bus(I2C_BUS, I2C_FREQ);

void setup() {
  Serial.begin(115200);

  bus.begin(SDA_PIN, SCL_PIN);

  Serial.println("IRM-V838M3 I2C scanner");
  Serial.println("A detected address does not define the measurement protocol.");
}

void loop() {
  static uint32_t lastScan = 0U;
  if (millis() - lastScan < SCAN_INTERVAL_MS) {
    return;
  }
  lastScan = millis();

  Serial.println("Scanning...");
  uint8_t devices = 0U;
  for (uint8_t address = 1; address < 127; ++address) {
    if (bus.ping(address)) {
      Serial.print("I2C response at 0x");
      if (address < 0x10) {
        Serial.print('0');
      }
      Serial.println(address, HEX);
      ++devices;
    }
  }

  if (devices == 0) {
    Serial.println("No I2C devices found.");
  } else {
    Serial.print("Devices found: ");
    Serial.println(devices);
  }
}
