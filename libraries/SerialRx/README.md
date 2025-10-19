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
│  │  └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘
│  │     Ch1     Ch2     Ch3     Ch4     Ch5     Ch6     Ch7     Ch8     Ch9    Ch10    Ch11    Ch12    Ch13    Ch14   Chksum
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

#### Frame Start Detection

The parser finds frame boundaries by **scanning the byte stream** for the 2-byte header pattern `0x20 0x40`:

**How it works**:
1. Parser starts in `WaitingForHeader0` state
2. **Every incoming byte** is tested against 0x20
3. When 0x20 found → advance to `ParserHasHeader0`
4. Next byte tested against 0x40:
   - If 0x40 → **Frame start confirmed!** Begin parsing
   - If not 0x40 → **False positive**, return to `WaitingForHeader0`

**Robustness**:
- ✅ No timing requirements (works at any baud rate accuracy)
- ✅ Automatic mid-stream synchronization (can start listening anytime)
- ✅ Self-recovering (bad frames don't break sync)
- ✅ No preamble needed (header pattern is unique enough)

**Example byte stream** (showing synchronization):
```
Incoming bytes: ... 3A F2 20 40 DB 05 DC 05 ... (valid frame starts)
                         ↑  ↑
                         │  └─ Header byte 1 (0x40) → Frame confirmed!
                         └──── Header byte 0 (0x20) → Potential frame start

Incoming bytes: ... 3A 20 F2 40 DB 05 ... (false start)
                         ↑  ↑
                         │  └─ Not 0x40 → Reset, keep scanning
                         └──── Header byte 0 (0x20) → Check next byte
```

**Can header pattern appear in channel data?**

Yes! The pattern 0x20 0x40 CAN occur in valid frames:
- Channel value 0x0520 = 1312 µs (valid) → high byte = 0x20
- Channel value 0x??40 = ??40 µs (valid) → low byte = 0x40
- If consecutive: `... 20 05 40 05 ...` contains the header pattern!

**How does the parser handle false positives?**

1. **Checksum validation prevents false frames**:
   - Parser finds 0x20 0x40 in channel data (mid-frame)
   - Accumulates next 28 bytes as "channel data"
   - Reads next 2 bytes as "checksum"
   - Checksum calculation FAILS (these aren't the right 30 bytes!)
   - Frame rejected, parser resets to `WaitingForHeader0`

2. **Frame loss risk**:
   - False positive at byte position N
   - Parser locks onto it and reads 32 bytes (N to N+31)
   - Real frame start (actual 0x20 0x40) might be at position N+10
   - If real header falls within N to N+31, that frame is lost!

3. **Why this rarely happens in practice**:
   - **Inter-frame gaps**: IBus receivers send frames at ~100 Hz (10ms intervals)
   - Frame transmission: 2.78 ms (32 bytes @ 115200 baud)
   - Gap between frames: ~7 ms (no bytes transmitted)
   - Real header (0x20 0x40) appears after gap
   - False header (0x20 0x40 in data) appears mid-transmission
   - Parser typically finds real header first after each gap

4. **Statistical probability**:
   - Probability of 0x20 at any channel high byte position: ~1/256
   - Probability of 0x40 at next channel low byte: ~1/256
   - Combined: ~1/65536 per adjacent channel pair
   - 14 channels = 13 adjacent pairs per frame
   - Expected false positive: ~1 in 5000 frames
   - With checksum protection: false frame never accepted, at most causes 1 frame skip

**Real-world validation**:
- Loopback test: 501/501 frames (0% loss)
- Real receiver test: 30 seconds, no unexplained frame loss
- The combination of inter-frame gaps + checksum validation provides robust synchronization

**Future Enhancement: Software Idle Line Detection**

The current implementation could be enhanced with **timestamp-based idle detection** to eliminate false positives entirely:

**Concept**:
- Track `micros()` timestamp of last received byte
- If time since last byte > idle threshold (e.g., 200-300 µs)
- Flag "idle detected" → next byte MUST be frame start (0x20)
- Reject byte if not 0x20 after idle (guaranteed false positive)

**Benefits**:
- ✅ **Eliminates false positives** - Idle period guarantees frame boundary
- ✅ **Arduino-compatible** - Uses standard `micros()`, no HAL needed
- ✅ **Portable** - Works with any Serial implementation (AVR, ESP32, STM32)
- ✅ **Configurable** - Optional feature, can be disabled for compatibility

**Idle threshold calculation** (IBus @ 115200 baud):
```
UART frame time = 10 bits ÷ 115200 = 86.8 µs
Safe threshold = 2-3 frame times = 200-300 µs

Inter-frame gap: ~7 ms (much larger than threshold)
Frame transmission: 32 bytes × 86.8 µs = 2.78 ms (no gaps within frame)

Threshold detection:
  - Idle after frame: 7 ms > 300 µs ✓ Detected
  - Bytes within frame: <1 µs gap < 300 µs ✓ Not idle
```

**Implementation approach**:
```cpp
class SerialRx {
private:
    uint32_t last_byte_time_us_;
    uint32_t idle_threshold_us_;
    bool expect_frame_start_;

public:
    void begin(const Config& config) {
        idle_threshold_us_ = config.idle_threshold_us;  // 0 = disabled, 300 = enabled
        last_byte_time_us_ = micros();
        expect_frame_start_ = false;
    }

    void update() {
        uint32_t now = micros();

        // Check for idle period (optional feature)
        if (idle_threshold_us_ > 0) {
            if (now - last_byte_time_us_ > idle_threshold_us_) {
                if (!expect_frame_start_) {
                    // Idle detected → prepare for guaranteed frame start
                    parser_->ResetParser();
                    expect_frame_start_ = true;
                }
            }
        }

        // Process incoming bytes
        while (serial_->available()) {
            uint8_t byte = serial_->read();
            last_byte_time_us_ = now;

            // Validate frame start after idle
            if (expect_frame_start_) {
                expect_frame_start_ = false;
                if (byte != 0x20) {
                    continue;  // Discard: not valid frame start after idle!
                }
            }

            if (parser_->ParseByte(byte)) {
                last_message_time_ = millis();
            }
        }
    }
};
```

**Configuration**:
```cpp
SerialRx::Config config;
config.serial = &SerialRC;
config.protocol = SerialRx::IBUS;
config.baudrate = 115200;
config.idle_threshold_us = 300;  // Enable idle detection (0 = disabled)
```

**Trade-offs**:

| Approach | Pros | Cons |
|----------|------|------|
| **Current (checksum only)** | Simple, minimal code, proven reliable | Theoretical false positive risk (~1/5000 frames) |
| **Software idle detection** | Eliminates false positives, portable | Requires `micros()` accuracy, slightly more complex |
| **Hardware idle interrupt** | Most efficient, DMA-ready | STM32-specific, breaks Arduino portability |

**Recommendation**:
1. **Current approach** is production-ready (validated: 501/501 frames, 0% loss)
2. **Software idle detection** is the best enhancement (portable + robust)
3. **Hardware idle interrupt** is only needed for DMA-based implementations

**Implementation priority**: Consider for next release after SBUS protocol support

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

### SBUS Extension Strategy

SBUS (FrSky/Futaba) implementation will follow the same state machine architecture as IBus.

**Protocol Specifications**:
- **Baudrate**: 100000 baud (inverted signal)
- **Frame size**: 25 bytes fixed
- **Channels**: 16 channels @ 11-bit resolution (0-2047 range)
- **Frame rate**: ~7-14 ms (similar to IBus)
- **Signal**: Inverted UART (requires hardware inverter or GPIO config)

**Frame Structure**:
```
Byte 0:     Header (0x0F)
Byte 1-22:  16 channels × 11 bits = 176 bits = 22 bytes (packed)
Byte 23:    Flags (digital channels, frame lost, failsafe)
Byte 24:    Footer (0x00)
Total: 25 bytes per frame
```

**Channel Packing** (11-bit little-endian):
```
Byte 1:  [Ch1   LSB 8 bits]
Byte 2:  [Ch2   LSB 5 bits] [Ch1   MSB 3 bits]
Byte 3:  [Ch2   MSB 6 bits] [Ch3   LSB 2 bits]
...
Byte 22: [Ch16  MSB 3 bits] [reserved]
```

**State Machine** (6 states):
```
WaitingForHeader → Header (0x0F)
                → AccumulateChannels (22 bytes)
                → ReadFlags (1 byte)
                → ValidateFooter (0x00)
                → ParserNotify()
```

**Implementation Plan**:

1. **Create SBusParser class** (`src/parsers/SBusParser.h/cpp`):
```cpp
class SBusParser : public ProtocolParser {
public:
    bool ParseByte(uint8_t byte) override;
    void ResetParser() override;

private:
    enum ParserState {
        WaitingForHeader,
        AccumulateChannels,
        ReadFlags,
        ValidateFooter
    };

    ParserState pstate_;
    uint8_t byte_count_;
    uint8_t raw_data_[23];  // 22 channel bytes + 1 flag byte
    uint8_t flags_;
};
```

2. **Channel unpacking algorithm**:
```cpp
void SBusParser::UnpackChannels() {
    // Extract 11-bit channels from packed byte array
    msg_.channels[0]  = (raw_data_[0]    | raw_data_[1]<<8)                 & 0x07FF;
    msg_.channels[1]  = (raw_data_[1]>>3 | raw_data_[2]<<5)                 & 0x07FF;
    msg_.channels[2]  = (raw_data_[2]>>6 | raw_data_[3]<<2 | raw_data_[4]<<10) & 0x07FF;
    msg_.channels[3]  = (raw_data_[4]>>1 | raw_data_[5]<<7)                 & 0x07FF;
    // ... continue for all 16 channels
}
```

3. **Hardware considerations**:
   - **Signal inversion**: SBUS uses inverted UART signal
   - **STM32 solutions**:
     - GPIO RX inversion (USART_CR2 register RXINV bit)
     - External inverter chip (74HC04, transistor circuit)
     - Software inversion (XOR byte with 0xFF - not recommended)

4. **Add to SerialRx.cpp**:
```cpp
case SBUS:
    parser_ = new SBusParser();
    // Configure USART for inverted RX if needed
    break;
```

5. **Validation approach**:
   - Dual-USART loopback (same as IBus)
   - Real receiver: FrSky X8R or Futaba R3008SB
   - Target: Zero frame loss in 5-second test

**Key Differences from IBus**:
- ✅ **No checksum**: Simpler validation (header + footer only)
- ⚠️ **Signal inversion**: Requires hardware or USART config
- ✅ **Packed channels**: Bit manipulation for 11-bit extraction
- ✅ **More channels**: 16 channels vs 14 (IBus)
- ✅ **Digital flags**: Ch17-18 as digital channels, frame lost, failsafe bits

**Extensibility Benefits**:
- Same `ProtocolParser` interface
- Same `update()` polling loop
- Same timeout/failsafe mechanism
- Drop-in protocol switching at runtime

### CRSF Extension Strategy (Future)

CRSF (TBS Crossfire) is more complex due to variable-length frames.

**Protocol Specifications**:
- **Baudrate**: 420000 baud
- **Frame size**: Variable (depends on frame type)
- **Channels**: 16 channels @ 10-bit resolution (0-1023 range)
- **Checksum**: CRC8-DVB-S2 polynomial

**Frame Structure** (RC Channels packet):
```
Byte 0:    Device address (0xC8 for RC channels)
Byte 1:    Frame length (excluding address and CRC)
Byte 2:    Frame type (0x16 for RC channels)
Byte 3-24: 16 channels × 11 bits packed
Byte 25:   CRC8
Total: 26 bytes for RC channels packet
```

**Implementation Complexity**:
- Variable-length frames require length byte parsing
- CRC8 calculation more complex than checksum
- Multiple frame types (RC channels, telemetry, GPS, etc.)
- Higher baudrate requires careful UART configuration

**Recommended Approach**:
- Implement SBUS first (simpler, more common)
- Use SBUS experience to design CRSF parser
- Consider separate CRSFMessage type for telemetry frames

## References

- **IBus Protocol**: FlySky iBus specification
- **Hardware Validation**: `libraries/SerialRx/examples/IBus_Loopback_Test/README.md`
- **Technical Documentation**: `doc/SERIAL.md`

## License

Part of the STM32 Arduino Core UAV project.

---

**Hardware Validated**: ✅ Loopback testing + Real FlySky FS-iA6B receiver
**Production Status**: Ready for flight controller integration
