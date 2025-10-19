# SerialRx - RC Receiver Serial Protocol Parser

Arduino library for parsing serial RC receiver protocols (IBus, SBUS, CRSF) commonly used in UAV flight controllers.

## Overview

SerialRx provides a clean abstraction for receiving and parsing serial RC protocols from radio receivers. The library uses a state machine approach for robust frame synchronization and validation.

**Hardware Validated**: Tested with real FlySky FS-iA6B receiver and dual-USART loopback testing.

## Supported Protocols

| Protocol | Status | Baudrate | Frame Size | Channels | Hardware Validated |
|----------|--------|----------|------------|----------|-------------------|
| **IBus** (FlySky) | ✅ Implemented | 115200 | 32 bytes | 14 | ✅ Yes |
| **SBUS** (FrSky/Futaba) | 📋 Framework Ready | 100000 | 25 bytes | 16 | ⚠️ No |
| **CRSF** (TBS Crossfire) | 📋 Framework Ready | 420000 | Variable | 16 | ⚠️ No |

## IBus Protocol Implementation

### Frame Structure

IBus frames consist of 32 bytes transmitted at 115200 baud:

```
Byte 0-1:   Header (0x20 0x40)
Byte 2-29:  14 channels × 2 bytes each (little-endian uint16_t)
Byte 30-31: Checksum (little-endian uint16_t)
Total: 32 bytes per frame
```

**Example Frame** (14 channels):
```
20 40 DB 05 DC 05 54 05 DC 05 E8 03 D0 07 D2 05 E8 03 DC 05 DC 05 DC 05 DC 05 DC 05 DC 05 DA F3
│  │  └─────┘ └─────┘ └─────┘ └─────┘ └─────┘ └─────┘ └─────┘ └─────┘ └─────┘ └─────┘ └─────┘ └─────┘ └─────┘ └─────┘ └─────┘
│  │    Ch1     Ch2     Ch3     Ch4     Ch5     Ch6     Ch7     Ch8     Ch9    Ch10    Ch11    Ch12    Ch13    Ch14  Checksum
│  └── Header byte 1 (0x40)
└───── Header byte 0 (0x20)
```

**Channel Value Range**:
- Typical: 1000-2000 µs (0x03E8 - 0x07D0)
- Center: 1500 µs (0x05DC)
- Min safe: 1000 µs (stick at minimum)
- Max safe: 2000 µs (stick at maximum)

### Checksum Algorithm

IBus uses a **16-bit sum checksum** with the following algorithm:

```cpp
// Checksum calculation (little-endian)
uint16_t checksum = 0xFFFF;  // Start with 0xFFFF

// Subtract all bytes (header + channel data)
for (int i = 0; i < 30; i++) {
    checksum -= frame[i];
}

// Compare with received checksum (bytes 30-31, little-endian)
uint16_t received_checksum = frame[30] | (frame[31] << 8);
bool valid = (checksum == received_checksum);
```

**Key Properties**:
- **Initial value**: 0xFFFF (not 0x0000)
- **Operation**: Subtraction (not addition)
- **Byte order**: Little-endian (LSB first)
- **Scope**: Covers header + channel data (bytes 0-29)

**Example Calculation**:
```
Frame: 20 40 DB 05 DC 05 54 05 DC 05 E8 03 D0 07 D2 05 E8 03 DC 05 DC 05 DC 05 DC 05 DC 05 DC 05 DA F3

Checksum = 0xFFFF
         - 0x20   // Header byte 0
         - 0x40   // Header byte 1
         - 0xDB   // Ch1 low
         - 0x05   // Ch1 high
         ...      // (continue for all 28 channel bytes)
         - 0x05   // Ch14 high
         = 0xF3DA  // Result

Received checksum (bytes 30-31): 0xDA 0xF3 → 0xF3DA (little-endian)
Match: ✅ Valid frame
```

### State Machine Parser

The IBus parser uses a 5-state machine for robust frame detection and validation:

#### Parser States

```
┌─────────────────────┐
│ WaitingForHeader0   │ ← Initial state
└──────────┬──────────┘
           │ Receive 0x20
           ↓
┌─────────────────────┐
│ ParserHasHeader0    │
└──────────┬──────────┘
           │ Receive 0x40
           ↓
┌─────────────────────┐
│ ParserHasHeader1    │ ← Accumulate 28 channel bytes
└──────────┬──────────┘   Calculate running checksum
           │ 30 bytes received
           ↓
┌─────────────────────┐
│ ParserHasFrame      │ ← Read checksum byte 0
└──────────┬──────────┘
           │ Receive checksum low byte
           ↓
┌─────────────────────┐
│ ParserHasCheckSum0  │ ← Read checksum byte 1
└──────────┬──────────┘   Validate checksum
           │
           ├─ Valid   → ParserNotify() → WaitingForHeader0
           └─ Invalid → ResetParser()  → WaitingForHeader0
```

#### Algorithm Flow

**1. Header Synchronization** (`WaitingForHeader0` → `ParserHasHeader0`):
```cpp
if (byte == 0x20) {
    byte_count_ = 1;
    running_checksum_ = 0xFFFF - byte;  // Initialize checksum
    pstate_ = ParserHasHeader0;
}
```

