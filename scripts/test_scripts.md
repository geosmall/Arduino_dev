# Script Testing Guide

This document provides comprehensive testing instructions for all scripts in the `/scripts` folder. These scripts implement the Build Workflow project phases for **STM32 Arduino development with J-Run + RTT + Exit Wildcard Detection** (primary) and J-Link fallback support.

## Exit Wildcard Methodology

**Key Innovation**: Tests use **exit wildcard detection** for deterministic completion instead of timeout-based approaches.

- **Default Exit Wildcard**: `"*STOP*"` 
- **How It Works**: J-Run monitors RTT output and exits automatically when the exit wildcard is detected
- **Benefits**: No timeout dependency, faster execution, deterministic test completion
- **Test Design**: Tests should emit the exit wildcard when complete, then halt execution

### Example Exit Wildcard Test Pattern

```cpp
void loop() {
    // Perform test operations
    test_counter++;
    SEGGER_RTT_printf(0, "Test step %d\r\n", test_counter);
    
    // Check completion condition
    if (test_counter >= MAX_TEST_STEPS) {
        // Emit test results
        SEGGER_RTT_WriteString(0, "=== Test Results ===\r\n");
        SEGGER_RTT_printf(0, "✓ Completed %d steps successfully\r\n", test_counter);
        SEGGER_RTT_WriteString(0, "========================\r\n");
        
        // **EXIT WILDCARD** - triggers J-Run to exit deterministically
        SEGGER_RTT_WriteString(0, "*STOP*\r\n");
        
        // Halt to prevent continued execution
        while(1) { /* visual feedback loop or halt */ }
    }
}
```

**Key Design Principles**:
1. **Emit wildcard only once** when test is truly complete
2. **Halt execution** after wildcard to prevent unwanted output
3. **Include test summary** before wildcard for verification
4. **Use deterministic conditions** (loop count, test state) not time-based

## Overview of Scripts

| Script | Purpose | Phase | Dependencies |
|--------|---------|-------|-------------|
| `env_probe.sh` | Environment verification and snapshot | Phase 0 | arduino-cli |
| `env_check_quick.sh` | **Fast environment validation (~100ms)** | **Phase 3** | **arduino-cli** |
| `build.sh` | Arduino CLI compile with optional --env-check | Phase 2-3 | arduino-cli |
| `jrun.sh` | **J-Run execution with RTT + exit wildcard detection** | **Phase 1-2** | **JRun (primary)** |
| `flash.sh` | J-Link flash with --quick/full modes (fallback) | Phase 1-2 | JLinkExe |
| `rtt_cat.sh` | RTT logging with timestamps (legacy) | Phase 2 | JLinkGDBServer, JLinkRTTClient |
| `aflash.sh` | **One-button build-jrun-test with optional --env-check** | **Phase 2-3** | **J-Run primary, JLinkExe fallback** |

## Prerequisites

Before testing, ensure you have:

1. **Hardware**: STM32 Nucleo F411RE with J-Link firmware (ST-Link reflashed)
2. **Software**: 
   - arduino-cli 1.3.0+
   - STM32 core 2.7.1+
   - **J-Link software v8.62+ (JRun primary, JLinkExe fallback)**
3. **Test Sketch**: HIL_RTT_ValidationSuite/ - HIL test framework with comprehensive validation

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

### 2. env_check_quick.sh - Fast Environment Validation

**Purpose**: **Lightning-fast environment validation** (~100ms) for integration into build workflows.

**Test Commands**:
```bash
# Silent validation (exit code only)
./scripts/env_check_quick.sh
echo "Exit code: $?"

# Verbose validation (detailed output)
./scripts/env_check_quick.sh true

# Test with broken environment (should fail)
# (temporarily rename arduino-cli to test failure)
```

**Expected Output (Verbose)**:
```
✓ Arduino CLI: 1.3.0
✓ STM32 Core: 2.7.1
ℹ ARM GCC: Not in PATH (Arduino CLI manages toolchain)
✓ FQBN: STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE
✓ Environment: All critical components validated
```

