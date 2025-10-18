/*
 * SerialRx - Arduino library for serial RC protocols
 *
 * Transport layer implementation
 */

#include "SerialRx.h"

SerialRx::SerialRx(Protocol protocol)
    : serial_(nullptr)
    , parser_(nullptr)
    , protocol_(protocol)
    , timeout_ms_(1000)
    , last_message_time_(0) {
}

SerialRx::~SerialRx() {
    if (parser_ != nullptr) {
        delete parser_;
        parser_ = nullptr;
    }
}

bool SerialRx::begin(const Config& config) {
    if (config.serial == nullptr) {
        return false;
    }

    serial_ = config.serial;
    protocol_ = config.protocol;
    timeout_ms_ = config.timeout_ms;
    last_message_time_ = millis();

    // Create parser based on protocol
    switch (protocol_) {
    case IBUS:
        parser_ = new IBusParser();
        break;
    case SBUS:
        // Future: parser_ = new SBusParser();
        return false;
    case NONE:
    default:
        return false;
    }

    if (parser_ == nullptr) {
        return false;
    }

    // Initialize serial port
    serial_->begin(config.baudrate);

    return true;
}

void SerialRx::update() {
    if (serial_ == nullptr || parser_ == nullptr) {
        return;
    }

    // Poll serial buffer and feed bytes to parser
    while (serial_->available()) {
        uint8_t byte = serial_->read();
        if (parser_->ParseByte(byte)) {
            // Complete message parsed
            last_message_time_ = millis();
        }
    }
}

bool SerialRx::available() const {
    if (parser_ == nullptr) {
        return false;
    }
    return parser_->Listener();
}

bool SerialRx::getMessage(RCMessage* msg) {
    if (parser_ == nullptr || msg == nullptr) {
        return false;
    }
    return parser_->GetMessageFromFIFO(msg);
}

bool SerialRx::timeout(uint32_t threshold_ms) const {
    return (millis() - last_message_time_) > threshold_ms;
}

uint32_t SerialRx::timeSinceLastMessage() const {
    return millis() - last_message_time_;
}

bool SerialRx::sendTelemetry(uint8_t* data, size_t len) {
    // Future implementation for bi-directional telemetry
    if (serial_ == nullptr || data == nullptr || len == 0) {
        return false;
    }

    // TODO: Implement protocol-specific telemetry framing
    serial_->write(data, len);
    return true;
}