**2. Header Validation** (`ParserHasHeader0` → `ParserHasHeader1`):
```cpp
if (byte == 0x40) {
    byte_count_ = 2;
    running_checksum_ -= byte;  // Update checksum
    pstate_ = ParserHasHeader1;
} else {
    ResetParser();  // Invalid header, resync
}
```

**3. Channel Data Accumulation** (`ParserHasHeader1`):
```cpp
byte_count_++;
i = (byte_count_ - 3) / 2;  // Channel index (0-13)

if (byte_count_ % 2) {  // Odd byte = low byte
    msg_.channels[i] = byte;
} else {  // Even byte = high byte
    msg_.channels[i] |= (byte << 8);
}

running_checksum_ -= byte;  // Update checksum

if (byte_count_ >= 30) {
    pstate_ = ParserHasFrame;
}
```

**4. Checksum Validation** (`ParserHasFrame` → `ParserHasCheckSum0`):
```cpp
// First checksum byte (low)
frame_checksum_ = byte;
pstate_ = ParserHasCheckSum0;

// Second checksum byte (high)
frame_checksum_ = (byte << 8) | frame_checksum_;  // Little-endian

if (frame_checksum_ == running_checksum_) {
    ParserNotify();  // ✅ Valid frame - notify listener
} else {
    // ❌ Checksum mismatch - discard frame
}

ResetParser();  // Return to WaitingForHeader0
```

### Error Handling and Recovery

The parser implements several robustness features:

**1. Automatic Resynchronization**:
- Invalid header byte → immediate reset to `WaitingForHeader0`
- Checksum mismatch → frame discarded, ready for next frame
- No frame timeout needed (state machine handles partial frames)

**2. Byte-by-Byte Processing**:
```cpp
void SerialRx::update() {
    while (serial_->available()) {
        uint8_t byte = serial_->read();
        if (parser_->ParseByte(byte)) {
            // Complete valid frame parsed
            last_message_time_ = millis();
        }
    }
}
```

**3. Timeout Detection** (Application Layer):
```cpp
bool SerialRx::timeout(uint32_t timeout_ms) const {
    return (millis() - last_message_time_) > timeout_ms;
}
```

### Frame Rate and Timing

**Typical IBus Timing**:
- Frame interval: 7-14 ms (varies by receiver model)
- Frame rate: ~70-140 Hz (FlySky FS-iA6B measured at ~100 Hz)
- Transmission time: 32 bytes ÷ 11520 bytes/sec = 2.78 ms per frame

**Byte Timing** (115200 baud):
- 1 start bit + 8 data bits + 1 stop bit = 10 bits per byte
- Byte period: 10 ÷ 115200 = 86.8 µs per byte
- Frame transmission: 32 bytes × 86.8 µs = 2.78 ms

### Hardware Validation Results

**Loopback Testing** (Dual-USART):
- Configuration: PA11 (USART6 TX) → PA10 (USART1 RX)
- Frame rate: 100 Hz (10ms intervals)
- Test duration: 5 seconds
- Result: **501/501 frames** (0% loss)

**Real Receiver Testing** (FlySky FS-iA6B):
- Configuration: Receiver IBus → PA10 (USART1 RX)
- Test duration: 30 seconds
- Result: **~300 frames received** (10 Hz display rate)
- Channel reading: ✅ All 10 channels (1000-2000µs range)
- Timeout detection: ✅ Intermittent RF dropouts handled correctly
- Status: **Production-ready**

## Usage

### Basic Setup

```cpp
#include <SerialRx.h>

// Create HardwareSerial instance for USART1 (RX=PA10, TX=PA9)
HardwareSerial SerialRC(PA10, PA9);

// Create SerialRx instance
SerialRx rc;

void setup() {
  // Configure RC receiver on USART1
  SerialRx::Config config;
  config.serial = &SerialRC;
  config.protocol = SerialRx::IBUS;  // IBus protocol
  config.baudrate = 115200;          // IBus standard baudrate
  config.timeout_ms = 1000;          // 1 second failsafe timeout

  if (!rc.begin(config)) {
    // Initialization failed
    while (1);
  }
}

void loop() {
  // Update RC receiver (polls serial buffer)
  rc.update();

  // Check for new messages
  if (rc.available()) {
    RCMessage msg;
    if (rc.getMessage(&msg)) {
      // Process RC commands
      uint16_t throttle = msg.channels[2];  // Channel 3 (0-indexed)
      uint16_t aileron = msg.channels[0];   // Channel 1
      uint16_t elevator = msg.channels[1];  // Channel 2
      uint16_t rudder = msg.channels[3];    // Channel 4

      // Use channel values (typically 1000-2000)
    }
  }

  // Check for timeout (failsafe)
  if (rc.timeout(1000)) {
    // Signal lost for >1000ms - activate failsafe
    // Stop motors, level aircraft, etc.
  }
}
```

### Channel Mapping

Standard AETR (Aileron, Elevator, Throttle, Rudder) mapping:

