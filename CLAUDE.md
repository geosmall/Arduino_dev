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
- `libraries/` - Additional STM32-specific libraries (LittleFS, SDFS, STM32RTC, etc.)
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
This repository is specifically designed for **UAV flight controller boards** with the following STM32 microcontrollers:

- **STM32F411** - Primary target (Nucleo F411RE, BlackPill F411CE for development)
- **STM32F405** - Secondary target (common in flight controllers) 
- **STM32H743** - Future target (high-performance flight controllers)

### Target Applications
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

**Integration Pattern**:
```cpp
#include "../../../../ci_log.h"
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
- **15 Total Tests**: LittleFS (8 tests), SDFS (7 tests), framework validation (3 tests)
- **Hardware Validation**: Real SPI flash and SD card testing on STM32F411RE
- **Dual-Mode Support**: Same tests work with RTT (HIL) and Serial (IDE)
- **Type Safety**: Established AUnit assertion patterns for embedded types

**Production Usage**:
```bash
./scripts/aflash.sh tests/LittleFS_Unit_Tests --use-rtt --build-id
./scripts/aflash.sh tests/SDFS_Unit_Tests --use-rtt --build-id
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

## Active Projects

### ICM-42688-P IMU Library Integration 🔄 **ACTIVE PROJECT**

Adapt existing ICM-42688-P library for STM32 Arduino Core framework with HIL testing integration.

**Goal**: Port manufacturer-provided ICM-42688-P library from UVOS framework to Arduino-compatible interface
**Status**: Phase 1 Complete ✅ | Phase 2 Complete ✅ | Phase 3 - Data acquisition in progress
**Target Hardware**: ICM-42688-P 6-axis IMU sensor via SPI

**Phase 1 Complete ✅**: Minimal SPI Communication
- **✅ Minimal Arduino Library**: `ICM42688P_Simple` class with software CS control
- **✅ SPI Communication**: Successfully reading WHO_AM_I register (0x47)
- **✅ HIL Integration**: Full `ci_log.h` integration with deterministic testing
- **✅ Pin Configuration**: NUCLEO_F411RE pins verified (PA4=CS, PA7=MOSI, PA6=MISO, PA5=SCLK)
- **✅ Build Integration**: Complete `./scripts/build.sh` and `./scripts/aflash.sh` support

**Phase 2 Complete ✅**: Manufacturer Self-Test Integration
- **✅ Factory Code Integration**: Complete TDK InvenSense driver suite (2,421 lines)
- **✅ Self-Test Execution**: Gyro and accelerometer self-tests passing
- **✅ Bias Calculation**: Float printf integration with bias value display
- **✅ CI/HIL Integration**: Deterministic testing with `*STOP*` wildcard
- **✅ Dual-Mode Support**: Same code works with RTT (HIL) and Serial (IDE)
- **✅ Documentation**: ICM42688P datasheet included for reference

**Production Integration**:
```cpp
// Basic SPI Communication (Phase 1)
#include <ICM42688P_Simple.h>
#include <SPI.h>
#include "../../../../ci_log.h"

SPIClass spi(PA7, PA6, PA5);  // MOSI, MISO, SCLK (software CS)
ICM42688P_Simple imu;

void setup() {
  if (imu.begin(spi, PA4, 1000000)) {  // CS=PA4, 1MHz
    uint8_t device_id = imu.readWhoAmI();  // Returns 0x47
    CI_LOG("✓ ICM42688P connected and responding\n");
  }
}

// Manufacturer Self-Test (Phase 2) - REQUIRES Float Printf
// Build with: STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE,rtlib=nanofp
./scripts/aflash.sh libraries/ICM42688P/examples/example-selftest "STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE,rtlib=nanofp" --use-rtt
```

**Self-Test Results**:
```
[I] Gyro Selftest PASS
[I] Accel Selftest PASS
[I] GYR LN bias (dps): x=0.358582, y=-0.778198, z=0.251770
[I] ACC LN bias (g): x=-0.010132, y=0.044250, z=0.039490
```

**Current Development Plan**:

**Phase 3: Data Acquisition and Configuration** (Next)
1. **Sensor Data Reading** - Accelerometer and gyroscope data acquisition
2. **Configuration Management** - Sample rates, ranges, filters
3. **Interrupt Handling** - Data ready and threshold interrupts
4. **Calibration Support** - Zero-offset and sensitivity calibration

