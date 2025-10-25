#!/bin/bash
# Show git status for workspace and all submodules

echo "========================================="
echo "Workspace Repository Status"
echo "========================================="
git status
echo ""

echo "========================================="
echo "Arduino_Core_STM32 Submodule Status"
echo "========================================="
cd Arduino_Core_STM32
git status
git log -1 --oneline
cd ..
echo ""

echo "========================================="
echo "BoardManagerFiles Submodule Status"
echo "========================================="
cd BoardManagerFiles
git status
git log -1 --oneline
cd ..
echo ""

echo "========================================="
echo "Submodule Summary"
echo "========================================="
git submodule status
