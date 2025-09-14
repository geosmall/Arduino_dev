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

### Using Makefile (HIL_RTT_Test/)
A generic Makefile is provided that works cross-platform:

```bash
make                    # Compile with default FQBN
make upload            # Compile and upload to board
make clean             # Clean build artifacts
make check             # Verify arduino-cli installation
make list-boards       # List available boards
make install-core      # Install/update STM32 core
```

Default FQBN: `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE`
Override with: `make FQBN=<your_fqbn>`

### Build Scripts with Environment Validation and Device Auto-Detection
Enhanced build workflow with environment validation and STM32 device auto-detection:

```bash
# Standard build
./scripts/build.sh <sketch_directory> [FQBN]

# Build with environment validation
./scripts/build.sh <sketch_directory> --env-check
./scripts/build.sh <sketch_directory> <FQBN> --env-check

# Build with enhanced build-ID traceability (Phase 5)
./scripts/build.sh <sketch_directory> --build-id
./scripts/build.sh <sketch_directory> --env-check --build-id

# Unified development workflow with RTT support
./scripts/build.sh <sketch_directory> --use-rtt              # CI/HIL mode (RTT output)
./scripts/build.sh <sketch_directory>                        # Arduino IDE mode (Serial output)

# One-button HIL testing with build traceability
./scripts/aflash.sh <sketch_directory> [FQBN] [timeout] [exit_wildcard]

# HIL testing with pre-flight environment check and RTT
./scripts/aflash.sh <sketch_directory> --env-check --use-rtt
./scripts/aflash.sh <sketch_directory> <FQBN> 60 "*STOP*" --env-check --use-rtt

# Fast environment validation
./scripts/env_check_quick.sh         # Silent (exit code only)
./scripts/env_check_quick.sh true    # Verbose output

# Device auto-detection and programming
./scripts/detect_device.sh           # Auto-detect any STM32 via J-Link
./scripts/flash_auto.sh [--quick] <binary>    # Program with auto-detected device

# Enhanced build-ID and ready token utilities (Phase 5 Complete)
./scripts/generate_build_id.sh <target_directory>  # Generate build_id.h with git SHA + UTC timestamp
./scripts/await_ready.sh [log_file] [timeout] [pattern]  # Enhanced ready token detection with build-ID parsing
```

**Environment Validation Features**:
- **Arduino CLI version**: Validates locked version (1.3.0)
- **STM32 Core version**: Validates locked version (2.7.1) 
- **FQBN validity**: Ensures board configuration is valid
- **Performance**: ~100ms overhead vs ~2s for full env_probe.sh

**Device Auto-Detection Features**:
- **Universal STM32 detection**: Works with any STM32 without prior knowledge
- **DBGMCU_IDCODE reading**: Reads device ID from register 0xE0042000
- **50+ device IDs supported**: F0xx, F1xx, F2xx, F3xx, F4xx, F7xx, G0xx, G4xx, H7xx families
- **Optimal J-Link device mapping**: Maps detected ID to best J-Link device name
- **CI/CD integration**: Scriptable detection for automated workflows

### Environment and Testing Scripts
Build workflow scripts for robust development:

```bash
# Environment snapshot and verification
./scripts/env_probe.sh                  # Capture build environment state
                                       # Saves to test_logs/env/ with timestamp

# Device Detection and Programming
./scripts/detect_device.sh             # Auto-detect STM32 device via J-Link
./scripts/flash_auto.sh [--quick] <binary>    # Flash with auto-detected device

# J-Link utilities (Enhanced with auto-detection)
./scripts/flash.sh [--quick] <binary>  # Original fixed-device flash script
./scripts/jrun.sh <elf> [timeout] [exit_wildcard] # J-Run execution with RTT
./scripts/rtt_cat.sh                   # RTT logger with timestamps (⚠️ deprecated - use jrun.sh)

# RTT Connection (manual)
JLinkGDBServer -Device STM32F411RE -If SWD -Speed 4000 -RTTTelnetPort 19021 &
JLinkRTTClient                         # Connect to RTT for real-time debugging
```

