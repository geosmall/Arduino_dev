#!/bin/bash
# Update both submodules to their latest commits on tracked branches

set -e

echo "=== Updating Arduino_Core_STM32 submodule ==="
cd Arduino_Core_STM32
git fetch origin
git pull origin ardu_ci
cd ..

echo ""
echo "=== Updating BoardManagerFiles submodule ==="
cd BoardManagerFiles
git fetch origin
git pull origin main
cd ..

echo ""
echo "✅ Both submodules updated to latest"
echo ""
echo "Submodule status:"
git submodule status
