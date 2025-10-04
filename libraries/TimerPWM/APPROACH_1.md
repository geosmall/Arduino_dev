# TimerPWM Library - Finalized Design Approach

## Research Phase Complete ✅

**Branch**: `timer-pwm-lib`
**Status**: Phase 1 Research Complete → Ready for Implementation

---

## Project Goal

Create a hardware timer-based PWM library for STM32 Arduino that provides **1 µs resolution** for servo and ESC control on UAV flight controllers.

### Target Use Cases
- **Servo Control**: 50-200 Hz with 1000-2000 µs pulses
- **ESC Control**: 400 Hz with 1000-2000 µs pulses
- **OneShot125**: 1-2 kHz with 125-250 µs pulses (Phase 4)

---

## Research Summary

### Documentation Created
1. **`doc/TIMERS.md`** - Complete STM32 timer architecture reference
   - HardwareTimer → HAL → LL → Register stack
   - RCC clock tree and 2× multiplier rule
   - Pin mapping resolution
   - All abstraction layers documented

2. **`doc/TIMERS_PWM_OUT.md`** - Practical PWM output guide
   - Servo and OneShot125 configuration examples
   - Complete calculation chain (RCC → PSC → ARR → CCRx)
   - Working code examples with HardwareTimer

3. **UVOS TimerPWM Analysis** - Reference implementation study
   - Confirmed 1 MHz tick rate approach
   - Validated prescaler calculation
   - Array-based channel configuration pattern

4. **Arduino Library Survey** - Comparative analysis
   - Arduino Servo library (time-multiplexing pattern)
   - ServoHT library (hardware PWM per channel)
   - ESP32 LEDC API (channel management)
   - Betaflight/iNav flight controller libraries (resource ownership)
   - HardwareTimer API capabilities

### Key Technical Findings

#### 1. RCC Clock Rule (Critical)
**STM32 Timer Clock Multiplier**:
```
If APB prescaler = 1 → Timer Clock = APB Clock (1×)
If APB prescaler > 1 → Timer Clock = 2 × APB Clock (2×)
```

**STM32F411RE @ 100 MHz** (typical Arduino configuration):
```
SYSCLK = 100 MHz
→ AHB /1 → HCLK = 100 MHz
→ APB1 /4 → PCLK1 = 25 MHz
→ Timer ×2 (because /4 > 1) → TIM3 Clock = 50 MHz
```

**Always query dynamically**:
```cpp
uint32_t timer_clk = mytimer->getTimerClkFreq();  // Returns 50000000
```

#### 2. HardwareTimer Capabilities
Arduino STM32 Core provides powerful features:
- **MICROSEC_COMPARE_FORMAT**: Direct microsecond pulse width setting
- **setOverflow()** with **MICROSEC_FORMAT**: Easy frequency control
- **Auto pin configuration**: No manual alternate function setup
- **Dynamic prescaler calculation**: Built-in helpers

#### 3. Flight Controller Validation
Betaflight and iNav confirm our design constraints:
- ✅ **Same timer = same frequency** (all channels share PSC/ARR)
- ✅ **Explicit timer banks** (resource ownership visibility)
- ✅ **Protocol isolation** (servos @ 50Hz, ESCs @ 400Hz on different timers)
- ✅ **1 MHz tick rate** (industry standard for servo/ESC)

---

## Finalized Design

### Architecture: Explicit Timer Bank Configuration

```cpp
// In targets/*.h files
namespace BoardConfig {
  namespace Servo {
    static constexpr TIM_TypeDef* timer = TIM3;
    static constexpr uint32_t frequency_hz = 50;  // 50 Hz for servos

    struct Channels {
      uint32_t pin;
      uint32_t channel;
      uint32_t min_us;
      uint32_t max_us;
    };

    static constexpr Channels channels[] = {
      {PA0, 1, 1000, 2000},  // S1 - TIM3_CH1
      {PA1, 2, 1000, 2000},  // S2 - TIM3_CH2
      {PA2, 3, 1000, 2000},  // S3 - TIM3_CH3
      {PA3, 4, 1000, 2000}   // S4 - TIM3_CH4
    };
  };

  namespace ESC {
    static constexpr TIM_TypeDef* timer = TIM5;
    static constexpr uint32_t frequency_hz = 400;  // 400 Hz for ESCs

    static constexpr Channels channels[] = {
      {PA0, 1, 1000, 2000},  // M1 - TIM5_CH1
      {PA1, 2, 1000, 2000},  // M2 - TIM5_CH2
      {PA2, 3, 1000, 2000},  // M3 - TIM5_CH3
      {PA3, 4, 1000, 2000}   // M4 - TIM5_CH4
    };
  };
}
```