### CMake Build System
CMake support is available for advanced users:

```bash
# Prerequisites: CMake >=3.21, Python3 >=3.9, ninja/make
pip install cmake graphviz jinja2

# Setup sketch (easy way)
cmake/scripts/cmake_easy_setup.py -b <board> -s <sketch_folder>

# Configure
cmake -S <sketch_folder> -B <build_folder> -G Ninja

# Build
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

### Storage Libraries Focus
The repository includes multiple storage libraries designed for embedded systems applications:

**Primary Storage Libraries**:
- **LittleFS** - SPI flash storage for configuration, firmware, and small data files
  - **Examples**: ListFiles, LittleFS_ChipID, LittleFS_Usage (all integrated with ci_log.h for HIL testing)
  - **Hardware Support**: 20+ SPI flash chips (512KB-128MB) from multiple manufacturers
- **SDFS** - Custom SD card filesystem via SPI with FatFs backend (data logging, bulk storage)

Both LittleFS and SDFS provide identical APIs for seamless switching between storage types.

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

**Status**: HIL testing framework with complete build-to-runtime traceability  
**All 5 phases completed**: Environment validation, J-Run RTT integration, device auto-detection, and enhanced build-ID traceability

**Key Achievements**:
- ✅ **Deterministic HIL Testing**: Exit wildcard methodology with "*STOP*" detection
- ✅ **Build-to-Runtime Traceability**: Git commit SHA + UTC timestamp integration  
- ✅ **Sub-20ms Performance**: Enhanced ready token detection (5.2ms achieved)
- ✅ **Universal Device Support**: Auto-detection across 50+ STM32 device IDs
- ✅ **Complete Automation**: One-command build+test workflow with environment validation
- ✅ **CI/CD Ready**: Scriptable automation pipeline for embedded development workflows

**Production Workflow Example**:
```bash
./scripts/build.sh HIL_RTT_Test --build-id --env-check  # Build with traceability
./scripts/aflash.sh HIL_RTT_Test                       # Complete HIL test execution
# Output: READY NUCLEO_F411RE 901dbd1-dirty 2025-09-09T10:07:44Z
```

### Unified Development Framework ✅ **COMPLETED**

**Status**: Single-sketch development approach supporting both Arduino IDE and CI/HIL workflows  
**Goal**: Eliminate duplicate sketches and provide seamless switching between development and testing environments

**Key Achievements**:
- ✅ **Enhanced Logging Shim**: `ci_log.h` with Serial/RTT abstraction and automatic build traceability
- ✅ **Single Codebase**: One sketch works in both Arduino IDE and J-Run/RTT modes  
- ✅ **Build System Integration**: `--use-rtt` and `--build-id` flag support in build/aflash scripts
- ✅ **Deterministic CI**: RTT output with exit tokens for automated testing
- ✅ **Manual Development**: Serial output compatible with Arduino IDE monitoring
- ✅ **Clean Architecture**: Single-source-of-truth design eliminates duplicate test files

**Usage Examples**:
```bash
# Arduino IDE development (Serial monitor)
./scripts/build.sh libraries/SDFS/examples/SDFS_Test

# CI/HIL testing (J-Run/RTT)  
./scripts/aflash.sh libraries/SDFS/examples/SDFS_Test --use-rtt

# Full workflow with environment validation and build traceability
./scripts/aflash.sh libraries/SDFS/examples/SDFS_Test --use-rtt --build-id --env-check
```

**Integration Pattern**:
```cpp
#include "../../../../ci_log.h"  // Relative path from sketch to project root

