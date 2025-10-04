# TimerPWM Library - Design Approach

## Project Status: Phase 1 - Research & Design (In Progress)

**Branch**: `timer-pwm-lib`

## Project Goal

Create an Arduino-compatible hardware timer library for STM32 that provides high-resolution (1 µs) PWM output for servo and ESC control on UAV flight controllers.

## Use Cases

### Primary Targets
1. **Servo Control**: 200 Hz with 1000-2000 µs pulses (1 µs resolution)
2. **ESC Control - OneShot125**: 125-250 µs pulses at 1-2 kHz

### Hardware Configuration
- **1-4 PWM channels** per timer bank
- **Multiple timer banks** to support 8+ outputs
- **Explicit timer mapping** in board configuration
- **Mixed frequencies** across banks (servos @ 200 Hz, ESCs @ 1 kHz)

## Architecture Research Completed

### Documentation Created
1. **`doc/TIMERS.md`**: Comprehensive technical reference
   - Complete architecture stack (HardwareTimer → HAL → LL → Registers)
   - RCC clock tree and timer derivation
   - Pin mapping resolution
   - All abstraction layers documented

2. **`doc/TIMERS_PWM_OUT.md`**: Practical PWM output guide
   - Servo and OneShot125 configuration examples
   - Complete calculation chain (RCC → PSC → ARR → CCRx → GPIO)
   - Working code examples
   - Best practices and common pitfalls

### Key Technical Findings

#### RCC Clock Rule (Critical)
**STM32 Timer Clock Multiplier**:
- If APB prescaler = 1 → Timer Clock = APB Clock (1× multiplier)
- If APB prescaler > 1 → Timer Clock = 2 × APB Clock (2× multiplier)

**STM32F411RE @ 100 MHz** (typical Arduino configuration):
```
SYSCLK = 100 MHz
↓ AHB /1 → HCLK = 100 MHz
↓ APB1 /4 → PCLK1 = 25 MHz
↓ Timer ×2 (because /4 > 1) → TIM3 Clock = 50 MHz ✓
```

**Always query dynamically**:
```cpp
uint32_t timer_clock = mytimer->getTimerClkFreq();  // Returns 50000000
```

#### Timer Hardware Selection
**APB1 Timers** (50 MHz on F411):
- TIM2, TIM3, TIM4, TIM5 (4 channels each)
- TIM5 is 32-bit (extended range)

**APB2 Timers** (100 MHz on F411):
- TIM1 (advanced, 4 channels + complementary)
- TIM9, TIM10, TIM11 (1-2 channels)

**Recommendation**: Use TIM3/TIM5 for servo/ESC (APB1, 4 channels, well-documented)

#### PWM Frequency Formula
```
f_pwm = f_timer_clock / [(PSC + 1) × (ARR + 1)]

Where:
- PSC = Prescaler register (0-65535)
- ARR = Auto-Reload register (0-65535 for 16-bit timers)
- CCRx = Capture/Compare register (pulse width)
```

**For 1 µs resolution**:
```cpp
PSC = (timer_clock_hz / 1'000'000) - 1;  // 1 MHz counter
// Example: (50 MHz / 1 MHz) - 1 = 49
```

## Configuration Design Decision: Explicit Timer Banks

### Why Explicit vs Auto-Detection

**Decision**: Use **explicit timer bank configuration** in target files.

**Rationale**:
1. ✅ **Timer Conflict Prevention**: Same timer, different frequency → visible at config time
2. ✅ **Frequency Constraints**: All channels on a timer share same frequency (PSC/ARR)
3. ✅ **Hardware Resource Visibility**: Designer sees timer allocation at a glance
4. ✅ **Performance Planning**: Know exactly which timers are free
5. ✅ **Compile-time Validation**: Catch conflicts before runtime
6. ✅ **Documentation**: Config file IS the documentation

**Rejected**: Auto-detection from pin list
- Hides timer conflicts until runtime
- No visibility into resource allocation
- Can't validate frequency constraints
- Difficult to debug multi-bank setups

### Configuration Structure

**Hybrid Approach**: Explicit banks + unified array interface

#### Target Configuration Pattern
```cpp
// targets/NUCLEO_F411RE_SERVO.h
namespace BoardConfig {
  // Explicit timer bank definitions
  struct ServoBank1 {
    static constexpr TIM_TypeDef* timer = TIM3;
    static constexpr uint32_t frequency_hz = 200;
    static constexpr uint32_t pins[] = {PA6, PA7, PB0, PB1};  // Arduino pin numbers
    static constexpr uint8_t channels[] = {1, 2, 3, 4};       // Timer channels
    static constexpr uint8_t count = 4;
  };

  struct ServoBank2 {
    static constexpr TIM_TypeDef* timer = TIM5;
    static constexpr uint32_t frequency_hz = 200;
    static constexpr uint32_t pins[] = {PA0, PA1, PA2, PA3};
    static constexpr uint8_t channels[] = {1, 2, 3, 4};
    static constexpr uint8_t count = 4;
  };

  struct ESCBank {
    static constexpr TIM_TypeDef* timer = TIM8;
    static constexpr uint32_t frequency_hz = 1000;  // OneShot125
    static constexpr uint32_t pins[] = {PC6, PC7, PC8, PC9};
    static constexpr uint8_t channels[] = {1, 2, 3, 4};
    static constexpr uint8_t count = 4;
  };
}
```