**Success Criteria**:
- ✅ Silent mode: Exit code 0 for success, 1 for failure
- ✅ Verbose mode: Detailed component validation status
- ✅ Performance: Completes in <200ms
- ✅ Locked version validation (Arduino CLI 1.3.0, STM32 Core 2.7.1)
- ✅ FQBN validity check
- ✅ Graceful handling of missing ARM GCC (non-critical)

### 3. jrun.sh - J-Run Execution with RTT (PRIMARY)

**Purpose**: **J-Run-based ELF execution with integrated RTT capture**. This is the **primary test runner** for HIL testing.

**Prerequisites**: Compiled ELF file (created by build.sh), test code that emits exit wildcard

**Test Commands**:
```bash
# Find ELF file from latest build
ELF_PATH=$(find /home/geo/.cache/arduino/sketches -name "HIL_RTT_ValidationSuite.ino.elf" | head -1)

# Basic execution with exit wildcard detection
./scripts/jrun.sh "$ELF_PATH"

# Custom device and timeout (fallback)
./scripts/jrun.sh "$ELF_PATH" STM32F411RE 60

# Custom log prefix
./scripts/jrun.sh "$ELF_PATH" STM32F411RE 60 test_run

# Custom exit wildcard  
./scripts/jrun.sh "$ELF_PATH" STM32F411RE 60 test_run "*DONE*"

# Test error handling
./scripts/jrun.sh nonexistent.elf
```

**Expected Output**:
- ELF file validation and exit wildcard confirmation
- J-Run connection and ELF download
- RTT output capture with exit wildcard detection
- **Deterministic exit when wildcard detected**

**Success Criteria**:
- ✅ ELF loads successfully via J-Run
- ✅ Device executes from ELF entry point  
- ✅ RTT connection established automatically
- ✅ **Exit wildcard detection triggers automatic completion**
- ✅ **No timeout dependency - test completes when ready**
- ✅ Output captured to timestamped log file with deterministic end

### 3. flash.sh - J-Link Flash Tool (FALLBACK)

**Purpose**: Unified J-Link flash tool with dual modes. **Fallback method** for mass operations or when ELF is unavailable.

**Prerequisites**: Compiled binary file

**Test Commands**:
```bash
# First compile a binary
arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE HIL_RTT_ValidationSuite --export-binaries

# Quick mode (development - fast upload without erase/verify)
./scripts/flash.sh --quick HIL_RTT_ValidationSuite/build/STMicroelectronics.stm32.Nucleo_64/HIL_RTT_ValidationSuite.ino.bin

# Full mode (production - erase + program + verify)
./scripts/flash.sh HIL_RTT_ValidationSuite/build/STMicroelectronics.stm32.Nucleo_64/HIL_RTT_ValidationSuite.ino.bin

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

### 4. build.sh - Arduino Build with Environment Validation

**Purpose**: Compile sketches with build caching, **ELF preservation**, binary export, and **optional environment validation**.

**Test Commands**:
```bash
# Basic build
./scripts/build.sh HIL_RTT_ValidationSuite

# Build with environment validation
./scripts/build.sh HIL_RTT_ValidationSuite --env-check

# Build with custom FQBN and environment check
./scripts/build.sh HIL_RTT_ValidationSuite STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE --env-check

# Test error handling
./scripts/build.sh nonexistent_sketch
```

**Expected Output**:
- **Environment validation section** (if --env-check used)
- Sketch name and FQBN confirmation
- Compilation progress
- Binary path and size
- **ELF file preservation in cache**
- Build time measurement

**Expected Output (with --env-check)**:
```
=== Environment Validation ===
✓ Arduino CLI: 1.3.0
✓ STM32 Core: 2.7.1
ℹ ARM GCC: Not in PATH (Arduino CLI manages toolchain)
✓ FQBN: STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE
✓ Environment: All critical components validated
✓ Environment validated