**Phase 4: Advanced Features** (Future)
5. **FIFO Management** - Buffered data acquisition
6. **Motion Detection** - Wake-on-motion and threshold detection
7. **Power Management** - Low-power modes and sleep states

**Technical Architecture**:
- **Minimal Layer**: `ICM42688P_Simple` for basic register access
- **Advanced Layer**: Full manufacturer driver integration
- **Software CS Control**: Required for reliable IMU communication
- **HIL Testing**: Deterministic validation at each layer

**Key Success Factors**:
1. **✅ Software CS Control**: Essential for IMU communication reliability
2. **✅ Pin Configuration**: Proper Arduino pin mapping without integer arithmetic
3. **✅ HIL Integration**: Automated testing with RTT and exit wildcards
4. **✅ Build Traceability**: Git SHA and timestamp integration

### EmbeddedPrintf Integration 🔄 **ACTIVE PROJECT**

Replace STM32 Arduino Core's problematic newlib printf with eyalroz/printf embedded implementation to eliminate nanofp confusion and reduce binary bloat.

**Goal**: Integrate eyalroz/printf v0.6.2 to provide reliable float formatting without FQBN complexity
**Status**: Phase 1 - Temporary integration in progress
**Target**: Eliminate `rtlib=nanofp` requirement and reduce binary size by ~20KB

**Problem Statement**:
- **FQBN Confusion**: `rtlib=nanofp` vs `rtlib=nano` causes build complexity
- **Binary Bloat**: nanofp adds ~10KB vs embedded printf savings of ~20KB
- **Float Formatting Issues**: Inconsistent behavior between Serial and RTT output
- **User Experience**: Complex build commands required for float support

**Solution Architecture**:
- **Phase 1**: Temporary integration in ICM42688P examples for validation
- **Phase 2**: Arduino_Core_STM32 integration to replace newlib printf system-wide
- **Integration**: Override `printf/sprintf/snprintf` family with embedded versions

**Phase 1 ✅ Complete**: Temporary Integration
- **✅ Research**: eyalroz/printf library analysis complete
- **✅ Download**: eyalroz/printf v6.2.0 source integration
- **✅ ICM42688P Integration**: Replace INV_MSG printf in example-selftest
- **✅ Testing**: Validate float formatting without nanofp requirement
- **✅ Verification**: Confirm RTT/Serial output consistency with bias values
- **✅ FQBN Simplification**: Standard build commands now work for float operations

**Phase 2 📋 Planned**: Core Integration
- **Arduino_Core_STM32 Integration**: Add printf/ directory to cores/arduino/
- **Platform Override**: Remove nanofp options from platform.txt
- **System-wide Adoption**: Update all examples to use standard FQBN
- **Putchar Implementation**: Route output to Serial/RTT consistently

**Technical Benefits**:
- **Simplified Builds**: Standard FQBN works for all float operations
- **Smaller Binaries**: ~20KB reduction vs nanofp approach
- **Consistent Behavior**: Same printf across RTT/Serial output
- **No Dependencies**: Self-contained, no libc linking issues
- **Embedded-Optimized**: Designed specifically for resource-constrained systems

**Before/After Results**:
```bash
# Before (Complex, Bloated)
./scripts/aflash.sh example-selftest "STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE,rtlib=nanofp" --use-rtt
# Binary: 41,916 bytes

# After (Simple, Optimized)
./scripts/aflash.sh example-selftest --use-rtt
# Binary: 34,612 bytes (17% reduction)
```

**Verification Results**:
```
[I] GYR LN bias (dps): x=0.366211, y=-0.778198, z=0.259399
[I] ACC LN bias (g): x=-0.013184, y=0.039551, z=0.039490
```

**Implementation Details**:
- **eyalroz/printf v6.2.0**: Temporary integration in example-selftest
- **Function Override**: `PRINTF_ALIAS_STANDARD_FUNCTION_NAMES_SOFT` approach
- **Factory Code Changes**: Minimal 4-line addition to inv_main.c
- **Output Routing**: `putchar_()` implementation for RTT/Serial consistency

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
# Complete cleanup (all build artifacts)
find tests/ libraries/ cmake/ -name "build" -type d -exec rm -rf {} + 2>/dev/null || true
find . -name "build_id.h" -delete 2>/dev/null || true
find . -name "*.bin" -o -name "*.hex" -o -name "*.elf" -delete 2>/dev/null || true

# Manual verification
git status    # Review changes before commit
```

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
