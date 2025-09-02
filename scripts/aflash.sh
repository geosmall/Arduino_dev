#!/bin/bash
#
# One-button build-flash-run harness
# Orchestrates: build → flash → RTT capture
# Usage: ./scripts/aflash.sh <sketch_directory> [FQBN] [rtt_duration]
#

set -euo pipefail

# Default configuration
DEFAULT_FQBN="STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE"
DEFAULT_RTT_DURATION=15

# Check arguments
if [ $# -lt 1 ]; then
    echo "One-Button Build-Flash-Run Harness"
    echo "Usage: $0 <sketch_directory> [FQBN] [rtt_duration_seconds]"
    echo
    echo "Examples:"
    echo "  $0 MyFirstSketch"
    echo "  $0 MyFirstSketch STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE"
    echo "  $0 MyFirstSketch STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE 30"
    echo
    echo "Default FQBN: $DEFAULT_FQBN"
    echo "Default RTT duration: ${DEFAULT_RTT_DURATION}s"
    exit 1
fi

SKETCH_DIR="$1"
FQBN="${2:-$DEFAULT_FQBN}"
RTT_DURATION="${3:-$DEFAULT_RTT_DURATION}"

# Validate sketch directory
if [ ! -d "$SKETCH_DIR" ]; then
    echo "Error: Sketch directory not found: $SKETCH_DIR"
    exit 1
fi

SKETCH_NAME=$(basename "$SKETCH_DIR")
echo "=== One-Button Build-Flash-Run: $SKETCH_NAME ==="
echo "FQBN: $FQBN"
echo "RTT Duration: ${RTT_DURATION}s"
echo

# Step 1: Build
echo "Step 1/3: Building..."
if ! ./scripts/build.sh "$SKETCH_DIR" "$FQBN"; then
    echo "✗ Build failed"
    exit 1
fi
echo

# Find the binary file
BINARY_PATH=$(find "$SKETCH_DIR/build" -name "*.bin" | head -1)
if [ -z "$BINARY_PATH" ]; then
    echo "✗ No binary file found after build"
    exit 1
fi

# Step 2: Flash (use quick mode for fast development cycle)
echo "Step 2/3: Flashing..."
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
./scripts/rtt_cat.sh "$RTT_DURATION" "$LOG_PREFIX"

echo
echo "=== Build-Flash-Run Complete ==="
echo "✓ Build: $(basename "$BINARY_PATH") ($(stat -c%s "$BINARY_PATH") bytes)"
echo "✓ Flash: Device programmed and running"
echo "✓ RTT: Log captured (${RTT_DURATION}s)"
echo
echo "To view RTT output: cat test_logs/rtt/latest_rtt.txt"
echo "To run RTT live: ./scripts/rtt_cat.sh [duration] [prefix]"