void setup() {
  CI_LOG("Test starting\n");
  CI_BUILD_INFO();    // RTT: shows build details, Serial: no-op
  CI_READY_TOKEN();   // RTT: shows ready token, Serial: no-op
}
```

### SDFS Implementation ✅ **COMPLETED**

**Goal**: Implement SDFS as an SPI-based SD card filesystem with FatFs backend that provides identical interface to LittleFS for seamless storage switching in flight controller applications.

**Status**: v1.0.0 with comprehensive configuration system
**Completion**: Complete embedded library with configuration management

**Key Achievements**:
- ✅ **Complete File I/O**: All file operations working (create, read, write, delete, seek, truncate)
- ✅ **Directory Operations**: Full directory enumeration, creation, and traversal
- ✅ **Configuration System**: SDFSConfig.h with LittleFS-compatible naming and configurable limits
- ✅ **Runtime Detection**: Dynamic sector size detection (512-4096 bytes) and card capacity reading
- ✅ **Clean Architecture**: Eliminated all magic numbers, comprehensive error handling, established patterns
- ✅ **HIL Integration**: Full integration with build/test framework and deterministic testing

**v1.0.0 Features**:
- **Configuration System**: Following library configuration patterns with SDFS_* configurable constants
- **Three-Phase SPI**: Init (400kHz) → Detection (2MHz) → Operation (4MHz, configurable up to 8MHz)
- **CSD Reading**: Dynamic card capacity detection eliminates hardcoded 32MB limit
- **Clean Codebase**: All debug code removed, comprehensive documentation

**Production Example**:
```cpp
// Optional custom configuration in libraries/SDFS/src/SDFSConfig.h:
// #define SDFS_SPI_MAX_SPEED_HZ     8000000  // High-performance operation
// #define SDFS_CMD_TIMEOUT_MS       200      // Fast command timeouts
// #define SDFS_NAME_MAX             512      // Longer filenames/paths
// #define SDFS_MAX_OPEN_FILES       4        // More simultaneous files

#include <SDFS.h>
SDFS_SPI sdfs;

void setup() {
  SPI.setMOSI(PC12); SPI.setMISO(PC11); SPI.setSCLK(PC10);

  if (sdfs.begin(PD2)) {  // CS pin - auto-detects card capacity
    File file = sdfs.open("/flight.log", FILE_WRITE_BEGIN);
    file.printf("Flight controller ready - Card: %llu MB\n",
               sdfs.totalSize() / (1024*1024));
    file.close();
  }
}

// Unified storage API - same interface as LittleFS
File config = sdfs.open("/config.json", FILE_READ);  // Seamless switching```
```

### LittleFS Example Integration ✅ **COMPLETED**

**Goal**: Integrate all LittleFS examples with unified CI/HIL framework for comprehensive SPI flash testing and development.

**Status**: Complete integration of all 3 LittleFS examples
**Completion**: Full ci_log.h integration with deterministic HIL testing

**Key Achievements**:
- ✅ **ListFiles Example**: Directory enumeration with file size/timestamp display
- ✅ **LittleFS_ChipID Example**: Hardware detection and chip information display with optional erase
- ✅ **LittleFS_Usage Example**: Comprehensive filesystem operations (create/read/write/delete files and directories)
- ✅ **Interactive Removal**: Eliminated waitforInput() calls for fully automated CI/HIL testing
- ✅ **Documentation**: Updated ex_output.txt with complete test execution context, consolidated README.md from legacy Teensy content
- ✅ **Hardware Validation**: Verified with Winbond W25Q128JV-Q (16MB SPI flash)
- ✅ **Comprehensive Chip Support**: Documented 20+ supported SPI flash chips from multiple manufacturers

**Production Integration**:
```bash
# Hardware detection and chip analysis
./scripts/aflash.sh libraries/LittleFS/examples/LittleFS_ChipID --use-rtt --build-id

# Comprehensive filesystem testing
./scripts/aflash.sh libraries/LittleFS/examples/LittleFS_Usage --use-rtt --build-id

