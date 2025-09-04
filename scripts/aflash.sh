#!/bin/bash
#
# One-button build-flash-run harness
# Orchestrates: build → flash → RTT capture
# Usage: ./scripts/aflash.sh <sketch_directory> [FQBN] [rtt_duration]
#

set -euo pipefail

# Default configuration
DEFAULT_FQBN="STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE"
DEFAULT_TIMEOUT=60
DEFAULT_EXIT_WILDCARD="*STOP*"

# Check arguments
if [ $# -lt 1 ]; then
    echo "One-Button Build-JRun-Test Harness"
    echo "Usage: $0 <sketch_directory> [FQBN] [timeout_seconds] [exit_wildcard]"
    echo
    echo "Examples:"
    echo "  $0 MyFirstSketch"
    echo "  $0 MyFirstSketch STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE"
    echo "  $0 MyFirstSketch STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE 60"
    echo "  $0 MyFirstSketch \"\" 60 \"*DONE*\""
    echo
    echo "Default FQBN: $DEFAULT_FQBN"
    echo "Default timeout: ${DEFAULT_TIMEOUT}s (fallback for exit wildcard)"
    echo "Default exit wildcard: \"$DEFAULT_EXIT_WILDCARD\""
    echo
    echo "Note: Tests should emit exit wildcard for deterministic completion"
    exit 1
fi

SKETCH_DIR="$1"
FQBN="${2:-$DEFAULT_FQBN}"
TIMEOUT="${3:-$DEFAULT_TIMEOUT}"
EXIT_WILDCARD="${4:-$DEFAULT_EXIT_WILDCARD}"

# Validate sketch directory
if [ ! -d "$SKETCH_DIR" ]; then
    echo "Error: Sketch directory not found: $SKETCH_DIR"
    exit 1
fi

SKETCH_NAME=$(basename "$SKETCH_DIR")
echo "=== One-Button Build-JRun-Test: $SKETCH_NAME ==="
echo "FQBN: $FQBN"
echo "Timeout: ${TIMEOUT}s (fallback)"
echo "Exit wildcard: \"$EXIT_WILDCARD\""
echo

# Step 1: Build
echo "Step 1/3: Building..."
if ! ./scripts/build.sh "$SKETCH_DIR" "$FQBN"; then
    echo "✗ Build failed"
    exit 1
fi
echo

# Find the ELF file (preferred for J-Run) or fall back to binary
ELF_PATH=$(find /home/geo/.cache/arduino/sketches -name "${SKETCH_NAME}.ino.elf" | head -1)
if [ -n "$ELF_PATH" ]; then
    echo "✓ Found ELF file: $(basename "$ELF_PATH")"
    EXEC_METHOD="jrun"
else
    # Fallback to binary + JLinkExe flash method
    BINARY_PATH=$(find "$SKETCH_DIR/build" -name "*.bin" | head -1)
    if [ -z "$BINARY_PATH" ]; then
        echo "✗ No ELF or binary file found after build"
        exit 1
    fi
    echo "✓ Found binary file: $(basename "$BINARY_PATH")"
    EXEC_METHOD="flash"
fi

if [ "$EXEC_METHOD" = "jrun" ]; then
    # Step 2: J-Run HIL test execution with exit wildcard detection
    echo "Step 2/2: Executing HIL test with J-Run (exit wildcard detection)..."
    LOG_PREFIX="${SKETCH_NAME}_aflash"
    if ! ./scripts/jrun.sh "$ELF_PATH" STM32F411RE "$TIMEOUT" "$LOG_PREFIX" "$EXIT_WILDCARD"; then
        echo "✗ J-Run HIL test execution failed"
        exit 1
    fi
else
    # Legacy: Step 2: Flash + Step 3: RTT Capture
    echo "Step 2/3: Flashing (legacy)..."
    if ! ./scripts/flash.sh --quick "$BINARY_PATH"; then
        echo "✗ Flash failed"
        exit 1
    fi
    echo
    
    # Brief pause to let device start
    sleep 2
    
    # Step 3: RTT Capture
    echo "Step 3/3: Capturing RTT output..."
    LOG_PREFIX="${SKETCH_NAME}_aflash"
    ./scripts/rtt_cat.sh "$TIMEOUT" "$LOG_PREFIX"
fi

echo
if [ "$EXEC_METHOD" = "jrun" ]; then
    echo "=== Build-JRun-HIL-Test Complete ==="
    echo "✓ Build: $(basename "$ELF_PATH") (ELF with symbols)"
    echo "✓ J-Run: ELF loaded and executed with exit wildcard detection"
    
    # Check if test completed deterministically
    if grep -q "$EXIT_WILDCARD" test_logs/rtt/latest_jrun.txt 2>/dev/null; then
        echo "✓ HIL Test: Completed deterministically (exit wildcard detected)"
        echo "✓ Test Result: SUCCESS - deterministic test execution"
    else
        echo "⚠ HIL Test: May be incomplete (exit wildcard not detected)"
        echo "⚠ Test Result: Check test implementation for exit wildcard emission"
    fi
    echo
    echo "To view J-Run HIL test output: cat test_logs/rtt/latest_jrun.txt"
else
    echo "=== Build-Flash-Run Complete (Legacy) ==="
    echo "✓ Build: $(basename "$BINARY_PATH") ($(stat -c%s "$BINARY_PATH") bytes)"
    echo "✓ Flash: Device programmed and running"
    echo "✓ RTT: Log captured (timeout: ${TIMEOUT}s)"
    echo
    echo "To view RTT output: cat test_logs/rtt/latest_rtt.txt"
    echo "To run RTT live: ./scripts/rtt_cat.sh [timeout] [prefix]"
fi