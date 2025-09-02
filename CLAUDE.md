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
- `libraries/` - Additional STM32-specific libraries (LittleFS, SDFS, STM32RTC, SD, SdFat, etc.)
- `MyFirstSketch/` - Example Arduino sketch with Makefile

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

### Using Makefile (MyFirstSketch/)
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

### Environment and Testing Scripts
Build workflow scripts for robust development:

```bash
# Environment snapshot and verification
./scripts/env_probe.sh                  # Capture build environment state
                                       # Saves to test_logs/env/ with timestamp

# J-Link utilities (Phase 1 complete)
./scripts/jlink_upload.sh <binary>     # Upload via J-Link (bypasses STM32CubeProgrammer)
./scripts/rtt-test.sh                  # RTT testing framework (SDFS-focused)
./scripts/rtt-test.sh -i               # Interactive RTT session

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
The repository includes multiple storage libraries designed for flight controller applications:

**Primary Storage Libraries**:
- **LittleFS** - SPI flash storage for configuration, firmware, and small data files
- **SDFS** - Custom SD card filesystem via SPI with FatFs backend (data logging, bulk storage)

**Additional SD Card Libraries**:
- **SD** - Arduino standard SD card library (v1.3.0) for basic SD card operations
- **SdFat** - Advanced SD card library (v2.1.2) with full FAT16/FAT32/exFAT support, optimized for Teensy but compatible with STM32

Both LittleFS and SDFS provide identical APIs for seamless switching between storage types. The SD and SdFat libraries offer alternative approaches for SD card access with different feature sets and performance characteristics.

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
- `SD` - Arduino standard SD card library (v1.3.0) - basic SD card access
- `SdFat` - Advanced SD card library (v2.1.2) with FAT16/FAT32/exFAT support, Teensy-optimized

### Hardware Abstraction

The core integrates three levels of STM32 APIs:
1. **HAL (Hardware Abstraction Layer)** - High-level, feature-rich APIs
2. **LL (Low Layer)** - Optimized, register-level APIs
3. **CMSIS** - ARM Cortex standard interface

Board-specific configurations are defined through the variant system, allowing the same core to support hundreds of STM32 boards with different pin layouts and capabilities.

## Active Projects

### Build Workflow

**Goal**: Step-by-step plan to build up robust a robust build snd test environment we can rely on for embedded HIL app development.
- Move in small, verifiable phases and keep things token-efficient for working with agentic AI.

Script names (to be created as we go):

- scripts/env_probe.sh - Probe installed environment to capture versions, FQBNs, paths, hardware status (fail loudly and exit on errors)
- scripts/build.sh → arduino-cli compile (cached build dir per sketch)
- scripts/flash.sh → JLinkExe batch: loadfile, r, g, qc
- scripts/rtt_cat.sh → attaches RTT, timestamps lines, writes to test_logs/
- scripts/aflash.sh → orchestrates the three

Phase 0 — Pin, probe, and snapshot the toolchain ✅ **COMPLETED**

Goal: deterministic environment + quick "can compile" proof.
- ✅ Locked versions: arduino-cli 1.3.0, STM32 core 2.7.1, GCC 12.2.1-1.2
- ✅ Created scripts/env_probe.sh to capture versions, FQBNs, paths, hardware status
- ✅ Canonical FQBN established: STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE
- ✅ Environment snapshots saved to test_logs/env/ with latest symlink

Exit criteria: arduino-cli compile succeeds for MyFirstSketch (13,396 bytes, 2% flash) and records build manifest in test_logs/env/.

Phase 1 — J-Link + RTT "hello" loop ✅ **COMPLETED**

Goal: fast visibility without serial—use RTT as your default I/O.
- ✅ J-Link CLI tools v8.62 verified working (ST-Link reflashed to J-Link)
- ✅ J-Link SWD connection to F411RE confirmed via JLinkExe
- ✅ **Upgraded Arduino_Core_STM32 RTT library**: Full SEGGER RTT v8.62 implementation (6 files)
- ✅ **J-Link upload capability**: scripts/jlink_upload.sh operational (13,828 bytes upload confirmed)
- ✅ **RTT test sketch**: MyFirstSketch.ino with Phase 1 "hello" loop + READY token operational
- ✅ **RTT connectivity verified**: JLinkRTTClient working with real-time printf output confirmed

Exit criteria: JLinkRTTClient sees READY F411RE <date> token + continuous RTT printf output. ✅ **MET**

Phase 2 — One-button build-flash-run harness ✅ **FULLY AUTOMATED & OPTIMIZED**

Goal: single command compiles, flashes (via J-Link), and tails RTT logs.
- ✅ **Build Environment Tracking**: All components locked and deterministic  
- ✅ **RTT Framework Ready**: SEGGER RTT v8.62 integrated, client connectivity confirmed
- ✅ **Build Script**: scripts/build.sh operational (2s build time, binary export)
- ✅ **Flash Script**: scripts/flash.sh unified tool with dual modes (AutoConnect, no prompts)
  - Quick mode: halt → load → reset → go (~1s, development)
  - Full mode: halt → erase → load → verify → reset → go (~9s, production)
- ✅ **RTT Logger**: scripts/rtt_cat.sh operational (timestamped capture to test_logs/rtt/)
- ✅ **Orchestration**: scripts/aflash.sh fully automated (~5s complete workflow + RTT duration)
- ✅ **Testing Documentation**: scripts/test_scripts.md (automation details, performance metrics)
- ✅ **Zero User Interaction**: All J-Link operations automated via -AutoConnect flags
- ✅ **Script Consolidation**: Eliminated redundancy (jlink_upload.sh merged into flash.sh)

Exit criteria: Complete build workflow (compile/upload/verify) confirmed operational with single command execution. ✅ **MET**
Performance: Build-flash-run cycle ~5s (quick mode), suitable for rapid development and CI/CD.

Phase 3 — Deterministic reset & ready-gate

Goal: stable test start.

- Add a tiny “ready token” macro + optional build-id line.
  - A tiny ready header on the device
  - Emits one line like: READY F411RE 1a2b3c7 2025-08-25T09:14:55Z
- scripts/await_ready.sh waits for token with timeout/backoff; surfaces clean error if absent.
  - Use build-ID injection (deterministic & cheap)
  - Stamp the build with git SHA + UTC time by generating a header build_id.h before compile.

Exit criteria: 3 consecutive runs show t_start→READY latency stats and zero flakes.

### SDFS Implementation (SPI SD Card Filesystem)

**Goal**: Implement SDFS as an SPI-based SD card filesystem with FatFs backend that provides identical interface to LittleFS for seamless storage switching in flight controller applications.

**Current Status**: Software Complete Phase 1, still working Phase 2 - Hardware debug abd verification

**Implementation Phases**:

**Phase 1: Foundation** ✅ COMPLETED
- Core SDFS class infrastructure inheriting from FS.h base class
- FatFs backend integration with SPI SD card communication
- Basic file operations (open, close, read, write) matching FS interface
- Initial test sketch with basic file create/read/write verification

**Phase 2: Filesystem Operations** ✅ COMPLETED  
- ✅ File system information methods (totalBytes, usedBytes, exists, remove)
- ✅ Directory operations (mkdir, rmdir, openNextFile, rewindDirectory) - implemented in interface
- ✅ Automated test script created for ST-Link workflow with serial capture
- ✅ Sync protocol implementation for reliable automated testing
- ✅ SPI speed configuration system (breadboard-friendly 1MHz default)

**Hardware Testing & Debugging** 🔧 ENHANCED WITH RTT FRAMEWORK
- ✅ Automated test script with sync protocol working perfectly  
- ✅ SDFS software architecture validated (compiles, initializes, mounts filesystem)
- ❌ SD card hardware communication issues identified (CMD17 failures: 0xFF/0x5 responses)
- 🎯 Root cause: Intermittent SPI communication - needs hardware troubleshooting
- ✅ **NEW: SEGGER RTT debugging framework started** - Superior testing approach

**RTT-Based Testing Framework** ✅ **OPERATIONAL**
- ✅ SEGGER RTT v8.62 library fully integrated into Arduino Core STM32 (`Arduino_Core_STM32/libraries/SEGGER_RTT/`)
- ✅ Leveraged working example code from ~/Arduino/Segger_RTT_PrintfTest_Lib_V862 (v8.62)
- ✅ RTT test sketch with READY token operational (MyFirstSketch.ino)
- ✅ **J-Link hardware configuration**: SWD connection confirmed, upload/debug operational 
- ✅ **RTT connectivity verified**: JLinkRTTClient connects and receives real-time printf output

**Phase 3: Advanced Features** ⏳ PENDING (Ready for development)
- Advanced file operations (seek, position, size, isDirectory)
- Arduino Stream/Print interface inheritance for I/O compatibility
- Enhanced error handling and status reporting throughout library

**Phase 4: Optimization & Integration** ⏳ PENDING  
- SPI communication optimization and buffering strategies
- Flight controller integration examples with real-world logging scenarios
- Comprehensive testing (stress tests, power cycle tests, LittleFS compatibility)

**Current Status**:
- **Software**: Fully functional with operational RTT debugging capability
- **Hardware**: J-Link SWD configured and operational, SD card SPI hardware pending troubleshooting
- **Test Framework**: RTT framework operational, ready for SDFS hardware debugging

**Next Actions**:
1. **J-Link debugger configured** - SWD connection operational, RTT framework ready
2. **RTT framework operational** - Real-time debugging available for immediate feedback
3. **Hardware troubleshooting**: Apply RTT framework to debug SD card SPI communication issues
4. **Proceed with Phase 3** once hardware issues resolved using enhanced RTT debugging
