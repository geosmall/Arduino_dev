#!/bin/bash
# Sync workspace: pull latest from parent repo and update submodules

set -e

echo "=== Syncing workspace repository ==="
git pull origin dev

echo ""
echo "=== Initializing/updating submodules ==="
git submodule update --init --recursive

echo ""
echo "=== Updating Arduino_Core_STM32 to latest ==="
cd Arduino_Core_STM32
git checkout ardu_ci
git pull origin ardu_ci
cd ..

echo ""
echo "=== Updating BoardManagerFiles to latest ==="
cd BoardManagerFiles
git checkout main
git pull origin main
cd ..

echo ""
echo "✅ Workspace fully synced"
echo ""
echo "Current status:"
git status --short
echo ""
echo "Submodule status:"
git submodule status