#### Library API
```cpp
class PWMOutputBank {
public:
  // Initialize with one or more timer banks
  template<typename... BankConfigs>
  bool begin(BankConfigs... banks);

  // Unified array interface (0-indexed)
  void writeMicroseconds(uint8_t index, uint32_t pulse_us);
  void writePercent(uint8_t index, uint8_t percent);

  uint8_t count();  // Total pins across all banks

private:
  // Internal timer bank management
  struct TimerBank {
    HardwareTimer* timer;
    TIM_TypeDef* instance;
    uint32_t frequency_hz;
  };

  TimerBank _banks[4];  // Max 4 timer banks

  // Logical pin index → timer bank + channel mapping
  struct PinMapping {
    uint8_t bank_idx;
    uint32_t channel;
  };
  PinMapping _pin_map[16];  // Max 16 pins
};
```

#### Usage Pattern
```cpp
#include <PWMOutputBank.h>
#include "targets/NUCLEO_F411RE_SERVO.h"

PWMOutputBank servos;
PWMOutputBank escs;

void setup() {
  // Explicit: User sees TIM3 + TIM5 for servos
  servos.begin(BoardConfig::ServoBank1{}, BoardConfig::ServoBank2{});

  // TIM8 for ESCs at different frequency
  escs.begin(BoardConfig::ESCBank{});

  // Unified array interface
  servos.writeMicroseconds(0, 1500);  // S1 (TIM3_CH1, PA6)
  servos.writeMicroseconds(7, 1200);  // S8 (TIM5_CH4, PA3)

  escs.writeMicroseconds(0, 125);     // M1 (TIM8_CH1, PC6)
}
```

## Pin Number Convention: Arduino uint32_t

### Decision: Use Arduino Pin Numbers (uint32_t)

**Established Pattern** (from Storage/IMU configs):
```cpp
StorageConfig storage{StorageBackend::LITTLEFS, PC12, PC11, PC10, PD2, 1000000};
IMUConfig imu{imu_spi, PC4};
```

Where:
- `PC12` → `#define PC12 17` (variant-specific)
- `PA6` → `#define PA6 16` (variant-specific)
- Type: `uint32_t` (Arduino pin number)

**NOT PinName** (STM32 HAL enums like `PA_6`, `PD_2`)

### Why uint32_t (Arduino Pin Numbers)

1. ✅ **Consistency**: Matches existing BoardConfig pattern
2. ✅ **Readability**: `PA6`, `PD2` are clear and portable
3. ✅ **Board-specific**: Variant files handle mapping automatically
4. ✅ **Core Compatibility**: HardwareTimer accepts both, converts internally
5. ✅ **Familiar**: Arduino users expect pin numbers

### How It Works

**Arduino Core Conversion** (automatic):
```cpp
// HardwareTimer.cpp:624
void HardwareTimer::setMode(uint32_t channel, TimerModes_t mode, uint32_t pin) {
  setMode(channel, mode, digitalPinToPinName(pin), filter);
  //                      ↑ Converts Arduino pin → PinName
}
```

**Our Config**:
```cpp
static constexpr uint32_t pins[] = {PA6, PA7, PB0, PB1};
// PA6 → #define PA6 16 → digitalPinToPinName(16) → PA_6 (PinName)
```

## Library Structure (Planned)

```
libraries/TimerPWM/
├── src/
│   ├── PWMOutputBank.h        // Main library class
│   ├── PWMOutputBank.cpp      // Implementation
│   └── TimerPWMConfig.h       // Config structure definition
├── examples/
│   ├── SimpleServo/           // Basic servo control
│   ├── SimpleESC/             // 4-motor OneShot125
│   └── MixedOutputs/          // Servos + ESCs
├── APPROACH.md                // This file
└── README.md                  // User documentation

targets/
├── NUCLEO_F411RE_SERVO.h      // 8 servos (TIM3 + TIM5 @ 200 Hz)
├── NUCLEO_F411RE_ESC.h        // 4 ESCs (TIM5 @ 400 Hz)
├── NUCLEO_F411RE_ONESHOT125.h // 4 ESCs (TIM8 @ 1 kHz)
└── README.md                  // How to create configs
```

## Implementation Status

