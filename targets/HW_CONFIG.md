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
├── config/
│   └── ConfigTypes.h            # Configuration structures (SPI, UART, Storage, etc.)
├── NUCLEO_F411RE.h              # Base development board configuration
├── NUCLEO_F411RE_LITTLEFS.h     # Development board with LittleFS storage
├── NUCLEO_F411RE_SDFS.h         # Development board with SDFS storage
├── BLACKPILL_F411CE.h           # BlackPill F411CE board configuration
├── NOXE_V3.h                    # Production flight controller configuration
└── PIN_USE.md                   # Pin usage documentation
```

### Configuration Framework

**Configuration Types** (`targets/config/ConfigTypes.h`):
```cpp
#pragma once

// Storage backend types
enum class StorageBackend {
    NONE,      // No storage hardware attached
    LITTLEFS,  // SPI flash storage
    SDFS       // SD card storage
};

namespace BoardConfig {
  struct SPIConfig {
    constexpr SPIConfig(uint32_t mosi, uint32_t miso, uint32_t sclk, uint32_t cs,
                       uint32_t setup_hz, uint32_t runtime_hz)
      : mosi_pin(mosi), miso_pin(miso), sclk_pin(sclk), cs_pin(cs),
        setup_clock_hz(setup_hz), runtime_clock_hz(runtime_hz) {}

    const uint32_t mosi_pin, miso_pin, sclk_pin, cs_pin;
    const uint32_t setup_clock_hz, runtime_clock_hz;
  };

  struct UARTConfig {
    constexpr UARTConfig(uint32_t tx, uint32_t rx, uint32_t baud)
      : tx_pin(tx), rx_pin(rx), baud_rate(baud) {}

    const uint32_t tx_pin, rx_pin;
    const uint32_t baud_rate;
  };

  struct I2CConfig {
    constexpr I2CConfig(uint32_t sda, uint32_t scl, uint32_t freq)
      : sda_pin(sda), scl_pin(scl), frequency_hz(freq) {}

    const uint32_t sda_pin, scl_pin;
    const uint32_t frequency_hz;
  };

  struct StorageConfig {
    constexpr StorageConfig(StorageBackend backend, uint32_t mosi, uint32_t miso,
                           uint32_t sclk, uint32_t cs, uint32_t setup_hz, uint32_t runtime_hz)
      : backend_type(backend), mosi_pin(mosi), miso_pin(miso), sclk_pin(sclk),
        cs_pin(cs), setup_clock_hz(setup_hz), runtime_clock_hz(runtime_hz) {}

    const StorageBackend backend_type;
    const uint32_t mosi_pin, miso_pin, sclk_pin, cs_pin;
    const uint32_t setup_clock_hz, runtime_clock_hz;
  };
}
```

## Board Configurations

### Development Board (NUCLEO_F411RE)

**Base Configuration** (`targets/NUCLEO_F411RE.h`):
```cpp
#pragma once
#include "config/ConfigTypes.h"

// NUCLEO F411RE development board configuration
// Uses Arduino pin macros for compatibility with existing code
namespace BoardConfig {
  // Storage: No storage hardware attached by default on base Nucleo
  // Use NUCLEO_F411RE_LITTLEFS.h or NUCLEO_F411RE_SDFS.h for storage testing
  static constexpr StorageConfig storage{StorageBackend::NONE, 0, 0, 0, 0, 0, 0};

  // IMU: SPI connections via jumpers (reduced speed for reliability)
  static constexpr SPIConfig imu{PA7, PA6, PA5, PA4, 1000000, 2000000};

  // GPS: UART communication
  static constexpr UARTConfig gps{PA9, PA10, 115200};

  // I2C: Magnetometer, barometer
  static constexpr I2CConfig sensors{PB9, PB8, 400000};
}
```

**With LittleFS Storage** (`targets/NUCLEO_F411RE_LITTLEFS.h`):
```cpp
#pragma once
#include "config/ConfigTypes.h"

// NUCLEO F411RE with LittleFS SPI flash storage for testing
namespace BoardConfig {
  // Storage: SPI flash via breakout board
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PA7, PA6, PA5, PA4, 1000000, 8000000};

  // IMU: Alternative SPI pins (when storage uses primary SPI)
  static constexpr SPIConfig imu{PC12, PC11, PC10, PD2, 1000000, 2000000};

  // GPS: UART communication
  static constexpr UARTConfig gps{PA9, PA10, 115200};

  // I2C: Magnetometer, barometer
  static constexpr I2CConfig sensors{PB9, PB8, 400000};
}
```

### Alternative Development Board (BLACKPILL_F411CE)

**Configuration** (`targets/BLACKPILL_F411CE.h`):
```cpp
#pragma once
#include "config/ConfigTypes.h"