**Why Explicit Configuration**:
1. ✅ **Prevents timer conflicts** - Visible at compile-time
2. ✅ **Shows frequency constraints** - All channels share same frequency
3. ✅ **Hardware resource visibility** - Designer sees allocation
4. ✅ **Config IS documentation** - Self-documenting hardware setup

### API Design: Hybrid Servo + HardwareTimer

Combines **Arduino Servo simplicity** with **HardwareTimer power**:

```cpp
class PWMOutputBank {
public:
  //
  // Initialization
  //

  // Initialize timer with target frequency
  // frequency_hz: PWM frequency (50 = servo, 400 = ESC, 1000-2000 = OneShot125)
  // Returns: true on success, false on error
  bool Init(TIM_TypeDef *timer, uint32_t frequency_hz = 50);

  //
  // Channel Management (Servo-style)
  //

  // Attach a channel to a pin with optional pulse width limits
  // channel: Timer channel (1-4)
  // pin: Arduino pin number (PA0, PB1, etc.)
  // min_us: Minimum pulse width in microseconds (default 1000)
  // max_us: Maximum pulse width in microseconds (default 2000)
  // Returns: true on success, false on error
  bool AttachChannel(uint32_t channel, uint32_t pin,
                     uint32_t min_us = 1000, uint32_t max_us = 2000);

  // Detach a channel (stop PWM output)
  void DetachChannel(uint32_t channel);

  //
  // Pulse Width Control (Microsecond API)
  //

  // Set pulse width for a specific channel in microseconds
  // channel: Timer channel (1-4)
  // pulse_us: Pulse width in microseconds (e.g., 1500)
  void SetPulseWidth(uint32_t channel, uint32_t pulse_us);

  // Set same pulse width for all attached channels
  void SetPulseWidthAll(uint32_t pulse_us);

  //
  // Arduino Servo Compatibility
  //

  // Write angle (0-180) or microseconds (>= 200) to a channel
  // channel: Timer channel (1-4)
  // value: Angle (0-180) or microseconds if >= 200
  void Write(uint32_t channel, int value);

  // Write microseconds (alias for SetPulseWidth)
  void WriteMicroseconds(uint32_t channel, uint32_t us);

  //
  // Control
  //

  // Start PWM generation on all channels
  void Start();

  // Stop PWM generation on all channels
  void Stop();

  // Get number of attached channels
  uint8_t GetChannelCount() const;

private:
  HardwareTimer* timer_;
  uint32_t frequency_hz_;

  struct ChannelConfig {
    uint32_t channel;
    uint32_t pin;
    uint32_t min_us;
    uint32_t max_us;
    bool active;
  };

  ChannelConfig channels_[4];  // Max 4 channels per timer
  uint8_t channel_count_;

  // Helper: Calculate 1 MHz prescaler
  uint32_t Calculate1MHzPrescaler();

  // Helper: Validate channel number (1-4)
  bool IsValidChannel(uint32_t channel) const;

  // Helper: Clamp pulse width to min/max limits
  uint32_t ClampPulseWidth(uint32_t channel, uint32_t pulse_us) const;
};
```

### Usage Pattern