### ✅ Completed (Phase 1 - Research)
- [x] HardwareTimer API research
- [x] Pin-to-timer mapping study
- [x] HAL/LL integration analysis
- [x] RCC clock tree understanding
- [x] Timer PWM examples analysis
- [x] Architecture documentation (TIMERS.md)
- [x] Practical guide (TIMERS_PWM_OUT.md)
- [x] Configuration approach design
- [x] Pin number convention decision

### 🔄 In Progress (Phase 1 - Research)
- [ ] Additional HardwareTimer pattern research
- [ ] Review existing Arduino timer libraries (if any)
- [ ] Validate config structure with actual pins
- [ ] Prototype timer bank initialization

### 📋 Next Phase (Phase 2 - Implementation)
- [ ] Create TimerPWMConfig.h structure
- [ ] Implement PWMOutputBank class
- [ ] Write SimpleServo example
- [ ] Write OneShot125 ESC example
- [ ] Create target configs (NUCLEO_F411RE)
- [ ] HIL validation with actual hardware

### 📋 Future Phases
**Phase 3**: Multi-Channel & Runtime Updates
- [ ] Runtime pulse width updates
- [ ] Frequency validation
- [ ] Error handling

**Phase 4**: Production Features
- [ ] Multiple timer support validation
- [ ] BoardConfig integration
- [ ] Documentation completion
- [ ] HIL test suite

## Design Constraints

### Timer Hardware Limits
- Max 4 channels per timer
- All channels share same PSC/ARR (frequency)
- 16-bit counters (except TIM2/TIM5 are 32-bit)
- PSC range: 0-65535
- ARR range: 0-65535 (16-bit), 0-4294967295 (32-bit)

### Resolution vs Frequency Tradeoff
```
Resolution (ticks) = f_counter / f_pwm

Example @ 1 MHz counter:
- 50 Hz   → 20,000 ticks (~14.3 bit)
- 200 Hz  → 5,000 ticks  (~12.3 bit)
- 1 kHz   → 1,000 ticks  (~10.0 bit)
- 2 kHz   → 500 ticks    (~9.0 bit)
```

### Practical Ranges
- **Servo**: 50-333 Hz (standard to high-speed)
- **ESC Standard**: 50-490 Hz
- **ESC OneShot125**: 1-2 kHz
- **Resolution needed**: 1 µs for servo/ESC control

## Key Reference Material

### Arduino Core Locations
- `cores/arduino/HardwareTimer.{h,cpp}` - Main API
- `cores/arduino/stm32/timer.{c,h}` - Clock/pin helpers
- `variants/STM32F4xx/*/PeripheralPins.c` - Pin mappings
- `system/Drivers/STM32F4xx_HAL_Driver/` - HAL layer

### Example Code Sources
- [STM32Examples/PWM_FullConfiguration](https://github.com/stm32duino/STM32Examples/blob/main/examples/Peripherals/HardwareTimer/PWM_FullConfiguration/PWM_FullConfiguration.ino)
- [STM32Examples/All-in-one_setPWM](https://github.com/stm32duino/STM32Examples/blob/main/examples/Peripherals/HardwareTimer/All-in-one_setPWM/All-in-one_setPWM.ino)

### Reference Documentation
- STM32F4 Reference Manual RM0383 (Timers: Chapter 13-18)
- [HardwareTimer Wiki](https://github.com/stm32duino/Arduino_Core_STM32/wiki/HardwareTimer-library)

## Open Questions for Next Session

1. **Config Structure Refinement**
   - Should we use struct or constexpr arrays for bank definitions?
   - Best way to handle channel validation at compile time?
   - Template vs runtime bank initialization?

2. **Error Handling**
   - How to detect timer conflicts at runtime?
   - Validation for pin-timer compatibility?
   - Frequency constraint checks?

3. **Additional Research Needed**
   - Are there existing Arduino timer libraries we should study?
   - Review UVOS TimerPWM implementation patterns more closely
   - Check for any STM32duino community PWM libraries

## Next Steps

### Immediate Tasks (Start of Next Session)
1. Review this APPROACH.md document
2. Continue Phase 1 research:
   - Study UVOS TimerPWM.cpp implementation details
   - Search for similar Arduino timer libraries
   - Validate proposed config structure with actual hardware
3. Finalize configuration structure design
4. Begin Phase 2 implementation

### Decision Points to Address
- Final config structure (template vs runtime)
- Error handling strategy
- Validation approach (compile-time vs runtime)
- Example priorities (what to implement first)

## Notes

- All code must be compatible with Arduino IDE and arduino-cli
- HIL testing framework integration from start (ci_log.h pattern)
- Follow existing BoardConfig pattern for consistency
- 1 µs resolution is non-negotiable for servo/ESC control
- Target boards: NUCLEO_F411RE (primary), BLACKPILL_F411CE (secondary)
