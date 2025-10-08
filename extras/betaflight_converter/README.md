# Betaflight to BoardConfig Converter

## Overview

Python tool that automatically converts Betaflight unified target configurations into Arduino STM32 BoardConfig headers with comprehensive validation.

**Status**: ✅ **Production Ready**

**Cross-Platform**: Works on Windows, macOS, and Linux with Python 3.7+

## Quick Start

### Requirements
- Python 3.7+
- Arduino Core STM32 (for PeripheralPins.c validation)

### Usage

```bash
# Auto-generate output filename from config (JHEF-JHEF411.config → output/JHEF-JHEF411.h)
python3 convert.py data/JHEF-JHEF411.config

# Or specify custom output filename
python3 convert.py data/JHEF-JHEF411.config output/CUSTOM_NAME.h
```

### Example Output
```
Loading Betaflight config: data/JHEF-JHEF411.config
  Board: JHEF411
  Manufacturer: JHEF
  MCU: STM32F411
Loading PeripheralPins.c: Arduino_Core_STM32/variants/STM32F4xx/F411C(C-E)(U-Y)/PeripheralPins.c

Validating configuration...
✅ Validation passed
Validation Summary:
  Errors: 0
  Warnings: 0

Generating BoardConfig: output/JHEF-JHEF411.h
✅ Successfully generated: output/JHEF-JHEF411.h
```

### Testing

```bash
# Install pytest (one-time)
pip install pytest

# Run tests
pytest -v
# Expected: 53 tests passing
```

## Implementation Status

**Completed**: ✅ Full converter with validation
- Betaflight config parser (18 tests)
- PeripheralPins.c parser (15 tests)
- Configuration validator (8 tests)
- C++ code generator (12 tests)
- **Total: 53 tests, 100% passing**

---

## Architecture

### Parser → Validator → Generator Pipeline

1. **BetaflightConfig Parser** (`src/betaflight_config.py`)
   - Parses `.config` files (resources, timers, settings)
   - Extracts motors, SPI buses, I2C, UARTs, ADC
   - Converts pin formats (B04 → PB_4)

2. **PeripheralPinMap Parser** (`src/peripheral_pins.py`)
   - Parses Arduino Core STM32 `PeripheralPins.c` files
   - Provides authoritative pin→peripheral mappings
   - Validates timer/AF, SPI, I2C, UART assignments

3. **ConfigValidator** (`src/validator.py`)
   - Cross-validates Betaflight config against PeripheralPins.c
   - Detects pin conflicts and invalid assignments
   - Groups motors by timer banks

4. **BoardConfigGenerator** (`src/code_generator.py`)
   - Generates C++ BoardConfig headers
   - Creates Storage, IMU, I2C, UART, ADC, Motor namespaces
   - Outputs compile-time configuration

---

## Pin Format Architecture

### Two Pin Systems in STM32 Arduino Core

The STM32 Arduino Core uses two different pin identification systems:

1. **PinName Enums** (`PA_0`, `PB_15`, `PC_12`) - HAL/Low-level format
   - Defined in `cores/arduino/stm32/PinNames.h`
   - Used internally by STM32 HAL drivers
   - Enum values like `PA_0 = 0x00`, `PB_15 = 0x1F`

2. **Arduino Pin Macros** (`PA0`, `PB15`, `PC12`) - Arduino API format
   - Defined in variant headers (e.g., `variant_NUCLEO_F411RE.h`)
   - Integer constants: `#define PA0  0`, `#define PC12  17`
   - Used by all Arduino APIs and library constructors

### Why Arduino Macros Are Used

**All STM32 Arduino libraries expect `uint32_t` pin numbers (Arduino macros), NOT PinName enums:**

```cpp
// Library constructors accept uint32_t
SPIClass::SPIClass(uint32_t mosi, uint32_t miso, uint32_t sclk);
HardwareSerial::HardwareSerial(uint32_t rx, uint32_t tx);

// Internally, libraries convert to PinName enums using digitalPinToPinName()
SPIClass::SPIClass(uint32_t mosi, ...) {
  _spi.pin_mosi = digitalPinToPinName(mosi);  // uint32_t → PinName enum
}
```