```cpp
#include <PWMOutputBank.h>
#include "targets/NUCLEO_F411RE_SERVO.h"

PWMOutputBank servos;
PWMOutputBank escs;

void setup() {
  //
  // Servo Setup (TIM3 @ 50 Hz)
  //
  servos.Init(BoardConfig::Servo::timer, BoardConfig::Servo::frequency_hz);

  for (auto& ch : BoardConfig::Servo::channels) {
    servos.AttachChannel(ch.channel, ch.pin, ch.min_us, ch.max_us);
  }

  servos.Start();

  //
  // ESC Setup (TIM5 @ 400 Hz)
  //
  escs.Init(BoardConfig::ESC::timer, BoardConfig::ESC::frequency_hz);

  for (auto& ch : BoardConfig::ESC::channels) {
    escs.AttachChannel(ch.channel, ch.pin, ch.min_us, ch.max_us);
  }

  escs.Start();
}

void loop() {
  // Microsecond control (servo center)
  servos.SetPulseWidth(1, 1500);

  // Angle control (Arduino Servo style)
  servos.Write(2, 90);  // 90 degrees

  // ESC throttle (microseconds)
  escs.SetPulseWidth(1, 1200);  // 20% throttle

  delay(20);
}
```

---

## Key Technical Decisions

| Aspect | Decision | Rationale |
|--------|----------|-----------|
| **Resolution** | 1 MHz (1 µs) | Industry standard, validated by UVOS/Betaflight |
| **API Style** | Servo-like + explicit timer | Familiar + clear resource ownership |
| **Pin Format** | Arduino uint32_t (PA0, PB1) | Matches BoardConfig pattern |
| **Prescaler** | Fixed 1 MHz for Phase 1 | Simplicity, auto-calc in Phase 4 |
| **Frequency** | Init-time parameter | Servo (50Hz) vs ESC (400Hz) |
| **Multi-Timer** | Separate PWMOutputBank instances | Clear ownership, no conflicts |
| **Channel Limit** | 4 per timer | Hardware constraint |
| **Pulse Format** | Microseconds (uint32_t) | Direct, no conversion needed |
| **HardwareTimer** | Wrapper, not raw HAL | Arduino ecosystem compatibility |

---

## Implementation Phases

### ✅ Phase 0 - Research (COMPLETED)
- [x] HardwareTimer API research
- [x] RCC clock tree understanding
- [x] Architecture docs (TIMERS.md, TIMERS_PWM_OUT.md)
- [x] UVOS TimerPWM analysis
- [x] Arduino library survey
- [x] Configuration approach design
- [x] Pin convention decision
- [x] Finalized API design

### 📋 Phase 1 - Basic Implementation (NEXT)
- [ ] Create `src/PWMOutputBank.h` header
- [ ] Create `src/PWMOutputBank.cpp` implementation
- [ ] Implement `Init()` with 1 MHz prescaler calculation
- [ ] Implement `AttachChannel()` for single channel
- [ ] Implement `SetPulseWidth()` using HardwareTimer
- [ ] Implement `Start()/Stop()`
- [ ] Create `examples/SimplePWM/` - Basic LED fade example
- [ ] HIL validation on NUCLEO_F411RE

### 📋 Phase 2 - Multi-Channel Support
- [ ] Support up to 4 channels per timer
- [ ] Array-based channel tracking
- [ ] Per-channel pulse width updates
- [ ] `SetPulseWidthAll()` broadcast method
- [ ] Create `examples/MultiServo/` - 4 servos on TIM3
- [ ] Create `targets/NUCLEO_F411RE_SERVO.h` config
- [ ] HIL validation with oscilloscope

### 📋 Phase 3 - Arduino Compatibility
- [ ] Implement `Write()` method (angle/microsecond auto-detect)
- [ ] Implement `WriteMicroseconds()` alias
- [ ] Implement `DetachChannel()` method
- [ ] Add channel validation (min/max limits)
- [ ] Create `examples/ServoSweep/` - Arduino Servo compatible
- [ ] Create `examples/ESC_Calibration/` - ESC setup guide
- [ ] Documentation and API reference

### 📋 Phase 4 - Production Features
- [ ] Multi-timer support validation (independent instances)
- [ ] BoardConfig integration examples
- [ ] OneShot125 protocol support (1-2 kHz)
- [ ] Error handling and diagnostics
- [ ] Create `targets/NOXE_V3_FLIGHT.h` - Flight controller config
- [ ] HIL validation suite
- [ ] Performance benchmarking

---

## Hardware Constraints (Validated)

### Timer Hardware Limits
- **Max 4 channels per timer** (STM32 hardware)
- **All channels share same frequency** (same PSC/ARR registers)
- **16-bit counters** (except TIM2/TIM5 are 32-bit)
- **PSC range**: 0-65535
- **ARR range**: 0-65535 (16-bit), 0-4294967295 (32-bit)

