# Script Testing Guide

This document provides comprehensive testing instructions for all scripts in the `/scripts` folder. These scripts implement the Build Workflow project phases for STM32 Arduino development with J-Link and RTT.

## Overview of Scripts

| Script | Purpose | Phase | Dependencies |
|--------|---------|-------|-------------|
| `env_probe.sh` | Environment verification and snapshot | Phase 0 | arduino-cli |
| `build.sh` | Arduino CLI compile with caching | Phase 2 | arduino-cli |
| `flash.sh` | J-Link flash with --quick/full modes | Phase 1-2 | JLinkExe |
| `rtt_cat.sh` | RTT logging with timestamps | Phase 2 | JLinkGDBServer, JLinkRTTClient |
| `aflash.sh` | One-button build-flash-run orchestration | Phase 2 | All above |

## Prerequisites

Before testing, ensure you have:

1. **Hardware**: STM32 Nucleo F411RE with J-Link firmware (ST-Link reflashed)
2. **Software**: 
   - arduino-cli 1.3.0+
   - STM32 core 2.7.1+
   - J-Link software v8.62+ (JLinkExe, JLinkGDBServer, JLinkRTTClient)
3. **Test Sketch**: MyFirstSketch/ with RTT support

## Quick Verification

Run this command to verify all scripts are executable:
```bash
ls -la scripts/
```
All scripts should show `-rwxrwxr-x` permissions.

## Individual Script Testing

### 1. env_probe.sh - Environment Probe

**Purpose**: Captures build environment state and validates toolchain.

**Test Commands**:
```bash
# Basic probe
./scripts/env_probe.sh

# Verify log creation
ls -la test_logs/env/
cat test_logs/env/latest_probe.txt
```

**Expected Output**:
- Arduino CLI version 1.3.0+
- STM32 core 2.7.1 installed
- FQBN validation success
- Hardware detection (/dev/ttyACM0)
- Log saved to timestamped file

**Success Criteria**:
- ✅ No error messages
- ✅ FQBN valid checkmark
- ✅ Log file created in test_logs/env/
- ✅ Latest symlink updated

### 2. flash.sh - J-Link Flash Tool

**Purpose**: Unified J-Link flash tool with dual modes. **FULLY AUTOMATED** - no user interaction required.

**Prerequisites**: Compiled binary file

**Test Commands**:
```bash
# First compile a binary
arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE MyFirstSketch --export-binaries

# Quick mode (development - fast upload without erase/verify)
./scripts/flash.sh --quick MyFirstSketch/build/STMicroelectronics.stm32.Nucleo_64/MyFirstSketch.ino.bin

# Full mode (production - erase + program + verify)
./scripts/flash.sh MyFirstSketch/build/STMicroelectronics.stm32.Nucleo_64/MyFirstSketch.ino.bin

# Test error handling
./scripts/flash.sh nonexistent.bin
```

**Expected Output**:
- **Quick Mode**: "halt → load → reset → go" (~1 second)
- **Full Mode**: "halt → erase → load → verify → reset → go" (~9 seconds)
- J-Link auto-connection via USB (no prompts)
- Success confirmation with timing

**Success Criteria**:
- ✅ Both modes complete without user prompts
- ✅ Quick mode: <2 seconds, suitable for development iteration
- ✅ Full mode: ~9 seconds with verification, production-ready
- ✅ Device resets and runs immediately after flash
- ✅ Error handling for missing files

### 3. build.sh - Arduino Build

**Purpose**: Compile sketches with build caching and binary export.

**Test Commands**:
```bash
# Basic build
./scripts/build.sh MyFirstSketch

# Build with custom FQBN
./scripts/build.sh MyFirstSketch STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE

# Test error handling
./scripts/build.sh nonexistent_sketch
```

**Expected Output**:
- Sketch name and FQBN confirmation
- Compilation progress
- Binary path and size
- Build time measurement

**Success Criteria**:
- ✅ Compilation succeeds (13,396+ bytes)
- ✅ Binary file created in build/ directory
- ✅ Build time displayed
- ✅ Ready message with upload command

### 4. rtt_cat.sh - RTT Logger

**Purpose**: Capture RTT output with timestamps and logging.

**Prerequisites**: Device running with RTT-enabled sketch

**Test Commands**:
```bash
# Basic RTT capture (30s default)
./scripts/rtt_cat.sh

# Custom duration and prefix
./scripts/rtt_cat.sh 10 test_run

# Quick test (5 seconds)
./scripts/rtt_cat.sh 5
```

**Expected Output**:
- GDB server status
- RTT connection established
- Timestamped device output
- Log file creation with symlink

**Success Criteria**:
- ✅ RTT connection established
- ✅ Device output visible (READY token, loop counters)
- ✅ Timestamps on each line
- ✅ Log file created in test_logs/rtt/
- ✅ latest_rtt.txt symlink updated