**The `digitalPinToPinName()` function:**
- **Expects:** Arduino pin number (0 to NUM_DIGITAL_PINS-1)
- **Returns:** PinName enum value
- **Implementation:** Looks up `digitalPin[index]` array to get the PinName

### Correct Usage in Generated Configs

**This converter generates Arduino macros (no underscore):**

```cpp
// ✅ CORRECT - Arduino macro format (this converter)
StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB2, 8000000};
//                                              ^^^^  ^^^^  ^^^^  ^^^
//                                              No underscore - Arduino macros

// ❌ WRONG - PinName enum format (would fail)
StorageConfig storage{StorageBackend::LITTLEFS, PB_15, PB_14, PB_13, PB_2, 8000000};
//                                              ^^^^^  ^^^^^  ^^^^^  ^^^^
//                                              Underscore - HAL enums (incorrect)
```

**Why PinName enums would fail:**
1. `PB_15` enum has value like `0x1F` (31 in decimal)
2. Passed to `SPIClass(uint32_t mosi, ...)` → implicitly cast to `31`
3. `digitalPinToPinName(31)` looks up `digitalPin[31]`
4. If `digitalPin[31] != PB_15`, wrong pin or crash

**Why Arduino macros work:**
1. `PB15` macro expands to correct Arduino pin number (e.g., `42`)
2. Passed to `SPIClass(uint32_t mosi, ...)` → receives `42`
3. `digitalPinToPinName(42)` looks up `digitalPin[42]` → returns `PB_15` enum
4. Correct conversion path, type-safe, guaranteed by variant

### Rationale for Arduino Macro Format

1. ✅ **Designed for this purpose** - Arduino macros are meant to be passed to library constructors
2. ✅ **All existing code uses this** - Every working Arduino example uses `PA7`, not `PA_7`
3. ✅ **Correct conversion path** - Macro → integer → `digitalPinToPinName()` → enum
4. ✅ **Type-safe** - Macros are defined in variant headers, guaranteed to be valid
5. ✅ **Matches manual configs** - All hand-written configs use this format

### ALT Pin Variants for Peripheral Selection

Some pins support multiple peripheral instances (e.g., `PB5` can be SPI1 or SPI3):

```cpp
// PeripheralPins.c shows:
{PB_5,      SPI1, ...},  // Default (first match)
{PB_5_ALT1, SPI3, ...},  // Alternate

// Variant header defines:
#define PB5      15          // Arduino pin number
#define PB5_ALT1 (PB5 | ALT1)  // 15 | 0x100 = 0x115
```

**How Arduino Core selects peripherals:**
1. User passes pin to library: `SPIClass spi(PB5, ...);`
2. `digitalPinToPinName(15)` returns `PB_5` enum
3. `pinmap_pinout()` searches PeripheralPins.c for `PB_5`
4. Finds first match: `{PB_5, SPI1, ...}` → **SPI1 configured**

**To select alternate peripheral:**
1. User passes ALT variant: `SPIClass spi(PB5_ALT1, ...);`
2. `digitalPinToPinName(0x115)` returns `PB_5_ALT1` enum
3. `pinmap_pinout()` searches for `PB_5_ALT1`
4. Finds: `{PB_5_ALT1, SPI3, ...}` → **SPI3 configured**

### Pin Format Selection in This Converter

**The converter must generate the correct pin format (base or ALT) to match the peripheral bus specified in the Betaflight config:**

```cpp
// Betaflight says: gyro_1_spibus = 3, pins PA7/PA6/PA5
// PeripheralPins.c shows: PA_7 → SPI1 (first), PA_7_ALT1 → SPI3
// Converter must generate: PA7_ALT1 (to get SPI3, not SPI1)
```

