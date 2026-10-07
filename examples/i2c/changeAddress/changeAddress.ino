/**
 * @file changeAddress.ino
 * @brief Serial tool to scan the bus and change the I2C address of an
 *        IRM-V838M3 DDP node (Device ID 0x0111, factory address 0x30).
 *
 * Open Serial at 115200 baud and enter, for example: scan / change 30 31
 *
 * @author Jonathan Mejorado
 * @organization UNIT Electronics MX
 */

#include <Arduino.h>
#include <Wire.h>
#include <DevLabDDP.h>
#include <DevLab_IR.h>
#include <DevLab_I2C_Orchestrator.h>

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

constexpr uint16_t EXPECTED_DEVICE_ID = DevLab_IR::DEVICE_ID;
DevLab_I2C_Orchestrator bus(I2C_BUS, I2C_FREQ);
DevLabDDP::Master master(bus, EXPECTED_DEVICE_ID);
String inputLine;

bool parseAddress(const String &text, uint8_t &address) {
  char *end = nullptr;
  long value = strtol(text.c_str(), &end, 16);
  if (end == text.c_str() || *end != '\0' || value < 0x08L || value > 0x77L) {
    return false;
  }
  address = (uint8_t)value;
  return true;
}

void printHexAddress(uint8_t address) {
  Serial.print("0x");
  if (address < 0x10U) Serial.print('0');
  Serial.print(address, HEX);
}

void scanBus() {
  bool found = false;
  Serial.println("Address  Sensor");
  for (uint8_t address = 0x08U; address <= 0x77U; ++address) {
    if (!master.ping(address)) continue;
    found = true;
    printHexAddress(address);
    Serial.print("     ");
    DevLabDDP::DeviceInfo info;
    Serial.println(master.identify(address, info)
                       ? DevLabDDP::deviceName(info.deviceId)
                       : "non-DDP");
  }
  if (!found) Serial.println("--       none");
}

void printHelp() {
  Serial.println("Commands:");
  Serial.println("  scan");
  Serial.println("  change <current_hex> <new_hex>");
  Serial.println("Example:");
  Serial.println("  change 28 30");
}

void processCommand(String line) {
  line.trim();
  line.toLowerCase();

  if (line == "scan") {
    scanBus();
    return;
  }

  int firstSpace = line.indexOf(' ');
  int secondSpace = firstSpace < 0 ? -1 : line.indexOf(' ', firstSpace + 1);
  if (firstSpace < 0 || secondSpace < 0 ||
      line.substring(0, firstSpace) != "change") {
    Serial.println("ERROR invalid command");
    printHelp();
    return;
  }

  String oldText = line.substring(firstSpace + 1, secondSpace);
  String newText = line.substring(secondSpace + 1);
  oldText.trim();
  newText.trim();

  uint8_t oldAddress, newAddress;
  if (!parseAddress(oldText, oldAddress) ||
      !parseAddress(newText, newAddress) ||
      oldAddress == newAddress) {
    Serial.println("ERROR addresses must be different hexadecimal values from 08 to 77");
    return;
  }

  DevLabDDP::DeviceInfo info;
  if (!master.matchesExpectedDevice(oldAddress, &info)) {
    Serial.println("ERROR current address does not contain the expected DDP device");
    return;
  }
  if (master.ping(newAddress)) {
    Serial.println("ERROR new address is already in use");
    return;
  }

  Serial.print("Changing ");
  printHexAddress(oldAddress);
  Serial.print(" -> ");
  printHexAddress(newAddress);
  Serial.println("...");

  if (!master.setI2cAddress(oldAddress, newAddress)) {
    Serial.println("ERROR address change failed");
    return;
  }

  Serial.print("OK device is now available at ");
  printHexAddress(newAddress);
  Serial.println();
  scanBus();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  bus.beginRecovered(SDA_PIN, SCL_PIN);

  Serial.print("Expected DDP device ID: 0x");
  Serial.println(EXPECTED_DEVICE_ID, HEX);
  printHelp();
  scanBus();
}

void loop() {
  while (Serial.available()) {
    char character = (char)Serial.read();
    if (character == '\r' || character == '\n') {
      if (inputLine.length() > 0U) {
        processCommand(inputLine);
        inputLine = "";
      }
    } else if (inputLine.length() < 64U) {
      inputLine += character;
    } else {
      inputLine = "";
      Serial.println("ERROR command is too long");
    }
  }
}