| Channel | Function | Typical Range |
|---------|----------|---------------|
| Ch1 (0) | Aileron (Roll) | 1000-2000 µs |
| Ch2 (1) | Elevator (Pitch) | 1000-2000 µs |
| Ch3 (2) | Throttle | 1000-2000 µs |
| Ch4 (3) | Rudder (Yaw) | 1000-2000 µs |
| Ch5 (4) | Aux1 (Switch) | 1000/2000 µs |
| Ch6 (5) | Aux2 (Switch) | 1000/2000 µs |

### Failsafe Handling

```cpp
void loop() {
  rc.update();

  if (rc.timeout(200)) {
    // RC signal lost for >200ms

    // Implement failsafe behavior:
    // 1. Stop motors
    // 2. Level aircraft
    // 3. Descend slowly
    // 4. Log event

    return;  // Skip normal control loop
  }

  // Normal RC processing...
}
```

## Hardware Setup

### NUCLEO-F411RE Wiring

```
RC Receiver → NUCLEO-F411RE
────────────────────────────
IBus pin    → PA10 (CN10-33, also labeled D2)
GND         → GND
VCC         → 5V (if receiver needs external power)
```

**Important Notes**:
- **RX only**: Only PA10 (USART1 RX) is needed for IBus reception
- **Power**: Most receivers can be powered from 5V pin or bind to ESC/flight controller power
- **Signal level**: IBus is 3.3V compatible (STM32 GPIO tolerant)

## Examples

### IBus_Basic
Real receiver channel reading and display with dual mode:
- **RTT mode** (`--use-rtt`): 30-second timed test with `*STOP*` wildcard
- **Serial mode** (Arduino IDE): Continuous display for manual testing
- **Hardware validated** with FlySky FS-iA6B receiver

**Usage**:
```bash
# CI/HIL testing (30-second capture)
./scripts/aflash.sh libraries/SerialRx/examples/IBus_Basic --use-rtt --build-id

# Arduino IDE (continuous monitoring)
# Upload via Arduino IDE, open Serial Monitor at 115200 baud
```

### IBus_Loopback_Test
Hardware validation using dual-USART loopback:
- Transmitter: USART6 TX (PA11) generates IBus frames at 100 Hz
- Receiver: USART1 RX (PA10) parses frames via SerialRx
- **Result**: 501/501 frames (0% loss) in 5-second test
- **Deterministic HIL testing** with `*STOP*` wildcard

**Setup**:
```
Jumper wire: PA11 (CN10-14) → PA10 (CN10-33)
```

## Testing

### Unit Tests
The library integrates with the AUnit testing framework:
```bash
# Run SerialRx unit tests (future)
./scripts/aflash.sh tests/SerialRx_Unit_Tests --use-rtt --build-id
```

### Hardware Testing
```bash
# Loopback test (validates protocol implementation)
./scripts/aflash.sh libraries/SerialRx/examples/IBus_Loopback_Test --use-rtt

# Real receiver test (validates hardware integration)
./scripts/aflash.sh libraries/SerialRx/examples/IBus_Basic --use-rtt
```

## Architecture

### Class Hierarchy

```
SerialRx (Transport Layer)
    │
    ├── RingBuffer (Serial buffering)
    │
    └── ProtocolParser (Interface)
            │
            ├── IBusParser (Implemented)
            ├── SBusParser (Future)
            └── CRSFParser (Future)
```

### Design Patterns

**1. Strategy Pattern**:
- `ProtocolParser` interface allows runtime protocol selection
- Each parser implements `ParseByte()` state machine

**2. Observer Pattern**:
- `ParserNotify()` signals complete frame to transport layer
- `Listener()` checks for available messages

**3. State Machine**:
- Each parser maintains internal state for robust frame detection
- Automatic recovery from partial/invalid frames

## Performance

### Memory Usage
- **Flash**: ~2 KB (parser + transport layer)
- **RAM**: ~100 bytes (message buffer + parser state)
- **Stack**: Minimal (byte-by-byte processing)

### CPU Usage
- **Parser**: ~10 µs per byte @ 100 MHz (STM32F411)
- **Update loop**: Processes serial buffer in non-blocking manner
- **Interrupt-free**: Compatible with other time-critical tasks

## Future Protocols

### SBUS (Planned)
- Baudrate: 100000 baud (inverted signal - requires hardware inverter)
- Frame size: 25 bytes
- Channels: 16 (11-bit resolution)
- Checksum: None (relies on header/footer validation)

### CRSF (Planned)
- Baudrate: 420000 baud
- Frame size: Variable (type-dependent)
- Channels: 16 (10-bit resolution)
- Checksum: CRC8-DVB-S2

## References

- **IBus Protocol**: FlySky iBus specification
- **Hardware Validation**: `libraries/SerialRx/examples/IBus_Loopback_Test/README.md`
- **Technical Documentation**: `doc/SERIAL.md`

## License

Part of the STM32 Arduino Core UAV project.

---

**Hardware Validated**: ✅ Loopback testing + Real FlySky FS-iA6B receiver
**Production Status**: Ready for flight controller integration