Building sketch: HIL_RTT_ValidationSuite
[...compilation output...]
```

**Success Criteria**:
- ✅ Compilation succeeds (13,396+ bytes)
- ✅ Binary file created in build/ directory
- ✅ **ELF file preserved with symbols for J-Run**
- ✅ Build time displayed
- ✅ **Environment validation completes in <200ms (when enabled)**
- ✅ Ready message with upload command

### 5. rtt_cat.sh - RTT Logger (LEGACY)

**Purpose**: Capture RTT output with timestamps and logging. **Legacy method** - J-Run has integrated RTT capture.

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

### 6. aflash.sh - One-Button Orchestration with Environment Validation (J-RUN PRIMARY)

**Purpose**: **Complete build-jrun-test workflow** in single command with **optional pre-flight environment validation**. **Automatically uses J-Run when ELF available**, falls back to JLinkExe for legacy support.

**Test Commands**:
```bash
# Full workflow test
./scripts/aflash.sh HIL_RTT_ValidationSuite

# Full workflow with pre-flight environment validation
./scripts/aflash.sh HIL_RTT_ValidationSuite --env-check

# Custom parameters with environment check
./scripts/aflash.sh HIL_RTT_ValidationSuite STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE 60 "*STOP*" --env-check

# Quick test with custom exit wildcard
./scripts/aflash.sh HIL_RTT_ValidationSuite "" 60 "*DONE*"
```

**Expected Output (J-Run Path)**:
- **Pre-flight environment check** (if --env-check used)
- Step 1/3: Build progress and success
- **Step 2/2: J-Run execution with integrated RTT capture**
- Summary with J-Run completion status

**Expected Output (with --env-check)**:
```
=== Pre-Flight Environment Check ===
✓ Arduino CLI: 1.3.0
✓ STM32 Core: 2.7.1
ℹ ARM GCC: Not in PATH (Arduino CLI manages toolchain)
✓ FQBN: STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE
✓ Environment: All critical components validated
✓ Environment validated for HIL testing

=== One-Button Build-JRun-Test: HIL_RTT_ValidationSuite ===
Environment check: ✓ Enabled
[...workflow continues...]
```

**Expected Output (Legacy Path)**:
- Step 1/3: Build progress and success  
- Step 2/3: Flash progress and success
- Step 3/3: RTT capture with timestamped output
- Summary with file locations

**Success Criteria**:
- ✅ **Automatically selects J-Run when ELF available**
- ✅ **Pre-flight environment validation completes in <200ms (when enabled)**
- ✅ Build step completes successfully
- ✅ **J-Run loads ELF and executes with RTT**
- ✅ Performance suitable for HIL testing (~5s + capture time)
- ✅ Fallback to JLinkExe when needed

## System Integration Testing

### Environment Validation Integration Test

**Purpose**: Verify environment validation works correctly across all workflows.

```bash
# Test standalone environment validation
./scripts/env_check_quick.sh
./scripts/env_check_quick.sh true

# Test build integration
./scripts/build.sh HIL_RTT_ValidationSuite --env-check

# Test full HIL workflow integration
./scripts/aflash.sh HIL_RTT_ValidationSuite --env-check

# Test failure cases (temporarily break environment)
# Example: rename arduino-cli temporarily to test error handling
```

**Validation Scenarios**:
1. **Healthy Environment**: All components validated, workflows succeed
2. **Missing Arduino CLI**: Should fail fast with clear error message
3. **Wrong STM32 Core Version**: Should warn but may continue (non-critical)
4. **Invalid FQBN**: Should fail with actionable error message

**Success Criteria**:
- ✅ Silent mode works for automation (exit codes)
- ✅ Verbose mode provides debugging information
- ✅ Integration adds <200ms overhead to workflows
- ✅ Fail-fast behavior prevents wasted build time
- ✅ Clear error messages guide troubleshooting

### Complete Workflow Test
```bash
# Full end-to-end test with environment validation
./scripts/env_probe.sh
./scripts/aflash.sh HIL_RTT_ValidationSuite --env-check

# Verify all outputs
ls -la test_logs/env/
ls -la test_logs/rtt/
cat test_logs/rtt/latest_jrun.txt
```

### Performance Benchmarks
```bash
# Time the complete workflow
time ./scripts/aflash.sh MyFirstSketch

