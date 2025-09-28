# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This repository contains the **STM32 Arduino Core** along with associated sketches and libraries. It provides Arduino IDE support for STM32 microcontrollers through the STM32Cube ecosystem.

The `Arduino_Core_STM32/` directory is a fork of the upstream [stm32duino/Arduino_Core_STM32](https://github.com/stm32duino/Arduino_Core_STM32) repository (https://github.com/geosmall/Arduino_Core_STM32). This fork includes:
- **Simplified variant selection** - Reduced number of board variants for focused development
- **Added FS.h** - Generic embedded file system base class for storage access (unified interface for SPI flash and SD card storage)
- **Symlinked integration** - Changes made here are automatically reflected in the sketch build process

### Repository Structure

- `Arduino_Core_STM32/` - Main STM32 Arduino core implementation
  - `cores/arduino/` - Core Arduino implementation for STM32
  - `variants/` - Board-specific pin definitions and configurations 
  - `libraries/` - Core STM32 libraries (SPI, Wire, SoftwareSerial, etc.)
  - `system/` - STM32Cube HAL drivers and CMSIS
- `cmake/` - CMake build examples and configuration
- `libraries/` - Additional STM32-specific libraries (LittleFS, SDFS, STM32RTC, ICM42688P, libPrintf, minIniStorage, Storage, AUnit, etc.)
- `HIL_RTT_Test/` - HIL test framework with comprehensive validation and RTT debugging

## Build Systems and Commands

### Arduino CLI (Recommended)
The primary build system uses arduino-cli with STM32 boards:

```bash
# Install STM32 core
arduino-cli core update-index
arduino-cli core install STMicroelectronics:stm32

# Compile sketch
arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE <sketch_directory>

# Upload to board
arduino-cli upload --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE <sketch_directory>

# List available/connected boards
arduino-cli board listall
arduino-cli board list

# J-Link upload (when ST-Link reflashed to J-Link)
./scripts/jlink_upload.sh <path_to_binary.bin>
```

### Makefile Support
Generic Makefile provided for cross-platform builds:

```bash
make                    # Compile with default FQBN
make upload            # Compile and upload to board
make clean             # Clean build artifacts
```

Default FQBN: `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE`

### Build Scripts
Enhanced build workflow with environment validation and device auto-detection:

```bash
# Standard build and HIL testing
./scripts/build.sh <sketch_directory> [--build-id] [--env-check] [--use-rtt]
./scripts/aflash.sh <sketch_directory> [--env-check] [--use-rtt] [--build-id]

# Environment and device utilities
./scripts/env_check_quick.sh         # Fast environment validation
./scripts/detect_device.sh           # Auto-detect STM32 via J-Link
./scripts/flash_auto.sh <binary>     # Program with auto-detected device
./scripts/cleanup_repo.sh            # Clean build artifacts before commit
```

**Key Features**:
- **Environment Validation**: Arduino CLI (1.3.0) and STM32 Core (2.7.1) validation
- **Build Traceability**: Git SHA + UTC timestamp integration
- **Device Auto-Detection**: 50+ STM32 device IDs supported
- **Cache Management**: `--clean-cache` for deterministic builds

### J-Link and RTT Utilities

```bash
# J-Link utilities
./scripts/jrun.sh <elf> [timeout] [exit_wildcard] # J-Run execution with RTT
./scripts/flash.sh [--quick] <binary>             # Flash with fixed device

# Manual RTT Connection
JLinkGDBServer -Device STM32F411RE -If SWD -Speed 4000 -RTTTelnetPort 19021 &
JLinkRTTClient                                     # Connect to RTT
```

### CMake Build System

```bash
# Prerequisites: CMake >=3.21, Python3 >=3.9
pip install cmake graphviz jinja2

# Setup and build
cmake/scripts/cmake_easy_setup.py -b <board> -s <sketch_folder>
cmake -S <sketch_folder> -B <build_folder> -G Ninja
cmake --build <build_folder>
```

## Architecture Overview

### Core Components

**Arduino Core** (`cores/arduino/`):
- `main.cpp` - Standard Arduino main loop with STM32 initialization
- `Arduino.h` - Main Arduino header with STM32-specific extensions
- HAL integration through `stm32/` subdirectory
- Hardware abstraction for STM32Cube HAL and LL APIs

**Board Variants** (`variants/`):
- Organized by STM32 family (F4xx, F7xx, G4xx, H7xx, etc.)
- Each variant contains:
  - `PeripheralPins.c` - Pin mapping to STM32 peripherals
  - `variant_*.h/.cpp` - Board-specific pin definitions
  - `boards_entry.txt` - Board configuration for boards.txt

**Build Configuration**:
- `boards.txt` - Arduino IDE board definitions and menus
- `platform.txt` - Toolchain and compiler settings
- Board selection uses FQBN format: `STMicroelectronics:stm32:<board_group>:pnum=<specific_board>`

## Target Hardware and Applications

### Primary Development Boards
- **Nucleo F411RE** (Primary): `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE`
  - **HIL Setup**: On-board ST-Link V2.1 reflashed to J-Link firmware
  - **Serial Monitor**: Available via J-Link connection (connected CDC ACM serial monitor)
  - **Programming**: J-Run execution via reflashed J-Link interface
- **BlackPill F411CE** (Secondary): `STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE`
- **Nucleo H753ZI** (High-Performance): `STMicroelectronics:stm32:Nucleo_144:pnum=NUCLEO_H753ZI`

### Target Hardware Platforms
This repository supports **UAV flight controller boards** with the following STM32 microcontrollers:

- **STM32F411** - Primary target (Nucleo F411RE, BlackPill F411CE for development)
- **STM32F405** - Secondary target (common in flight controllers) 
- **STM32H743** - Future target (high-performance flight controllers)

### Example Target Applications
- **UAV Flight Controllers** - Autonomous drone flight control systems
- **Embedded Storage Systems** - SPI flash (LittleFS) and SD card (SDFS) file systems
- **Real-time Data Logging** - Flight data, telemetry, and configuration storage
- **Sensor Data Management** - IMU, GPS, and other sensor data processing

### Storage Libraries
- **LittleFS** - SPI flash storage (configuration, firmware, small data files)
- **SDFS** - SD card filesystem via SPI with FatFs backend (data logging, bulk storage)
- **Generic Storage Abstraction** - Unified interface for seamless backend switching

### Key Libraries

**Core Libraries**:
- `Wire` - I2C communication
- `SPI` - SPI communication  
- `SoftwareSerial` - Software UART implementation
- `CMSIS_DSP` - ARM CMSIS DSP functions
- `SEGGER_RTT` - Segger RTT library

**STM32-Specific Libraries**:
- `STM32RTC` - Real-time clock functionality
- `LittleFS` - SPI flash filesystem with wear leveling (configuration, firmware)
- `SDFS` - SD card filesystem via SPI with FatFs backend (data logging, bulk storage)
- `Storage` - Generic storage abstraction providing unified interface for LittleFS and SDFS
- `minIniStorage` - INI configuration management with automatic storage backend selection
- `ICM42688P` - 6-axis IMU library with TDK InvenSense drivers and manufacturer self-test
- `libPrintf` - Optional embedded printf library (eyalroz/printf v6.2.0) - eliminates nanofp confusion, 20% binary reduction
- `AUnit` - Arduino unit testing framework (v1.7.1) - STM32-compatible testing with RTT integration

### Hardware Abstraction

The core integrates three levels of STM32 APIs:
1. **HAL (Hardware Abstraction Layer)** - High-level, feature-rich APIs
2. **LL (Low Layer)** - Optimized, register-level APIs
3. **CMSIS** - ARM Cortex standard interface

Board-specific configurations are defined through the variant system, allowing the same core to support hundreds of STM32 boards with different pin layouts and capabilities.

## Completed Projects

### Build Workflow ✅ **COMPLETED**

HIL testing framework with complete build-to-runtime traceability and device auto-detection.

**Key Features**:
- **Deterministic HIL Testing**: Exit wildcard methodology with "*STOP*" detection
- **Build Traceability**: Git commit SHA + UTC timestamp integration
- **Universal Device Support**: Auto-detection across 50+ STM32 device IDs
- **One-Command Workflow**: Complete build+test automation with environment validation

**Production Usage**:
```bash
./scripts/build.sh HIL_RTT_Test --build-id --env-check
./scripts/aflash.sh HIL_RTT_Test
```

### Unified Development Framework ✅ **COMPLETED**

Single-sketch development supporting both Arduino IDE and CI/HIL workflows via `ci_log.h` abstraction.

**Key Features**:
- **Dual-Mode Support**: Same sketch works with Serial (IDE) or RTT (HIL)
- **Build Integration**: `--use-rtt` and `--build-id` flag support
- **Single Codebase**: Eliminates duplicate test files
- **Clean Include Paths**: Arduino core integration for system-wide availability

**Integration Pattern**:
```cpp
#include <ci_log.h>
void setup() {
  CI_LOG("Test starting\n");
  CI_BUILD_INFO();    // RTT: shows build details, Serial: no-op
  CI_READY_TOKEN();   // RTT: shows ready token, Serial: no-op
}
```

### SDFS Implementation ✅ **COMPLETED**

SPI-based SD card filesystem with FatFs backend providing LittleFS-compatible API for seamless storage switching.

**Key Features**:
- **Complete File I/O**: All operations (create, read, write, delete, seek, truncate)
- **Directory Operations**: Full enumeration, creation, and traversal
- **Configuration System**: SDFSConfig.h with configurable limits
- **Runtime Detection**: Dynamic sector size and card capacity detection
- **Error Handling**: SDFSERR enum with mount protection and diagnostics

**Production Example**:
```cpp
#include <SDFS.h>
SDFS_SPI sdfs;

void setup() {
  if (sdfs.begin(CS_PIN)) {  // Auto-detects card capacity
    File file = sdfs.open("/flight.log", FILE_WRITE_BEGIN);
    file.printf("Card: %llu MB\n", sdfs.totalSize() / (1024*1024));
    file.close();
  }
}
```

### LittleFS Example Integration ✅ **COMPLETED**

Complete integration of all LittleFS examples with unified CI/HIL framework for SPI flash testing.

**Key Features**:
- **3 Examples**: ListFiles, LittleFS_ChipID, LittleFS_Usage
- **HIL Integration**: Full ci_log.h integration with deterministic testing
- **Hardware Support**: 20+ SPI flash chips from multiple manufacturers
- **Interactive Removal**: Eliminated waitforInput() calls for automation

### AUnit Testing Framework Integration ✅ **COMPLETED**

AUnit v1.7.1 unit testing framework integrated with HIL CI/CD workflow for comprehensive storage library testing.

**Key Features**:
- **Complete Integration**: `aunit_hil.h` wrapper with RTT/Serial abstraction
- **22 Total Tests**: LittleFS (8 tests), SDFS (7 tests), minIniStorage (6 tests), framework validation (1 test)
- **Hardware Validation**: Real SPI flash and SD card testing on STM32F411RE
- **Dual-Mode Support**: Same tests work with RTT (HIL) and Serial (IDE)
- **Type Safety**: Established AUnit assertion patterns for embedded types

**Production Usage**:
```bash
./scripts/aflash.sh tests/LittleFS_Unit_Tests --use-rtt --build-id
./scripts/aflash.sh tests/SDFS_Unit_Tests --use-rtt --build-id
./scripts/aflash.sh tests/minIniStorage_Unit_Tests --use-rtt --build-id
```

### Board Configuration System ✅ **COMPLETED**

Structured, compile-time board configuration system for multiple target platforms with automatic board detection.

**Key Features**:
- **Multi-Board Support**: NUCLEO_F411RE and BLACKPILL_F411CE configurations
- **Automatic Selection**: Via `ARDUINO_*` defines with preprocessor integration
- **SPI Speed Optimization**: 2MHz for jumper connections, 8MHz for hardwired
- **Storage Integration**: Complete LittleFS and SDFS unit test validation
- **Hardware Validation**: 15/15 storage tests passed on real hardware

**Production Usage**:
```cpp
#if defined(ARDUINO_BLACKPILL_F411CE)
#include "../../targets/BLACKPILL_F411CE.h"
#else
#include "../../targets/NUCLEO_F411RE.h"
#endif

SPIClass SPIbus(BoardConfig::storage.mosi_pin, BoardConfig::storage.miso_pin, BoardConfig::storage.sclk_pin);
```

### Generic Storage Abstraction ✅ **COMPLETED**

Unified storage interface abstracting SDFS and LittleFS with automatic backend selection.

**Key Features**:
- **Unified API**: Single interface for both storage backends
- **Board Integration**: Automatic backend selection via BoardConfig
- **Runtime Safety**: Factory pattern with comprehensive error handling

**Production Usage**:
```cpp
#include <Storage.h>
#include <BoardStorage.h>
#include "targets/NUCLEO_F411RE_LITTLEFS.h"

void setup() {
  if (BoardStorage::begin(BoardConfig::storage)) {
    Storage& fs = BOARD_STORAGE;
    File log = fs.open("/flight.log", FILE_WRITE);
    log.println("Unified storage working!");
    log.close();
  }
}
```

### minIni Configuration Management System ✅ **COMPLETED**

INI file configuration management integrated with Generic Storage Abstraction for both LittleFS and SDFS backends.

**Key Features**:
- **minIni v1.5**: Latest version with enhanced validation (hassection/haskey)
- **Storage Integration**: Works with both LittleFS and SDFS via unified interface
- **Data Types**: Strings, integers, floats, booleans with section/key enumeration
- **HIL Testing**: Full deterministic validation with 6 test suites

**Production Usage**:
```cpp
#include <minIniStorage.h>
#include "targets/NUCLEO_F411RE_LITTLEFS.h"

minIniStorage config("config.ini");

void setup() {
  if (config.begin(BoardConfig::storage)) {
    config.put("network", "ip_address", "192.168.1.100");
    config.put("system", "sample_rate", 1000);

    std::string ip = config.gets("network", "ip_address", "192.168.1.1");
    int rate = config.geti("system", "sample_rate", 100);
  }
}
```

### libPrintf Embedded Printf Library ✅ **COMPLETED**

Optional Arduino library wrapper for eyalroz/printf v6.2.0 that eliminates nanofp confusion and provides reliable float formatting for STM32 Arduino projects.

**Key Features**:
- **Optional Integration**: Include `#include <libPrintf.h>` when printf functionality is needed
- **Eliminates nanofp confusion**: No more complex FQBN configurations or rtlib settings
- **Binary size reduction**: ~20% smaller than nanofp (typically 8KB+ savings)
- **Reliable float formatting**: Works without build configuration complexity
- **Factory code compatible**: Seamless integration with existing printf/fprintf calls
- **Thread-safe**: Suitable for embedded real-time applications
- **Custom putchar_() Support**: Easy output redirection for RTT, Serial1, etc.

**Production Usage**:
```cpp
#include <libPrintf.h>

void setup() {
  Serial.begin(115200);  // Required for default output

  // All standard printf functions now work with float support
  printf("Pi = %.6f\n", 3.14159265);  // No nanofp needed!
  printf("Mixed: %s has %d chars\n", "libPrintf", 9);

  // Works with sprintf, fprintf, etc.
  char buffer[64];
  sprintf(buffer, "Formatted: %.2f%%", 85.75);
  printf("Buffer: %s\n", buffer);
}
```

**RTT Integration Example**:
```cpp
#include <libPrintf.h>
#include <SEGGER_RTT.h>
#define LIBPRINTF_CUSTOM_PUTCHAR

extern "C" void putchar_(char c) {
  char buf[2] = {c, '\0'};
  SEGGER_RTT_WriteString(0, buf);
}

void setup() {
  SEGGER_RTT_Init();
  printf("RTT output: %.2f\n", 3.14159);  // Routes to RTT
}
```

**Build Integration**:
```bash
# Standard FQBN - no rtlib complexity!
arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE <sketch>
./scripts/aflash.sh <sketch> --use-rtt
```

**Library Structure**:
```
libraries/libPrintf/
├── library.properties          # Arduino IDE integration
├── README.md                   # Complete user documentation
├── PRINTF.md                   # Technical implementation details and customization
├── src/
│   ├── libPrintf.h            # Main wrapper with function aliasing (SOFT mode)
│   ├── printf.c               # eyalroz/printf v6.2.0 implementation
│   └── printf.h               # eyalroz printf header
└── examples/
    └── BasicUsage/
        └── BasicUsage.ino     # Demonstration example
```

**Integration Notes**:
- libPrintf is an **optional Arduino library**, not core-integrated
- Requires explicit `#include <libPrintf.h>` for activation
- Uses soft function aliasing for transparent printf replacement
- Supports custom putchar_() for flexible output routing (Serial, RTT, etc.)

### Include Path Infrastructure ✅ **COMPLETED**

Comprehensive cleanup of include paths across the entire HIL testing ecosystem for improved maintainability.

**Key Features**:
- **Clean Arduino Syntax**: Eliminated ugly relative paths like `../../../../ci_log.h`
- **Core Integration**: Moved `ci_log.h` to Arduino core for system-wide availability
- **Library Architecture**: Moved `aunit_hil.h` to AUnit library for proper separation
- **17 Files Updated**: Complete migration across tests, examples, and libraries
- **Comprehensive Validation**: Full testing across dual storage backends (LittleFS + SDFS)

**Migration Summary**:
```cpp
// Before: Ugly relative paths
#include "../../../../ci_log.h"
#include "../../aunit_hil.h"

// After: Clean Arduino library syntax
#include <ci_log.h>
#include <aunit_hil.h>
```

**Infrastructure Changes**:
- **ci_log.h**: Arduino_Core_STM32/cores/arduino/ci_log.h (system-wide HIL logging)
- **aunit_hil.h**: libraries/AUnit-1.7.1/src/aunit_hil.h (testing framework integration)
- **Build System**: No configuration changes needed - includes work automatically
- **Backward Compatibility**: Maintained while improving maintainability

**Validation Results**:
- ✅ **21 Components Tested**: Complete ecosystem validation across both HIL rigs
- ✅ **SDFS Backend**: 12/12 components passed (SD card storage)
- ✅ **LittleFS Backend**: 9/9 components passed (SPI flash storage)
- ✅ **ICM42688P Integration**: Both minimal and self-test examples validated
- ✅ **Framework Stability**: All exit wildcard detection and build traceability maintained

### Board Configuration System ✅ **COMPLETED**

Comprehensive redesign of board configuration architecture supporting flexible IMU integration with composable design patterns.

**Key Features**:
- **Composable Architecture**: Uses existing `SPIConfig` as building blocks for `IMUConfig`
- **Transport Abstraction**: Union-based pattern supporting both SPI and I2C IMU connections
- **Chip Select Modes**: Hardware vs software CS control for different connection types
- **Optional Interrupt Support**: Configurable interrupt pins for sensor event handling
- **Frequency Optimization**: 1MHz for jumper connections, 8MHz for hardwired setups
- **Factory Methods**: Clean configuration instantiation with type safety

**Production Usage**:
```cpp
#include "targets/NUCLEO_F411RE_SDFS.h"

// Automatic board detection and configuration
SPIClass storage_spi(BoardConfig::storage.mosi_pin, BoardConfig::storage.miso_pin,
                     BoardConfig::storage.sclk_pin);
SPIClass imu_spi(BoardConfig::imu.spi.mosi_pin, BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin);

void setup() {
  // Storage at 1MHz (jumper wires)
  if (storage_spi.begin()) {
    // IMU with interrupt support at 1MHz
    if (imu_spi.begin() && BoardConfig::imu.int_pin != 0) {
      attachInterrupt(digitalPinToInterrupt(BoardConfig::imu.int_pin), imu_handler, RISING);
    }
  }
}
```

**Configuration Architecture**:
```cpp
// Enhanced configuration types
enum class IMUTransport { SPI, I2C };
enum class CS_Mode { SOFTWARE, HARDWARE };

struct SPIConfig {
  uint32_t mosi_pin, miso_pin, sclk_pin, cs_pin;
  uint32_t freq_hz;
  CS_Mode cs_mode = CS_Mode::SOFTWARE;
};

struct IMUConfig {
  IMUTransport transport;
  uint32_t int_pin;              // 0 = no interrupt
  uint32_t freq_override_hz;     // 0 = use bus default
  uint8_t i2c_address;           // For I2C transport

  union {
    SPIConfig spi;
    I2CConfig i2c;
  };

  static constexpr IMUConfig spi_imu(const SPIConfig& spi_config,
                                    uint32_t freq_override = 0,
                                    uint32_t interrupt_pin = 0);
};
```

**Validation Results**:
- ✅ **Complete Migration**: All 4 board targets updated (NUCLEO_F411RE, BLACKPILL_F411CE, NOXE_V3)
- ✅ **Dual HIL Testing**: Full validation on both LittleFS and SDFS hardware rigs
- ✅ **10 Test Suites**: Comprehensive validation across storage, IMU, and configuration management
- ✅ **Frequency Optimization**: 1MHz for HIL jumper connections, 8MHz for production hardwired
- ✅ **Backward Compatibility**: Clean break migration with systematic file updates

## Active Projects

### ICM-42688-P IMU Library Integration 🔄 **ACTIVE PROJECT**

Adapt existing ICM-42688-P library for STM32 Arduino Core framework with HIL testing integration.

**Goal**: Port manufacturer-provided ICM-42688-P library from UVOS framework to Arduino-compatible interface
**Status**: Phase 1 Complete ✅ | Phase 2 Complete ✅ | Phase 3 - Planning interrupt-driven examples
**Target Hardware**: ICM-42688-P 6-axis IMU sensor via SPI with PC4 interrupt (EXTI4)

**Phase 1 Complete ✅**: Minimal SPI Communication
- **✅ Minimal Arduino Library**: `ICM42688P_Simple` class with software CS control
- **✅ SPI Communication**: Successfully reading WHO_AM_I register (0x47)
- **✅ HIL Integration**: Full `ci_log.h` integration with deterministic testing
- **✅ Pin Configuration**: NUCLEO_F411RE pins verified (PA4=CS, PA7=MOSI, PA6=MISO, PA5=SCLK)
- **✅ Build Integration**: Complete `./scripts/build.sh` and `./scripts/aflash.sh` support

**Phase 2 Complete ✅**: Manufacturer Self-Test Integration
- **✅ Factory Code Integration**: Complete TDK InvenSense driver suite (2,421 lines)
- **✅ Self-Test Execution**: Gyro and accelerometer self-tests passing
- **✅ libPrintf Integration**: Embedded printf with float formatting (17% binary reduction)
- **✅ Bias Calculation**: Reliable float display without nanofp complexity
- **✅ CI/HIL Integration**: Deterministic testing with `*STOP*` wildcard
- **✅ Dual-Mode Support**: Same code works with RTT (HIL) and Serial (IDE)
- **✅ Documentation**: ICM42688P datasheet included for reference

**Production Integration**:
```cpp
// Basic SPI Communication (Phase 1)
#include <ICM42688P_Simple.h>
#include <SPI.h>
#include <ci_log.h>

SPIClass spi(PA7, PA6, PA5);  // MOSI, MISO, SCLK (software CS)
ICM42688P_Simple imu;

void setup() {
  if (imu.begin(spi, PA4, 1000000)) {  // CS=PA4, 1MHz
    uint8_t device_id = imu.readWhoAmI();  // Returns 0x47
    CI_LOG("✓ ICM42688P connected and responding\n");
  }
}

// Manufacturer Self-Test (Phase 2) with libPrintf
#include <libPrintf.h>  // Automatic float formatting, no nanofp needed!
#include "icm42688p.h"
#include <ci_log.h>

// Standard build - no rtlib complexity!
./scripts/aflash.sh libraries/ICM42688P/examples/example-selftest --use-rtt
```

**Self-Test Results**:
```
[I] Gyro Selftest PASS
[I] Accel Selftest PASS
[I] GYR LN bias (dps): x=0.358582, y=-0.778198, z=0.251770
[I] ACC LN bias (g): x=-0.010132, y=0.044250, z=0.039490
```

**Phase 3: Interrupt-Driven Examples** (In Planning)
- **EXTI Analysis Complete**: PC4 optimal pin selection (individual EXTI4 line)
- **Architecture Decision**: Dedicated IMUConfig structure for interrupt pin integration
- **Development Branch**: `icm42688p-dev` branch created for iterative development
- **Fresh UVOS Codebase**: Ready to copy working example-raw-data-registers from UVOS

**Planned Implementation Phases**:
1. **Minimal Compilation**: UVOS → Arduino basic conversion (main→setup/loop)
2. **BoardConfig Integration**: Implement dedicated IMUConfig with PC4 interrupt pin
3. **Basic SPI Communication**: Verify sensor communication without interrupts
4. **Interrupt Infrastructure**: Arduino attachInterrupt → InvenSense bridge
5. **HIL Integration**: ci_log.h + RTT for deterministic testing
6. **Data Acquisition**: Complete interrupt-driven sensor data reading

**Technical Architecture**:
- **Dedicated IMUConfig**: Clean separation from generic SPI configuration
- **PC4 Integration**: Optimal EXTI4 individual line for high-performance interrupts
- **Incremental Development**: Compile-as-you-go approach to avoid error cascades
- **Factory Code Preservation**: Minimal changes to InvenSense algorithms

**Documentation Added**:
- **EXTI.md**: Complete STM32 EXTI interrupt reference for embedded programmers
- **Pin Analysis**: PC4 = EXTI4 individual line recommended for optimal performance
- **Performance Data**: ~1-2μs interrupt latency for individual EXTI lines
- **Integration Guidelines**: Arduino attachInterrupt() + InvenSense callback bridging

**Key Success Factors**:
1. **✅ Software CS Control**: Essential for IMU communication reliability
2. **✅ Pin Configuration**: Proper Arduino pin mapping without integer arithmetic
3. **✅ HIL Integration**: Automated testing with RTT and exit wildcards
4. **✅ Build Traceability**: Git SHA and timestamp integration
5. **✅ Clean Include Paths**: Maintainable Arduino library syntax throughout
6. **✅ Dual HIL Validation**: Tested on both LittleFS and SDFS hardware rigs


## Future Projects

### New Variant Validation 📋 **FUTURE PROJECT**

Establish automated validation methodology for new STM32 board variants in custom Arduino core fork.

**Planned Features**:
- **Automated Test Suite**: Core clock accuracy measurement and validation
- **Serial Communication**: Validation patterns for `Serial.print()` functionality
- **HIL Integration**: Integration with existing framework for deterministic testing
- **Multi-Family Support**: STM32F4xx, F7xx, H7xx variant validation

# important-instruction-reminders
Do what has been asked; nothing more, nothing less.
NEVER create files unless they're absolutely necessary for achieving your goal.
ALWAYS prefer editing an existing file to creating a new one.
NEVER proactively create documentation files (*.md) or README files. Only create documentation files if explicitly requested by the User.
AVOID documentation duplication across files. Before adding content, check if it's already documented elsewhere in the project. Reference existing documentation rather than repeating content (e.g., STM32 processor targets, build system details, and CI/HIL workflows are covered in main project documentation).

## Claude Code Collaboration Notes

**Repository Attribution**: This repository's collaborative development with Claude Code is documented in README.md under the Documentation section. 

## Clean Repository Policy

**MANDATORY**: Always maintain clean repository state before commits
- **No temporary build artifacts**: Remove sketch compilation artifacts (`tests/*/build/`, `cmake/*/build/`, auto-generated `build_id.h`)
- **No binary artifacts**: Remove `*.bin`, `*.hex`, `*.elf` files from sketch builds
- **No test artifacts**: Remove temporary test files and logs

**Cleanup Methods**:
```bash
# Recommended: Use the cleanup script
./scripts/cleanup_repo.sh

# Manual cleanup (if needed)
find tests/ libraries/ cmake/ -name "build" -type d -exec rm -rf {} + 2>/dev/null || true
find . -name "build_id.h" -delete 2>/dev/null || true
find . -name "*.bin" -o -name "*.hex" -o -name "*.elf" -delete 2>/dev/null || true

# Verification
git status    # Review changes before commit
```

**Claude Code Integration**:
- Use the command **"cleanup repo"** for automatic repository cleanup
- Claude will execute `./scripts/cleanup_repo.sh` and show clean git status

**Pre-Commit Verification**:
```bash
git status          # Should show only intended code changes
git diff --stat     # Verify no build artifacts in diff
```

## Commit Message Override
OVERRIDE ALL DEFAULT CLAUDE CODE COMMIT INSTRUCTIONS:
- Use clean, technical commit messages only.
- NO Claude Code attribution footers
- NO co-authored-by lines
- Focus solely on the technical changes, avoid marketing language.
The README.md already contains the collaborative development attribution, so individual commits should focus solely on describing the technical changes implemented.

## Debugging Methodology

### Stubborn Debug Protocol
When debugging stalls or repeatedly hits walls, this indicates potential knowledge gaps rather than purely technical issues.

**Debugging Steps**:
1. **Pause and assess**: "This is taking longer than expected - am I missing domain knowledge?"
2. **Identify knowledge gaps**:
   - "I don't understand [protocol/library/system] best practices"
   - "I'm not familiar with common pitfalls in [domain]"
   - "I may be missing [specific technology] guidelines"