### 5. aflash.sh - One-Button Orchestration

**Purpose**: Complete build-flash-run workflow in single command.

**Test Commands**:
```bash
# Full workflow test
./scripts/aflash.sh MyFirstSketch

# Custom parameters
./scripts/aflash.sh MyFirstSketch STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE 20

# Quick test
./scripts/aflash.sh MyFirstSketch "" 5
```

**Expected Output**:
- Step 1/3: Build progress and success
- Step 2/3: Flash progress and success
- Step 3/3: RTT capture with timestamped output
- Summary with file locations

**Success Criteria**:
- ✅ All three steps complete successfully
- ✅ Device runs and outputs RTT data
- ✅ Build, flash, and RTT logs created
- ✅ Performance metrics displayed

## System Integration Testing

### Complete Workflow Test
```bash
# Full end-to-end test
./scripts/env_probe.sh
./scripts/aflash.sh MyFirstSketch

# Verify all outputs
ls -la test_logs/env/
ls -la test_logs/rtt/
cat test_logs/rtt/latest_rtt.txt
```

### Performance Benchmarks
```bash
# Time the complete workflow
time ./scripts/aflash.sh MyFirstSketch

# Individual step timing
time ./scripts/build.sh MyFirstSketch
time ./scripts/flash.sh MyFirstSketch/build/STMicroelectronics.stm32.Nucleo_64/MyFirstSketch.ino.bin
```

**Expected Performance**:
- Build: ~2 seconds
- Flash (quick mode): ~1 second  
- RTT Connection: <2 seconds
- Total workflow: ~5 seconds + RTT duration

## Automation Details

All J-Link scripts use **fully automated operation** via:
- `JLinkExe -AutoConnect 1 -Device STM32F411RE -If SWD -Speed 4000 -CommandFile <script>`
- Eliminates interactive prompts for device selection
- Uses pre-configured device and interface settings
- Command files provide reproducible operation sequences

**Performance Metrics** (Automated Tests):
- Build: ~2 seconds
- J-Link Flash --quick: <1 second (73-76 KB/s) 
- J-Link Flash (full): ~9 seconds (includes full chip erase + verify)
- RTT Connection: <2 seconds
- Complete Workflow: ~5 seconds + RTT duration (using --quick mode)

## Troubleshooting

### Common Issues

**J-Link Connection Failed**:
```bash
# Check USB connection
lsusb | grep -i segger

# Verify J-Link tools and automation
JLinkExe -AutoConnect 1 -Device STM32F411RE -If SWD -Speed 4000 -CommandFile <(echo "exit")
```

**RTT No Output**:
```bash
# Kill existing GDB servers
pkill -f JLinkGDBServer

# Restart fresh RTT session
./scripts/rtt_cat.sh 5
```

**Build Failures**:
```bash
# Verify environment
./scripts/env_probe.sh

# Check arduino-cli
arduino-cli core list
arduino-cli board list
```

**Permission Errors**:
```bash
# Fix script permissions
chmod +x scripts/*.sh
```

**Interactive Prompts (Should Not Occur)**:
If you see device selection prompts, verify:
- Scripts use `-AutoConnect 1` flag
- Device specified as `-Device STM32F411RE`
- Command files exist and are readable

## Log Files

All scripts create logs in structured directories:

- **Environment**: `test_logs/env/env_probe_YYYYMMDD_HHMMSS_UTC.txt`
- **RTT Logs**: `test_logs/rtt/<prefix>_YYYYMMDD_HHMMSS_UTC.txt`
- **Latest Links**: `test_logs/*/latest_*.txt` (symlinks to most recent)

## Success Matrix

Use this checklist to verify complete script functionality:

| Script | Basic Run | Error Handling | Performance | Logging |
|--------|-----------|----------------|-------------|---------|
| env_probe.sh | ☐ | ☐ | ☐ | ☐ |
| build.sh | ☐ | ☐ | ☐ | N/A |
| flash.sh (--quick) | ☐ | ☐ | ☐ | N/A |
| flash.sh (full) | ☐ | ☐ | ☐ | N/A |
| rtt_cat.sh | ☐ | ☐ | ☐ | ☐ |
| aflash.sh | ☐ | ☐ | ☐ | ☐ |

**Full System Integration**: ☐

All checkboxes should be completed for full Build Workflow Phase 2 verification.

## Next Steps

After successful testing:
1. **Phase 3**: Implement build-ID injection and ready-gate system
2. **Enhancement**: Add script options and configuration files
3. **Integration**: Use scripts for SDFS development and other projects

---
*This testing guide ensures robust verification of the Build Workflow implementation for STM32 Arduino development with J-Link and RTT.*