### Resolution vs Frequency Tradeoff
```
Resolution (ticks) = f_counter / f_pwm

Example @ 1 MHz counter:
- 50 Hz   → 20,000 ticks (~14.3 bit resolution)
- 200 Hz  → 5,000 ticks  (~12.3 bit resolution)
- 400 Hz  → 2,500 ticks  (~11.3 bit resolution)
- 1 kHz   → 1,000 ticks  (~10.0 bit resolution)
- 2 kHz   → 500 ticks    (~9.0 bit resolution)
```

**Practical Ranges**:
- **Servo**: 50-200 Hz (standard to high-speed)
- **ESC Standard**: 50-490 Hz
- **ESC OneShot125**: 1-2 kHz
- **Resolution needed**: 1 µs sufficient for all use cases

### Recommended Timer Selection
**APB1 Timers** (50 MHz on F411RE):
- **TIM3**: 4 channels, well-documented, servo-friendly
- **TIM5**: 4 channels, 32-bit (extended range)
- TIM2, TIM4: Alternative options

**APB2 Timers** (100 MHz on F411RE):
- **TIM1**: Advanced timer, 4 channels + complementary
- TIM9, TIM10, TIM11: 1-2 channels (limited)

**Recommendation**: Use TIM3/TIM5 for servo/ESC applications.

---

## Library Structure

```
libraries/TimerPWM/
├── src/
│   ├── PWMOutputBank.h        # Main library class
│   └── PWMOutputBank.cpp      # Implementation
├── examples/
│   ├── SimplePWM/             # Basic PWM output (Phase 1)
│   ├── PWM_Verification/      # Input capture verification (Phase 1)
│   ├── MultiServo/            # 4 servos on one timer (Phase 2)
│   ├── ServoSweep/            # Arduino Servo compatible (Phase 3)
│   ├── ESC_Calibration/       # ESC setup guide (Phase 3)
│   └── MixedOutputs/          # Servos + ESCs (Phase 4)
├── APPROACH_0.md              # Initial research document
├── APPROACH_1.md              # This file (finalized design)
└── README.md                  # User documentation

targets/
├── NUCLEO_F411RE_SERVO.h      # 4 servos (TIM3 @ 50 Hz)
├── NUCLEO_F411RE_ESC.h        # 4 ESCs (TIM5 @ 400 Hz)
├── NUCLEO_F411RE_ONESHOT125.h # 4 ESCs (TIM8 @ 1 kHz) - Phase 4
└── NOXE_V3_FLIGHT.h           # Flight controller config - Phase 4
```

---

## PWM Verification Strategy (Phase 1)

### Overview
Before implementing production features, we need reliable verification of PWM output without external test equipment (oscilloscope/logic analyzer). This approach uses **Input Capture mode** with hardware timer measurement via jumper wire loopback.

### Method: Input Capture Loopback

**Hardware Setup**:
```
PA6 (TIM3_CH1 PWM Output) → [Jumper Wire] → PA0 (TIM2_CH1 Input Capture)
```

**Advantages**:
- ✅ Validates actual pin output (not just register values)
- ✅ Measures both frequency and pulse width
- ✅ No external tools needed (just 1 jumper wire)
- ✅ Hardware-accurate timing measurement
- ✅ Integrates with HIL framework for automated testing

**Implementation Pattern**:

