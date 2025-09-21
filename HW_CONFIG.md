# Hardware Configuration System

This document describes the hardware configuration system for STM32-based embedded applications, providing a structured approach to define target board hardware at compile time while maintaining clean separation between Arduino core and application-level configuration.

## Overview

The configuration system addresses the need to support multiple embedded board types (2-5 boards) with different hardware configurations for peripherals like storage, sensors, and communication interfaces, while avoiding circular dependencies and minimizing changes to the Arduino_Core_STM32 submodule. The system is designed for general embedded applications including flight controllers, ground rovers, AUVs, and robotic systems that require structured hardware configuration.

## Architecture Principles

### Clear Separation of Responsibilities

**Arduino Core Variant Responsibilities:**
- **Pin Definitions**: Physical pin mappings (`PA0`, `PC12`, etc.)
- **Peripheral Mappings**: `PeripheralPins.c` - which pins can use which peripherals
- **RCC Clock Configuration**: System clock setup, PLL configuration, peripheral clocks
- **Board Identification**: `ARDUINO_*` defines for board detection
- **Hardware Abstraction**: Basic STM32 HAL/LL integration

**External Configuration Responsibilities:**
- **Pin Usage Patterns**: Which pins are assigned to which functions (storage, sensors, communication)
- **Peripheral Configuration**: SPI speeds, UART baud rates, timing parameters
- **Application Wiring**: How hardware components connect to the microcontroller
- **Board-Specific Behaviors**: Storage types, sensor configurations, communication protocols

### Dependency Flow (Clean Unidirectional)

```
Core Variant (pins, clocks, peripherals)
    ↓
Libraries (hardware abstraction, no config knowledge)
    ↓
External Configuration (pin assignments, speeds, protocols)
    ↓
Application (uses libraries + config to implement functionality)
```

This eliminates circular dependencies by keeping configuration external to the core while allowing libraries to remain hardware-agnostic.

## Implementation Structure

### Project-Level Configuration

```
/home/geo/Arduino/targets/
├── BoardConfig.h                 # Core configuration framework
├── ConfigTypes.h                 # Configuration structures (SPI, UART, etc.)
├── boards/
│   ├── NUCLEO_F411RE.h          # Development board configuration
│   └── NOXE_V3.h                # Production flight controller configuration
└── README.md                    # Configuration documentation
```

### Configuration Framework

**Core Framework** (`targets/BoardConfig.h`):
```cpp
#pragma once
#include "ConfigTypes.h"

// Board selection based on Arduino defines (from core)
#if defined(ARDUINO_NUCLEO_F411RE)
  #include "boards/NUCLEO_F411RE.h"
#elif defined(ARDUINO_NOXE_V3)
  #include "boards/NOXE_V3.h"
#else
  #error "Board configuration not available for this board"
#endif
```

**Configuration Types** (`targets/ConfigTypes.h`):
```cpp
#pragma once

namespace BoardConfig {
  struct SPIConfig {
    constexpr SPIConfig(uint8_t mosi, uint8_t miso, uint8_t sclk, uint8_t cs,
                       uint32_t setup_hz, uint32_t runtime_hz)
      : mosi_pin(mosi), miso_pin(miso), sclk_pin(sclk), cs_pin(cs),
        setup_clock_hz(setup_hz), runtime_clock_hz(runtime_hz) {}

    const uint8_t mosi_pin, miso_pin, sclk_pin, cs_pin;
    const uint32_t setup_clock_hz, runtime_clock_hz;
  };

  struct UARTConfig {
    constexpr UARTConfig(uint8_t tx, uint8_t rx, uint32_t baud)
      : tx_pin(tx), rx_pin(rx), baud_rate(baud) {}

    const uint8_t tx_pin, rx_pin;
    const uint32_t baud_rate;
  };

  struct I2CConfig {
    constexpr I2CConfig(uint8_t sda, uint8_t scl, uint32_t freq)
      : sda_pin(sda), scl_pin(scl), frequency_hz(freq) {}

    const uint8_t sda_pin, scl_pin;
    const uint32_t frequency_hz;
  };
}
```

## Board Configurations

### Development Board (NUCLEO_F411RE)

**Configuration** (`targets/boards/NUCLEO_F411RE.h`):
```cpp
#pragma once
#include "../ConfigTypes.h"

// NUCLEO F411RE development board configuration
// Uses standard Arduino pin names from core variant
namespace BoardConfig {
  // Storage: SD card via SPI (development/testing)
  static constexpr SPIConfig storage{PC12, PC11, PC10, PD2, 1000000, 8000000};

  // IMU: Accelerometer/Gyroscope via SPI
  static constexpr SPIConfig imu{PA7, PA6, PA5, PA4, 1000000, 8000000};

  // GPS: UART communication
  static constexpr UARTConfig gps{PA9, PA10, 115200};

  // I2C: Magnetometer, barometer
  static constexpr I2CConfig sensors{PB9, PB8, 400000};
}
```

### Production Board (NOXE_V3)

**Configuration** (`targets/boards/NOXE_V3.h`):
```cpp
#pragma once
#include "../ConfigTypes.h"

// NOXE V3 flight controller production configuration
namespace BoardConfig {
  // Storage: High-speed SD card for data logging
  static constexpr SPIConfig storage{PB15, PB14, PB13, PB12, 1000000, 8000000};

  // IMU: High-performance IMU for flight control
  static constexpr SPIConfig imu{PA7, PA6, PA5, PA4, 1000000, 8000000};

  // GPS: High-precision GPS module
  static constexpr UARTConfig gps{PC6, PC7, 115200};

  // I2C: Environmental sensors
  static constexpr I2CConfig sensors{PB6, PB7, 400000};
}
```

