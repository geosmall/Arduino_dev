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
    , last_message_time_(0)
    , idle_threshold_us_(0)
    , last_byte_time_us_(0)
    , expect_frame_start_(false) {
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
    protocol_ = config.rx_protocol;
    timeout_ms_ = config.timeout_ms;
    idle_threshold_us_ = config.idle_threshold_us;
    last_message_time_ = millis();
    last_byte_time_us_ = micros();
    expect_frame_start_ = false;

    // Create parser based on protocol
    switch (protocol_) {
    case IBUS:
        parser_ = new IBusParser();
        break;
    case SBUS:
        parser_ = new SBusParser();
        break;
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

    uint32_t now = micros();

    // Software idle line detection (optional feature)
    if (idle_threshold_us_ > 0) {
        uint32_t idle_time = now - last_byte_time_us_;

        if (idle_time > idle_threshold_us_) {
            if (!expect_frame_start_) {
                // Idle period detected → prepare for guaranteed frame start
                parser_->ResetParser();
                expect_frame_start_ = true;
            }
        }
    }

    // Poll serial buffer and feed bytes to parser
    while (serial_->available()) {
        uint8_t byte = serial_->read();
        last_byte_time_us_ = micros();  // Update timestamp

        // Validate frame start after idle detection
        if (expect_frame_start_) {
            expect_frame_start_ = false;

            // After idle, first byte MUST be frame start (0x20 for IBus)
            if (byte != 0x20) {
                // Not a valid frame start after idle → discard byte
                continue;
            }
        }

        if (parser_->ParseByte(byte)) {
            // Complete valid message parsed (checksum validated)
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