```cpp
// PWM_Verification.ino - Phase 1 validation example

#include <ci_log.h>

// PWM Output on TIM3
HardwareTimer tim3(TIM3);

// Input Capture on TIM2
HardwareTimer tim2(TIM2);
volatile uint32_t capture_rising = 0;
volatile uint32_t capture_falling = 0;
volatile bool measurement_ready = false;

void setup() {
  CI_LOG_INIT();
  CI_LOG("=== PWM Verification Test ===\n");
  CI_BUILD_INFO();

  // Configure PWM Output (TIM3 @ 50 Hz, 1500 µs pulse)
  tim3.setPrescaleFactor(99);  // 100 MHz / 100 = 1 MHz tick
  tim3.setOverflow(20000, MICROSEC_FORMAT);  // 50 Hz (20 ms period)
  tim3.setCaptureCompare(1, 1500, MICROSEC_COMPARE_FORMAT);  // 1500 µs pulse
  tim3.setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA6);
  tim3.resume();

  CI_LOG("PWM Output: PA6, 50 Hz, 1500 µs pulse\n");

  // Configure Input Capture (TIM2 measuring rising edge)
  tim2.setMode(1, TIMER_INPUT_CAPTURE_RISING, PA0);
  tim2.setPrescaleFactor(99);  // Match TIM3 tick rate (1 MHz)
  tim2.setOverflow(0xFFFFFFFF);  // Max period
  tim2.attachInterrupt(1, captureCallback);
  tim2.resume();

  CI_LOG("Input Capture: PA0 measuring period\n");
  CI_LOG("Connect jumper: PA6 → PA0\n\n");
  CI_READY_TOKEN();
}

void captureCallback() {
  static uint32_t last_capture = 0;
  uint32_t current_capture = tim2.getCaptureCompare(1);

  // Calculate period in microseconds
  uint32_t period_us = current_capture - last_capture;
  last_capture = current_capture;

  if (period_us > 10000 && period_us < 30000) {  // Valid 50 Hz range
    capture_rising = period_us;
    measurement_ready = true;
  }
}

void loop() {
  if (measurement_ready) {
    measurement_ready = false;

    float measured_freq = 1000000.0 / capture_rising;  // Hz

    CI_LOG("Period: %lu µs, Freq: %.2f Hz\n", capture_rising, measured_freq);

    // Validation with ±2% tolerance
    bool freq_valid = (measured_freq >= 49.0 && measured_freq <= 51.0);

    if (freq_valid) {
      CI_LOG("✓ PASS: Frequency within tolerance\n");
    } else {
      CI_LOG("✗ FAIL: Frequency out of range\n");
    }
  }

  delay(1000);
}
```

**Expected Results**:
```
Period: 20000 µs, Freq: 50.00 Hz
✓ PASS: Frequency within tolerance
```

**Pulse Width Measurement** (Advanced):
Use dual-edge capture (rising + falling) to measure pulse width:
```cpp
// Configure for both edges
tim2.setMode(1, TIMER_INPUT_CAPTURE_BOTHEDGE, PA0);

void captureCallback() {
  static bool last_edge_rising = true;
  static uint32_t rising_time = 0;

  uint32_t capture_time = tim2.getCaptureCompare(1);
  bool current_edge_rising = digitalRead(PA0);

  if (current_edge_rising && !last_edge_rising) {
    // Rising edge - store timestamp
    rising_time = capture_time;
  } else if (!current_edge_rising && last_edge_rising) {
    // Falling edge - calculate pulse width
    uint32_t pulse_width = capture_time - rising_time;
    CI_LOG("Pulse: %lu µs\n", pulse_width);
  }

  last_edge_rising = current_edge_rising;
}
```

**HIL Integration**:
- Works with `--use-rtt` flag for automated testing
- Validation logic outputs PASS/FAIL for deterministic test results
- Can verify multiple pulse widths (1000-2000 µs range)
- Integrates with build traceability (`--build-id`)

**Alternative Methods** (documented for reference):
1. **DWT Cycle Counter + GPIO Polling** - Software polling with cycle-accurate timing
2. **External Interrupt + DWT** - EXTI on edges with DWT timestamps
3. **Timer Synchronization** - Master/slave timer configuration (ITR signals)

**Verification Checklist**:
- [ ] 50 Hz frequency measurement (±2% tolerance)
- [ ] 1500 µs pulse width measurement (±10 µs tolerance)
- [ ] Variable pulse width validation (1000-2000 µs)
- [ ] Multi-channel verification (channels 1-4)
- [ ] HIL automated test integration

---

## Implementation Details

### 1 MHz Prescaler Calculation
```cpp
uint32_t PWMOutputBank::Calculate1MHzPrescaler() {
  uint32_t timer_clk = timer_->getTimerClkFreq();  // Dynamic query
  uint32_t prescaler = (timer_clk / 1'000'000) - 1;  // 1 MHz tick rate
  return prescaler;
}
```