// BlackPill F411CE development board configuration
// Compact development board with USB-C connector
namespace BoardConfig {
  // Storage: SPI flash or SD card via breakout board
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PA7, PA6, PA5, PA4, 1000000, 8000000};

  // IMU: SPI connections (optimized speeds for hardwired connections)
  static constexpr SPIConfig imu{PB15, PB14, PB13, PB12, 1000000, 8000000};

  // GPS: UART communication
  static constexpr UARTConfig gps{PA9, PA10, 115200};

  // I2C: Magnetometer, barometer
  static constexpr I2CConfig sensors{PB8, PB9, 400000};
}
```

### Production Board (NOXE_V3)

**Configuration** (`targets/NOXE_V3.h`):
```cpp
#pragma once
#include "config/ConfigTypes.h"

// NOXE V3 flight controller production configuration
namespace BoardConfig {
  // Storage: High-speed SD card for data logging
  static constexpr StorageConfig storage{StorageBackend::SDFS, PB15, PB14, PB13, PB12, 1000000, 8000000};

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

**Board Configuration Tests** (`tests/BoardConfig_Test/BoardConfig_Test.ino`):
```cpp
#include <Arduino.h>
#include "../../../../ci_log.h"

// Include target-specific configuration
#if defined(ARDUINO_BLACKPILL_F411CE)
#include "../../targets/BLACKPILL_F411CE.h"
#else
#include "../../targets/NUCLEO_F411RE.h"
#endif

void setup() {
  CI_LOG("Board Configuration Test\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // Test configuration accessibility
  CI_LOG("IMU SPI Config: MOSI=%lu, MISO=%lu, SCLK=%lu, CS=%lu\n",
         BoardConfig::imu.mosi_pin, BoardConfig::imu.miso_pin,
         BoardConfig::imu.sclk_pin, BoardConfig::imu.cs_pin);

  CI_LOG("Storage Config: Backend=%d, CS=%lu\n",
         (int)BoardConfig::storage.backend_type, BoardConfig::storage.cs_pin);

  CI_LOG("GPS UART Config: TX=%lu, RX=%lu, Baud=%lu\n",
         BoardConfig::gps.tx_pin, BoardConfig::gps.rx_pin, BoardConfig::gps.baud_rate);

  CI_LOG("Board configuration test completed successfully\n");
  CI_LOG("*STOP*\n");
}
```

**Storage Tests** (`tests/Generic_Storage_LittleFS_Unit_Tests/Generic_Storage_LittleFS_Unit_Tests.ino`):
```cpp
#include <Arduino.h>
#include <Storage.h>
#include <BoardStorage.h>
#include "../../../../ci_log.h"
#include "../../targets/NUCLEO_F411RE_LITTLEFS.h"

void setup() {
  CI_LOG("Generic Storage LittleFS Unit Tests\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // BoardStorage uses configuration from included target header
  if (BoardStorage::begin(BoardConfig::storage)) {
    CI_LOG("LittleFS storage initialized successfully\n");

    // Generic storage operations via abstraction
    Storage& fs = BOARD_STORAGE;
    File testFile = fs.open("/test.txt", FILE_WRITE);
    if (testFile) {
      testFile.println("Hardware configuration test data");
      testFile.close();
      CI_LOG("Test file created via generic interface\n");
    }
  } else {
    CI_LOG("Storage initialization failed\n");
  }

  CI_LOG("*STOP*\n");
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

**Hardware Configuration Tests**:
```bash
# Test board configuration on NUCLEO_F411RE
./scripts/aflash.sh tests/BoardConfig_Test --use-rtt --build-id

# Test storage configurations
./scripts/aflash.sh tests/Generic_Storage_LittleFS_Unit_Tests --use-rtt --build-id
./scripts/aflash.sh tests/Generic_Storage_SDFS_Unit_Tests --use-rtt --build-id
```

**Multi-Board Testing**:
```bash
# NUCLEO_F411RE (automatic configuration selection)
arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE tests/BoardConfig_Test/

# BlackPill F411CE (automatic configuration selection)
arduino-cli compile --fqbn STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE tests/BoardConfig_Test/
```

**Production Builds**:
```bash
# NOXE_V3 flight controller (when variant is available)
arduino-cli compile --fqbn STMicroelectronics:stm32:NOXE_V3 sketches/NOXE_V3_FlightController/
```

### Board Selection

The Arduino build system automatically sets the correct `ARDUINO_*` define based on the FQBN:
- `ARDUINO_NUCLEO_F411RE` → `targets/NUCLEO_F411RE.h`
- `ARDUINO_BLACKPILL_F411CE` → `targets/BLACKPILL_F411CE.h`
- `ARDUINO_NOXE_V3` → `targets/NOXE_V3.h`

Tests can override default configuration by explicitly including specific target headers (e.g., `NUCLEO_F411RE_LITTLEFS.h`).

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