/**
 * @file DevLab_IR.cpp
 * @brief DevLab IRM-V838M3 infrared receiver driver (NEC) over DDP/I2C.
 *
 * @author Jonathan Mejorado
 * @organization UNIT Electronics MX
 */

#include "DevLab_IR.h"

bool DevLab_IR::begin() {
    _verified = false;
    _bus.setClock(_clock);
    _busReady = _bus.begin();
    if (!_busReady) return false;
    return (_verified = _ddp.matchesExpectedDevice(_address, &_info));
}

bool DevLab_IR::begin(uint8_t sdaPin, uint8_t sclPin, uint32_t clock) {
    _verified = false;
    _clock = clock;
    _bus.setClock(_clock);
    _busReady = _bus.begin(sdaPin, sclPin);
    if (!_busReady) return false;
    return (_verified = _ddp.matchesExpectedDevice(_address, &_info));
}

bool DevLab_IR::beginRecovered(uint8_t sdaPin, uint8_t sclPin, uint32_t timeoutUs, bool restart) {
    _verified = false;
    _bus.setClock(_clock);
    _busReady = _bus.beginRecovered(sdaPin, sclPin, timeoutUs, restart);
    if (!_busReady) return false;
    return (_verified = _ddp.matchesExpectedDevice(_address, &_info));
}

void DevLab_IR::printInfo(Print &out) const {
    DevLabDDP::printDeviceInfo(out, _address, _info, _ddp.expectedDeviceId());
}

bool DevLab_IR::readCarrier(bool &carrier) {
    if (!_verified) return false;
    uint8_t raw = 0U;
    if (!_ddp.readCommand(_address, CMD_READ_LEVEL, &raw, 1U, _kResponseDelayMs)) return false;
    carrier = (raw & 0x01U) != 0U;
    return true;
}

bool DevLab_IR::readStatus(uint8_t &status) {
    if (!_verified) return false;
    return _ddp.readCommand(_address, CMD_READ_STATUS, &status, 1U, _kResponseDelayMs);
}

bool DevLab_IR::readRawFrame(uint8_t frame[4]) {
    if (!_verified) return false;
    return _ddp.readCommand(_address, CMD_READ_FRAME, frame, 4U, _kResponseDelayMs);
}

bool DevLab_IR::poll(DevLab_IR_Frame &out) {
    out = DevLab_IR_Frame();

    uint8_t status = 0U;
    if (!readStatus(status)) return false;

    out.repeat = (status & STATUS_REPEAT) != 0U;
    out.timingError = (status & STATUS_ERROR) != 0U;

    if (status & STATUS_NEW_FRAME) {
        uint8_t raw[4];
        if (!readRawFrame(raw)) return false;
        out.newFrame = true;
        out.address = raw[0];
        out.command = raw[2];
        out.valid = (status & STATUS_CHECK_OK) != 0U &&
                    (uint8_t)(raw[0] ^ raw[1]) == 0xFFU &&
                    (uint8_t)(raw[2] ^ raw[3]) == 0xFFU;
    }
    return true;
}
