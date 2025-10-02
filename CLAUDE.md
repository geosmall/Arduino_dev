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
- `ICM42688P` - Low-level 6-axis IMU library with TDK InvenSense drivers and manufacturer self-test
- `IMU` - High-level C++ wrapper for InvenSense IMU sensors with chip detection and multi-instance support
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

Comprehensive compile-time board configuration system supporting multiple target platforms with automatic detection, flexible peripheral configuration, and hardware/software chip select control.

**Key Features**:
- **Multi-Board Support**: NUCLEO_F411RE, BLACKPILL_F411CE, NOXE_V3 with automatic selection via `ARDUINO_*` defines
- **CS Mode Control**: Software vs hardware chip select modes via CS_Mode enum with get_ssel_pin() helper
- **Composable Architecture**: SPIConfig building blocks for storage and IMU configurations
- **Transport Abstraction**: Union-based pattern supporting SPI and I2C peripherals
- **Optional Interrupts**: Configurable interrupt pins for event-driven sensor operation
- **Frequency Optimization**: 1MHz for jumper connections, 8MHz for hardwired setups
- **Complete Integration**: Storage (LittleFS/SDFS) and IMU (ICM42688P) fully validated

**Usage**:
```cpp
#include "targets/NUCLEO_F411RE_LITTLEFS.h"
SPIClass spi(BoardConfig::storage.mosi_pin, BoardConfig::storage.miso_pin,
             BoardConfig::storage.sclk_pin, BoardConfig::storage.get_ssel_pin());
if (BoardConfig::imu.int_pin != 0) {
  attachInterrupt(digitalPinToInterrupt(BoardConfig::imu.int_pin), handler, RISING);
}
// See targets/*.h for board configurations and examples for complete usage
```

### Generic Storage Abstraction ✅ **COMPLETED**

Unified storage interface abstracting SDFS and LittleFS with automatic backend selection via BoardConfig. Factory pattern provides runtime safety with single API for both storage backends.

**Usage**:
```cpp
#include <Storage.h>
#include <BoardStorage.h>
BoardStorage::begin(BoardConfig::storage);
Storage& fs = BOARD_STORAGE;
File log = fs.open("/flight.log", FILE_WRITE);
```

### minIni Configuration Management System ✅ **COMPLETED**

INI file configuration management integrated with Generic Storage Abstraction for both LittleFS and SDFS backends.

**Key Features**:
- **minIni v1.5**: Latest version with enhanced validation (hassection/haskey)
- **Storage Integration**: Works with both LittleFS and SDFS via unified interface
- **Data Types**: Strings, integers, floats, booleans with section/key enumeration
- **HIL Testing**: Full deterministic validation with 6 test suites

**Usage**:
```cpp
#include <minIniStorage.h>
minIniStorage config("config.ini");
config.begin(BoardConfig::storage);
config.put("network", "ip_address", "192.168.1.100");
std::string ip = config.gets("network", "ip_address", "default");
// See tests/minIniStorage_Unit_Tests for complete examples
```

### libPrintf Embedded Printf Library ✅ **COMPLETED**

Optional Arduino library (eyalroz/printf v6.2.0) that eliminates nanofp confusion and provides reliable float formatting with ~20% binary size reduction (8KB+ savings).

**Key Features**:
- Eliminates complex FQBN rtlib configurations
- Supports custom putchar_() for RTT/Serial routing
- Thread-safe for embedded applications

**Usage**:
```cpp
#include <libPrintf.h>
printf("Pi = %.6f\n", 3.14159265);  // Float formatting works automatically
```

### ICM-42688-P IMU Library Integration ✅ **COMPLETED**

Complete Arduino-compatible ICM42688P library with manufacturer-grade reliability and performance. Successfully adapted from UVOS framework while preserving 100% of InvenSense factory algorithms.

**Target Hardware**: ICM-42688-P 6-axis IMU sensor via SPI with PC4 interrupt (EXTI4)

**Key Features**:
- **Factory Code Preservation**: Zero modifications to InvenSense sensor algorithms
- **Multiple Usage Modes**: Basic SPI, self-test, interrupt-driven data, processed AG data
- **BoardConfig Integration**: Dynamic pin/frequency configuration support
- **HIL Testing**: Automated validation with RTT and build traceability
- **Arduino Ecosystem**: Full compatibility with Arduino IDE and CLI

**Available Examples**:
1. **ICM42688P_Simple**: Basic SPI communication and device identification
2. **example-selftest**: Manufacturer self-test with bias calculation
3. **example-raw-data-registers**: Interrupt-driven raw sensor data acquisition
4. **example-raw-ag**: Processed accelerometer/gyroscope data with clock calibration

**Usage**:
```cpp
#include <ICM42688P_Simple.h>
SPIClass spi(PA7, PA6, PA5);
ICM42688P_Simple imu;
imu.begin(spi, PA4, 1000000);  // Returns 0x47 device ID
// See libraries/ICM42688P/examples/ for complete examples
```

### High-Level IMU Library ✅ **COMPLETED**

Unified C++ wrapper library for InvenSense IMU sensors, starting with ICM-42688-P and designed for easy extension to MPU-6000, MPU-9250, and other InvenSense parts.

**Key Features**:
- **Context-Based Design**: Multi-instance support via `serif->context` pointer
- **Chip Detection**: ChipType enum with GetChipType() for ICM42688_P, MPU-6000, MPU-9250
- **Interrupt Support**: EnableDataReadyInt1() and DisableDataReadyInt1() methods
- **Full API**: Init, Reset, RunSelfTest, ReadIMU6, sensor configuration (FSR, ODR, power modes)
- **BoardConfig Integration**: Works with multi-board configuration system
- **HIL Validated**: Both examples tested on NUCLEO_F411RE hardware

**Available Examples**:
1. **imu-selftest**: Manufacturer self-test with bias calculation and chip detection
2. **imu-raw-data-registers**: Interrupt-driven raw sensor data acquisition at 1kHz

**Usage**:
```cpp
#include <IMU.h>
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin, BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin, BoardConfig::imu.spi.get_ssel_pin());
IMU imu;
imu.Init(spi_bus, BoardConfig::imu.spi.cs_pin, BoardConfig::imu.spi.freq_hz);
IMU::ChipType chip = imu.GetChipType();  // Returns ICM42688_P (0x47)
// See libraries/imu/examples/ for complete examples
```

**Notes**: Currently supports ICM-42688-P only. Chip detection framework ready for future MPU-6000/MPU-9250 support.

## Active Projects

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