# Individual step timing
time ./scripts/build.sh MyFirstSketch
time ./scripts/flash.sh MyFirstSketch/build/STMicroelectronics.stm32.Nucleo_64/MyFirstSketch.ino.bin
```

**Expected Performance (J-Run with Exit Wildcard)**:
- Build: ~2 seconds
- **J-Run execution: ~3 seconds (ELF load + RTT setup)**
- **Test execution: Variable (depends on test logic, not timeout)**
- **Total workflow: ~5 seconds + actual test duration (deterministic)**

**Expected Performance (Legacy)**:
- Build: ~2 seconds
- Flash (quick mode): ~1 second  
- RTT Connection: <2 seconds
- Total workflow: ~5 seconds + RTT duration

## Automation Details

**J-Run (Primary Method)** - **Fully automated HIL testing**:
- `JRun --device STM32F411RE --if SWD --speed 4000 --rtt --pc vec --sp vec <elf_file>`
- **ELF-based execution with integrated RTT capture**
- **No separate flash/RTT steps required**
- **Automatic PC/SP initialization from vector table**

**JLinkExe (Fallback Method)** - Mass operations:
- `JLinkExe -AutoConnect 1 -Device STM32F411RE -If SWD -Speed 4000 -CommandFile <script>`
- Eliminates interactive prompts for device selection
- Uses pre-configured device and interface settings
- Command files provide reproducible operation sequences

**Performance Metrics** (J-Run Primary with Exit Wildcard):
- Build: ~2 seconds
- **J-Run ELF load + execute: ~3 seconds**
- **J-Run integrated RTT: <1 second to establish**
- **Test execution: Deterministic (based on test logic, not timeout)**
- **Complete Workflow: ~5 seconds + actual test duration (no timeout overhead)**

**Performance Metrics** (JLinkExe Fallback):
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
- **J-Run Logs**: `test_logs/rtt/<prefix>_YYYYMMDD_HHMMSS.txt` (primary)
- **RTT Logs**: `test_logs/rtt/<prefix>_YYYYMMDD_HHMMSS_UTC.txt` (legacy)
- **Latest Links**: 
  - `test_logs/rtt/latest_jrun.txt` → most recent J-Run log
  - `test_logs/rtt/latest_rtt.txt` → most recent RTT log
  - `test_logs/env/latest_probe.txt` → most recent env probe

## Success Matrix

Use this checklist to verify complete script functionality:

| Script | Basic Run | Error Handling | Performance | Logging |
|--------|-----------|----------------|-------------|---------|
| env_probe.sh | ☐ | ☐ | ☐ | ☐ |
| **env_check_quick.sh** | **☐** | **☐** | **☐** | **N/A** |
| build.sh | ☐ | ☐ | ☐ | N/A |
| build.sh (--env-check) | ☐ | ☐ | ☐ | N/A |
| **jrun.sh (primary)** | **☐** | **☐** | **☐** | **☐** |
| flash.sh (--quick) | ☐ | ☐ | ☐ | N/A |
| flash.sh (full) | ☐ | ☐ | ☐ | N/A |
| rtt_cat.sh (legacy) | ☐ | ☐ | ☐ | ☐ |
| **aflash.sh (j-run)** | **☐** | **☐** | **☐** | **☐** |
| **aflash.sh (--env-check)** | **☐** | **☐** | **☐** | **☐** |

**Environment Validation Integration**: ☐
**Full System Integration**: ☐

All checkboxes should be completed for full Build Workflow Phase 2-3 verification.

## Next Steps

After successful testing:
1. **Phase 4**: Implement build-ID injection and ready-gate system (deterministic reset)
2. **Enhancement**: Add script options and configuration files
3. **Integration**: Use scripts for SDFS development and other projects
4. **CI/CD**: Leverage environment validation for automated testing workflows

---
*This testing guide ensures robust verification of the Build Workflow implementation for STM32 Arduino development with J-Link and RTT.*