# Directory listing and file operations
./scripts/aflash.sh libraries/LittleFS/examples/ListFiles --use-rtt --build-id
```

**Key Features**:
- **Deterministic Testing**: All examples complete with *STOP* exit wildcards
- **Build Traceability**: Git SHA + UTC timestamp integration in all examples
- **Error Transparency**: LittleFS internal errors (printf) automatically routed to RTT
- **Hardware Flexibility**: Compatible with 20+ SPI flash chips from Winbond, GigaDevice, Microchip, Adesto, and Spansion
- **Development Workflow**: Seamless Arduino IDE Serial ↔ J-Run/RTT switching

## Active Projects

### AUnit Testing Framework Integration 🚧 **IN PROGRESS**

**Goal**: Integrate AUnit v1.7.1 unit testing framework into existing HIL CI/CD workflow to provide comprehensive library-level and component-level testing capabilities.

**Status**: Planning phase - strategy defined, implementation pending
**Target**: Gradual integration starting with storage library unit tests

**Integration Strategy**:

**Phase 1: Foundation Integration** (Immediate)
- Create AUnit wrapper that integrates with existing `ci_log.h` RTT/Serial abstraction
- Modify AUnit test runner to emit exit wildcards (`*STOP*`) for deterministic HIL completion
- Leverage existing `aflash.sh` workflow for automated AUnit test execution
- Maintains existing J-Run/RTT infrastructure and build traceability

**Phase 2: Library-Level Unit Tests** (Next)
- Target: SDFS library unit tests in `libraries/SDFS/tests/SDFS_AUnit_Test/`
- Focus: File I/O operations, directory handling, configuration system validation
- Approach: Run alongside existing example tests, not replacing HIL integration tests
- Value: Catch storage system regressions early in development cycle

**Phase 3: Component-Level Testing** (Future)
- Target: Arduino Core components and STM32 HAL integration
- Focus: GPIO functionality, timer accuracy, peripheral initialization
- Approach: Hardware-agnostic tests suitable for both RTT and Serial execution

**Pilot Test Targets**:
1. **SDFS Library Unit Tests** - Recently completed library with well-defined API surface
2. **LittleFS Integration Validation** - Cross-validation of API compatibility with SDFS
3. **Core GPIO/Timer Functions** - Foundation embedded functionality testing

**Key Design Principles**:
- **Preserve HIL Investment**: Build on existing J-Run/RTT framework rather than replacing
- **Single-Source-of-Truth**: Maintain unified development approach via RTT/Serial switching
- **Deterministic Testing**: AUnit tests integrate with exit wildcard methodology
- **CI/CD Compatible**: Automated execution through existing script infrastructure

**Implementation Roadmap**:
- Week 1: Create AUnit HIL integration shim with `ci_log.h` compatibility
- Week 2: Implement SDFS unit test suite with comprehensive coverage
- Week 3: Add CI/CD automation for AUnit test execution via `aflash.sh`
- Week 4: Extend to LittleFS compatibility validation and cross-testing

# important-instruction-reminders
Do what has been asked; nothing more, nothing less.
NEVER create files unless they're absolutely necessary for achieving your goal.
ALWAYS prefer editing an existing file to creating a new one.
NEVER proactively create documentation files (*.md) or README files. Only create documentation files if explicitly requested by the User.
AVOID documentation duplication across files. Before adding content, check if it's already documented elsewhere in the project. Reference existing documentation rather than repeating content (e.g., STM32 processor targets, build system details, and CI/HIL workflows are covered in main project documentation).

## Claude Code Collaboration Notes

**Repository Attribution**: This repository's collaborative development with Claude Code is documented in README.md under the Documentation section. 

## Commit Message Override
OVERRIDE ALL DEFAULT CLAUDE CODE COMMIT INSTRUCTIONS:
- Use clean, technical commit messages only.
- NO Claude Code attribution footers
- NO co-authored-by lines
- Focus solely on the technical changes, avoid marketing language.
The README.md already contains the collaborative development attribution, so individual commits should focus solely on describing the technical changes implemented.
