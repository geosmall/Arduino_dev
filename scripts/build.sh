#!/bin/bash
#
# Arduino CLI build script with caching
# Usage: ./scripts/build.sh <sketch_directory> [FQBN]
#

set -euo pipefail

# Default configuration
DEFAULT_FQBN="STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE"

# Check arguments
if [ $# -lt 1 ]; then
    echo "Usage: $0 <sketch_directory> [FQBN]"
    echo "Example: $0 MyFirstSketch"
    echo "Example: $0 MyFirstSketch STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE"
    exit 1
fi

SKETCH_DIR="$1"
FQBN="${2:-$DEFAULT_FQBN}"

# Validate sketch directory
if [ ! -d "$SKETCH_DIR" ]; then
    echo "Error: Sketch directory not found: $SKETCH_DIR"
    exit 1
fi

# Find the .ino file
INO_FILE=$(find "$SKETCH_DIR" -name "*.ino" | head -1)
if [ -z "$INO_FILE" ]; then
    echo "Error: No .ino file found in $SKETCH_DIR"
    exit 1
fi

SKETCH_NAME=$(basename "$INO_FILE" .ino)
echo "Building sketch: $SKETCH_NAME"
echo "FQBN: $FQBN"
echo "Directory: $SKETCH_DIR"

# Build with export binaries for J-Link upload
echo "Starting arduino-cli compile..."
START_TIME=$(date +%s)

arduino-cli compile \
    --fqbn "$FQBN" \
    --export-binaries \
    --build-properties "compiler.cpp.extra_flags=-DBUILD_TIMESTAMP=$(date +%s)" \
    "$SKETCH_DIR"

END_TIME=$(date +%s)
BUILD_TIME=$((END_TIME - START_TIME))

# Find the generated binary
BINARY_PATH=$(find "$SKETCH_DIR/build" -name "*.bin" | head -1)
if [ -z "$BINARY_PATH" ]; then
    echo "Error: No binary file found after build"
    exit 1
fi

BINARY_SIZE=$(stat -c%s "$BINARY_PATH")
echo
echo "✓ Build successful:"
echo "  Binary: $BINARY_PATH"
echo "  Size: $BINARY_SIZE bytes"
echo "  Build time: ${BUILD_TIME}s"
echo
echo "Ready for upload with: ./scripts/flash.sh --quick $BINARY_PATH"