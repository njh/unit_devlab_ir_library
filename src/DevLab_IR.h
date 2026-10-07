/**
 * @file DevLab_IR.h
 * @brief DevLab IRM-V838M3 infrared receiver driver (NEC) over DDP/I2C.
 *
 * @author Jonathan Mejorado
 * @organization UNIT Electronics MX
 */

#ifndef DEVLAB_IR_H
#define DEVLAB_IR_H

#pragma once

#include "DevLabDDP.h"
#include "DevLabDDPConsole.h"
#include "DevLab_I2C_Orchestrator.h"

/* Result of DevLab_IR::poll(). */
struct DevLab_IR_Frame
{
    bool newFrame = false;   /* a complete NEC frame was received */
    bool repeat = false;     /* repeat code (key held down) */
    bool valid = false;      /* ~address / ~command check passed (newFrame only) */
    bool timingError = false;/* a frame with out-of-tolerance timing was seen */
    uint8_t address = 0U;    /* NEC address byte (newFrame only) */
    uint8_t command = 0U;    /* NEC command byte (newFrame only) */
};

class DevLab_IR
{
public:
    /* DDP Device ID of the IRM-V838M3 module (DDP_DEVICE_ID in firmware). */
    static constexpr uint16_t DEVICE_ID = 0x0111U;

    /* Factory I2C address (STARTUP_I2C_ADDRESS). */
    static constexpr uint8_t DEFAULT_ADDRESS = 0x30U;

    /* Bits returned by readStatus() (command 0x81). */
    static constexpr uint8_t STATUS_NEW_FRAME = 0x01U;
    static constexpr uint8_t STATUS_REPEAT = 0x02U;
    static constexpr uint8_t STATUS_CHECK_OK = 0x04U;
    static constexpr uint8_t STATUS_CARRIER = 0x08U;
    static constexpr uint8_t STATUS_ERROR = 0x10U;

    explicit DevLab_IR(TwoWire &wire = Wire, uint8_t address = DEFAULT_ADDRESS, uint32_t clock = 400000UL)
    : _bus(wire, clock), _ddp(_bus, DEVICE_ID), _address(address), _clock(clock) {}

    bool begin();
    bool begin(uint8_t sdaPin, uint8_t sclPin, uint32_t clock = 400000UL);
    bool beginRecovered(uint8_t sdaPin, uint8_t sclPin, uint32_t timeoutUs = 20000, bool restart = false);

    /* Instantaneous module output (command 0x80): `carrier` is true while the
     * receiver detects a 38 kHz burst (output low). */
    bool readCarrier(bool &carrier);

    /* Reads the status byte (command 0x81). Reading CLEARS the event bits
     * (NEW_FRAME, REPEAT, ERROR) in the module. */
    bool readStatus(uint8_t &status);

    /* Reads the raw NEC frame (command 0x82): address, ~address, command,
     * ~command. Clears NEW_FRAME in the module. */
    bool readRawFrame(uint8_t frame[4]);

    /* One-call polling helper: reads the status and, if a new frame arrived,
     * the frame itself. Returns false on an I2C error. */
    bool poll(DevLab_IR_Frame &out);

    bool busReady() const { return _busReady; }
    bool isConnected() const { return _verified; }
    uint8_t address() const { return _address; }
    const DevLabDDP::DeviceInfo &deviceInfo() const { return _info; }
    void printInfo(Print &out = Serial) const;

    DevLabDDP::Master &protocol() { return _ddp; }
    DevLab_I2C_Orchestrator &bus() { return _bus; }

private:
    static constexpr uint8_t CMD_READ_LEVEL = 0x80U;
    static constexpr uint8_t CMD_READ_STATUS = 0x81U;
    static constexpr uint8_t CMD_READ_FRAME = 0x82U;
    static constexpr uint16_t _kResponseDelayMs = 2U;

    DevLab_I2C_Orchestrator _bus;
    DevLabDDP::Master _ddp;
    uint8_t _address;
    uint32_t _clock;
    bool _busReady = false;
    bool _verified = false;
    DevLabDDP::DeviceInfo _info;
};

#endif