## Arduino Core Variant Requirements

### Minimal Core Changes

**For NOXE_V3** - only standard variant definition:
```cpp
// Arduino_Core_STM32/variants/STM32F4xx/NOXE_V3/variant_NOXE_V3.h
#pragma once

// Standard pin definitions only - NO configuration
#define PA0  PIN_A0
#define PA1  PIN_A1
#define PA2  PIN_A2
// ... pin definitions for NOXE_V3 hardware

// Clock configuration and peripheral mappings
#define HSE_VALUE    8000000U  // External crystal
#define HSI_VALUE    16000000U // Internal oscillator

// Board identification (triggers config selection)
#define ARDUINO_NOXE_V3

// ... standard variant definitions
```

**Clock Configuration** (`Arduino_Core_STM32/variants/STM32F4xx/NOXE_V3/variant_NOXE_V3.cpp`):
```cpp
void SystemClock_Config(void) {
  // RCC configuration - system clocks, PLL, peripheral clocks
  // This is core responsibility, not application configuration
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  // ... core clock setup
}
```

## Usage Examples

### Hardware Unit Tests

**Storage Tests** (`tests/Storage_Hardware_Tests/Storage_Hardware_Tests.ino`):
```cpp
#include <Arduino.h>                     // Core pins available
#include "../../targets/BoardConfig.h"   // Board configuration
#include <Storage.h>                     // Generic storage interface

void setup() {
  CI_LOG("Storage Hardware Test\n");

  // BoardConfig handles all storage setup internally
  // - Storage type selection (SDFS vs LittleFS)
  // - Pin configuration and SPI setup
  // - Hardware-specific initialization
  if (BoardConfig::storage.begin()) {
    CI_LOG("Storage initialized successfully\n");

    // Generic storage operations
    File testFile = BoardConfig::storage.open("/test.txt", FILE_WRITE);
    if (testFile) {
      testFile.println("Test data");
      testFile.close();
      CI_LOG("Test file created\n");
    }
  } else {
    CI_LOG("Storage initialization failed\n");
  }
}
```

### Production Application

**Flight Controller Application** (`sketches/NOXE_V3_FlightController/NOXE_V3_FlightController.ino`):
```cpp
#include <Arduino.h>
#include "../../targets/BoardConfig.h"
#include <Storage.h>

void setup() {
  // All peripheral initialization handled by BoardConfig
  if (BoardConfig::begin()) {
    // GPS, IMU, sensors, and storage automatically configured

    // Application-level usage with clean abstractions
    File flightLog = BoardConfig::storage.open("/flight.log", FILE_WRITE);
    if (flightLog) {
      flightLog.println("Flight controller started");
      flightLog.close();
    }

    // Access to configured peripherals
    BoardConfig::gps.begin();       // GPS ready with correct pins/baud
    BoardConfig::imu.begin();       // IMU ready with correct SPI config
    BoardConfig::sensors.begin();   // I2C sensors ready

    Serial.println("All systems initialized");
  } else {
    Serial.println("System initialization failed");
  }
}

void loop() {
  // Application logic using abstracted interfaces
  // No knowledge of pin assignments or hardware specifics
}
```

## CI/CD Integration

### Build System Compatibility

**Hardware Tests on Development Board**:
```bash
# Automatically uses NUCLEO_F411RE configuration
./scripts/aflash.sh tests/Storage_Hardware_Tests --use-rtt --build-id
```

**Production Builds**:
```bash
# Automatically uses NOXE_V3 configuration
arduino-cli compile --fqbn STMicroelectronics:stm32:NOXE_V3 sketches/NOXE_V3_FlightController/
```

### Board Selection

The Arduino build system automatically sets the correct `ARDUINO_*` define based on the FQBN, which triggers the appropriate configuration inclusion.

## Benefits

### Technical Benefits

1. **Zero Circular Dependencies**: Clear unidirectional dependency flow
2. **Minimal Core Impact**: Configuration completely external to Arduino_Core_STM32
3. **Type Safety**: Compile-time configuration with zero runtime overhead
4. **Clean Separation**: Hardware definitions (core) vs. usage configuration (external)
5. **Library Independence**: Libraries have no configuration dependencies

### Development Benefits

1. **Automatic Configuration**: FQBN selection automatically includes correct config
2. **Single Codebase**: Same test code works on development and production hardware
3. **Scalable**: Easy to add new boards without breaking existing code
4. **CI/CD Compatible**: Existing workflow unchanged
5. **Maintainable**: Clear organization and extensible structure

### Upstream Benefits

1. **Clean Submodule**: Arduino_Core_STM32 changes minimal and upstream-friendly
2. **Generic Variants**: New variants follow standard Arduino patterns
3. **No Framework Changes**: Leverages existing Arduino architecture

## Future Expansion

### Adding New Boards

Examples of additional boards that could be supported:
- **Rover Controller**: Ground-based robotic vehicle control
- **AUV Controller**: Autonomous underwater vehicle systems
- **Robot Controller**: General robotic platform control

Adding a new board requires:
1. **Create variant** in `Arduino_Core_STM32/variants/STM32F4xx/NEW_BOARD/`
2. **Add configuration** in `targets/boards/NEW_BOARD.h`
3. **Update selector** in `targets/BoardConfig.h`
4. **Add boards.txt entry** for Arduino CLI recognition

### Additional Peripheral Types

New peripheral types can be added to `ConfigTypes.h`:
- `CANConfig` for CAN bus communication
- `PWMConfig` for motor control
- `ADCConfig` for analog sensors
- `DAC Config` for analog output

The framework is designed to be easily extensible for additional hardware configurations as embedded application requirements evolve.