**Implementation:**
1. Parser reads Betaflight `set gyro_1_spibus = 3` → bus_num = 3
2. Validator checks PeripheralPins.c to find which pin format (base or ALT) maps to SPI3
3. Generator outputs correct format: `PA7` (if SPI1 needed) or `PA7_ALT1` (if SPI3 needed)

**Validation ensures:**
- Pins can support the requested peripheral
- Pin format matches the bus number from Betaflight config
- All pins on the same bus use compatible formats

---

## Development Documentation

See [dev-docs/](dev-docs/) for detailed design documentation:

### Design Documents
- **RESEARCH.md** - Betaflight unified target format reference
- **CONVERTER_ANALYSIS.md** - Implementation design and architecture
- **MOTOR_CONFIG_DESIGN.md** - Motor→TimerPWM integration
- **KNOWLEDGE_CHECKLIST.md** - Gap analysis and validation strategy
- **IMPLEMENTATION_SUMMARY.md** - Project completion summary
- **COMPARISON.md** - Generated vs manual config validation
- **REFACTORING_OPPORTUNITIES.md** - Code improvement analysis

---

## Example: JHEF-JHEF411 (NOXE V3)

**Input:** `data/JHEF-JHEF411.config` (Betaflight unified target)

**Output:** `output/JHEF-JHEF411.h` - BoardConfig header with:
- Storage: SPI flash (SPI2) → `StorageConfig`
- IMU: ICM42688P/MPU6000 (SPI1) → `IMUConfig`
- I2C: Environmental sensors (I2C1) → `I2CConfig`
- UARTs: 2 serial ports → `UARTConfig`
- ADC: Battery monitoring → `ADCConfig`
- LEDs: Status LEDs → `LEDConfig`
- Servos: PWM servo outputs (50 Hz) → `Servo` namespace (when present)
- Motors: 5 motors on 2 timer banks → `Motor` namespace

**Validation:** All pins cross-validated against `PeripheralPins.c`

See `output/JHEF-JHEF411.h` for complete example.

---

## Usage Examples

### Basic Configuration Access

See `examples/basic_config_usage/` - Demonstrates how to read and use generated BoardConfig:
- Storage (SPI flash/SD card) configuration
- IMU sensor setup (SPI + interrupt)
- UART port configuration
- Battery monitoring (ADC)
- Status LED initialization
- Motor/servo frequency and pin information

**Key Pattern**:
```cpp
#include "../../output/JHEF-JHEF411.h"

void setup() {
  // Access storage config
  auto cs_pin = BoardConfig::storage.cs_pin;
  auto freq = BoardConfig::storage.freq_hz;

  // Access IMU config
  auto imu_cs = BoardConfig::imu.spi.cs_pin;
  auto imu_int = BoardConfig::imu.int_pin;

  // Initialize status LED
  pinMode(BoardConfig::status_leds.led1_pin, OUTPUT);
}
```

### Motor and Servo PWM Control

See `examples/pwm_motor_servo/` - Demonstrates PWM output using TimerPWM library:
- Servo control (50 Hz, 1000-2000 µs)
- Motor control (1000 Hz, OneShot125)
- Timer bank configuration
- Channel attachment and pulse width control

**Key Pattern**:
```cpp
#include <PWMOutputBank.h>
#include "../../output/MTKS-MATEKH743.h"

PWMOutputBank servo_pwm;

void setup() {
  // Initialize servo timer bank
  servo_pwm.Init(BoardConfig::Servo::TIM15_Bank::timer,
                 BoardConfig::Servo::frequency_hz);

  // Attach servo channels
  auto& servo1 = BoardConfig::Servo::TIM15_Bank::servo1;
  servo_pwm.AttachChannel(servo1.ch, servo1.pin,
                          servo1.min_us, servo1.max_us);
  servo_pwm.Start();
}

void loop() {
  servo_pwm.SetPulseWidth(servo1.ch, 1500); // Center position
}
```
