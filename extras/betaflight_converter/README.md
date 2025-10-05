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
# Linux/macOS
python3 convert.py data/JHEF-JHEF411.config output/NOXE_V3.h

# Windows
python convert.py data\JHEF-JHEF411.config output\NOXE_V3.h
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

Generating BoardConfig: output/NOXE_V3.h
✅ Successfully generated: output/NOXE_V3.h
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

## Example: JHEF411 (NOXE V3)

**Input:** `data/JHEF-JHEF411.config` (Betaflight unified target)

**Output:** BoardConfig header with:
- Storage: SPI flash (SPI2) → `StorageConfig`
- IMU: ICM42688P/MPU6000 (SPI1) → `IMUConfig`
- I2C: Environmental sensors (I2C1) → `I2CConfig`
- UARTs: 2 serial ports → `UARTConfig`
- ADC: Battery monitoring → `ADCConfig`
- Motors: 5 motors on 2 timer banks → `Motor` namespace

**Validation:** All pins cross-validated against `PeripheralPins.c`

See `output/NOXE_V3_generated.h` for complete example.