**Example** (STM32F411RE with TIM3):
- Timer clock: 50 MHz (from RCC)
- Target: 1 MHz (1 µs tick)
- Prescaler: (50,000,000 / 1,000,000) - 1 = **49**

### Frequency to Period Conversion
```cpp
void PWMOutputBank::Init(TIM_TypeDef *timer, uint32_t frequency_hz) {
  timer_ = new HardwareTimer(timer);
  frequency_hz_ = frequency_hz;

  // Calculate and set 1 MHz prescaler
  uint32_t prescaler = Calculate1MHzPrescaler();
  timer_->setPrescaleFactor(prescaler);

  // Set period from frequency (at 1 MHz tick rate)
  // Example: 50 Hz → 20,000 µs period
  uint32_t period_us = (1'000'000 / frequency_hz);
  timer_->setOverflow(period_us, MICROSEC_FORMAT);
}
```

### Channel Attachment
```cpp
bool PWMOutputBank::AttachChannel(uint32_t channel, uint32_t pin,
                                   uint32_t min_us, uint32_t max_us) {
  if (!IsValidChannel(channel)) return false;

  // Configure channel for PWM output
  timer_->setMode(channel, TIMER_OUTPUT_COMPARE_PWM1, pin);

  // Store channel configuration
  uint8_t idx = channel - 1;  // Convert to 0-indexed
  channels_[idx].channel = channel;
  channels_[idx].pin = pin;
  channels_[idx].min_us = min_us;
  channels_[idx].max_us = max_us;
  channels_[idx].active = true;

  channel_count_++;
  return true;
}
```

### Pulse Width Setting
```cpp
void PWMOutputBank::SetPulseWidth(uint32_t channel, uint32_t pulse_us) {
  if (!IsValidChannel(channel)) return;

  uint8_t idx = channel - 1;
  if (!channels_[idx].active) return;

  // Clamp to min/max limits
  pulse_us = ClampPulseWidth(channel, pulse_us);

  // Set compare value in microseconds (HardwareTimer handles conversion)
  timer_->setCaptureCompare(channel, pulse_us, MICROSEC_COMPARE_FORMAT);
}
```

### Arduino Servo Compatibility
```cpp
void PWMOutputBank::Write(uint32_t channel, int value) {
  // Smart detection: value < 200 = angle, >= 200 = microseconds
  if (value < 200) {
    // Convert angle (0-180) to microseconds (1000-2000)
    uint32_t pulse_us = map(value, 0, 180, 1000, 2000);
    SetPulseWidth(channel, pulse_us);
  } else {
    // Direct microsecond value
    SetPulseWidth(channel, value);
  }
}
```

---

## Design Strengths

### 1. Simple for Basic Use
```cpp
PWMOutputBank servos;
servos.Init(TIM3, 50);              // 50 Hz servo frequency
servos.AttachChannel(1, PA0);       // Attach servo to PA0
servos.Start();                     // Start PWM
servos.SetPulseWidth(1, 1500);      // Center position
```

### 2. Powerful for Advanced Use
```cpp
PWMOutputBank fast_escs;
fast_escs.Init(TIM5, 400);          // 400 Hz ESC update rate
fast_escs.AttachChannel(1, PA0, 1000, 2000);
fast_escs.Write(1, 1200);           // Direct microseconds
```

### 3. Clear Resource Management
- **One PWMOutputBank = one timer** (explicit ownership)
- **Explicit frequency in Init()** (prevents conflicts)
- **No hidden timer sharing** (predictable behavior)

### 4. BoardConfig Integration Ready
```cpp
#include "targets/NUCLEO_F411RE_SERVO.h"
servos.Init(BoardConfig::Servo::timer, BoardConfig::Servo::frequency_hz);
// Config shows complete hardware mapping
```

### 5. HIL Testable
- Standard Arduino sketch structure
- Works with ci_log.h framework
- Oscilloscope validation possible
- Deterministic timing for automated tests

### 6. Arduino Ecosystem Compatible
- Familiar Servo-like API
- Works with Arduino IDE and arduino-cli
- Leverages HardwareTimer (no raw HAL)
- Auto pin configuration via core

---

## What We Learned from Research

### From UVOS TimerPWM
✅ 1 MHz tick rate is correct approach
✅ Prescaler calculation formula validated
✅ Array-based channel config scales well
✅ Two-step init (configure → start) makes sense

### From Arduino Servo Library
✅ write() method with angle/µs detection is intuitive
✅ attach() with min/max calibration is useful
❌ Time-multiplexing not needed (abundant hardware channels)

### From ServoHT Library
✅ Hardware PWM per channel is the right pattern
✅ Dynamic attach() allocation works well
✅ Can wrap HardwareTimer cleanly

### From Betaflight/iNav
✅ Explicit timer banks prevent conflicts
✅ Same timer = same protocol constraint is real
✅ Resource ownership flags improve clarity
✅ Flight controllers validate our use cases

### From HardwareTimer API
✅ MICROSEC_COMPARE_FORMAT simplifies pulse width setting
✅ setOverflow() with MICROSEC_FORMAT is perfect for our needs
✅ Auto pin configuration eliminates manual AF setup
✅ Dynamic clock query handles RCC complexity

---

## Critical Constraints Summary

1. ✅ **Same timer = same frequency** (hardware: shared PSC/ARR)
2. ✅ **1 MHz tick = 1 µs resolution** (industry standard)
3. ✅ **Max 4 channels per timer** (hardware limit)
4. ✅ **Dynamic clock query required** (RCC 2× multiplier rule)
5. ✅ **Arduino pin numbers** (BoardConfig consistency)
6. ✅ **Explicit timer ownership** (prevents runtime conflicts)

---

## Next Steps

### Immediate (Start of Implementation)
1. Create `src/PWMOutputBank.h` with complete API
2. Create `src/PWMOutputBank.cpp` with basic implementation
3. Implement `Init()`, `AttachChannel()`, `SetPulseWidth()`, `Start()`
4. Create `examples/SimplePWM/SimplePWM.ino` - single LED fade
5. Test on NUCLEO_F411RE with oscilloscope validation

### Short-Term (Phase 2)
1. Add multi-channel support (4 channels)
2. Create servo control example
3. Create target configuration file
4. HIL validation with multiple servos

### Long-Term (Phase 3-4)
1. Arduino Servo compatibility layer
2. ESC calibration example
3. OneShot125 protocol support
4. Flight controller configuration example

---

## Success Criteria

**Phase 1 Complete When**:
- ✅ Single timer, single channel works
- ✅ 1 MHz tick rate verified (oscilloscope)
- ✅ Pulse width accuracy ±1 µs
- ✅ Example compiles and runs on NUCLEO_F411RE

**Phase 2 Complete When**:
- ✅ 4 channels work independently
- ✅ Per-channel pulse width control
- ✅ Servo example moves 4 servos smoothly

**Phase 3 Complete When**:
- ✅ Arduino Servo API compatibility
- ✅ ESC calibration documented
- ✅ Min/max limiting works

**Phase 4 Complete When**:
- ✅ Multi-timer support validated
- ✅ OneShot125 protocol works
- ✅ Flight controller config example
- ✅ HIL test suite passes

---

## Reference Material

### Local Documentation
- `doc/TIMERS.md` - Complete timer architecture reference
- `doc/TIMERS_PWM_OUT.md` - Practical PWM output guide
- `libraries/TimerPWM/TimerPWM.cpp` - UVOS reference implementation
- `Arduino_Core_STM32/cores/arduino/HardwareTimer.h` - Core API

### External References
- [HardwareTimer Wiki](https://github.com/stm32duino/Arduino_Core_STM32/wiki/HardwareTimer-library)
- [STM32Examples PWM_FullConfiguration](https://github.com/stm32duino/STM32Examples/blob/main/examples/Peripherals/HardwareTimer/PWM_FullConfiguration/)
- [Arduino Servo Library](https://github.com/arduino-libraries/Servo)
- [ServoHT STM32 Hardware Timer Servo](https://github.com/PR-DC/PRDC_ServoHT)
- STM32F4 Reference Manual RM0383 (Timers: Chapter 13-18)

---

**Research Phase: COMPLETE ✅**
**Ready for Phase 1 Implementation**
