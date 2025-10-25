# CLAUDE.md - Arduino Development Workspace

This file provides guidance to Claude Code (claude.ai/code) when working in this development workspace.

## Workspace Overview

This is a **Claude development workspace** that manages two submodules:

1. **Arduino_Core_STM32** - STM32 Arduino core with robotics libraries and CI/CD infrastructure
2. **BoardManagerFiles** - Arduino IDE Board Manager package distribution

### Workspace Purpose

- **Single entry point** for Claude Code sessions
- **Submodule management** for both development and distribution repositories
- **Helper scripts** for streamlined workflow
- **Clean separation** between core development and package distribution

---

## Repository Structure

```
Arduino/ (Workspace Root)
├── Arduino_Core_STM32/       # Submodule: Core development
│   ├── cores/arduino/        # Arduino core implementation
│   ├── libraries/            # Robotics libraries
│   ├── system/ci/            # CI/CD build scripts
│   ├── tests/                # Unit tests
│   ├── targets/              # Board configurations
│   ├── CLAUDE.md             # Detailed development guide
│   └── README.md             # Core documentation
│
├── BoardManagerFiles/        # Submodule: Package distribution
│   ├── package_stm32_robotics_index.json  # Board Manager index
│   └── README.md             # Installation instructions
│
└── [Workspace Files]
    ├── .claude/              # Claude Code configuration
    ├── CLAUDE.md             # This file (workspace guide)
    ├── README.md             # Workspace documentation
    ├── .gitmodules           # Submodule configuration
    ├── update-submodules.sh  # Helper: Update submodules
    ├── sync-workspace.sh     # Helper: Sync workspace
    └── status-all.sh         # Helper: Show all git status
```

---

## Working with Submodules

### Navigation
```bash
# Core development
cd Arduino_Core_STM32

# Package distribution
cd BoardManagerFiles

# Return to workspace root
cd /home/geo/Arduino
```

### Getting Context

When working on a task, reference the appropriate CLAUDE.md:

- **Workspace tasks** (this file): Submodule management, helper scripts
- **Core development** (`Arduino_Core_STM32/CLAUDE.md`): Build systems, libraries, testing, hardware validation
- **Distribution** (`BoardManagerFiles/README.md`): Package index, release management

### Common Patterns

#### Read from Submodules
```python
# Read a library file
Read("Arduino_Core_STM32/libraries/SerialRx/src/SerialRx.h")

# Read core file
Read("Arduino_Core_STM32/cores/arduino/ci_log.h")

# Read board configuration
Read("Arduino_Core_STM32/targets/NUCLEO_F411RE_LITTLEFS.h")
```

#### Edit in Submodules
```python
# Edit library code
Edit("Arduino_Core_STM32/libraries/IMU/src/IMU.cpp", ...)

# Update package index
Edit("BoardManagerFiles/package_stm32_robotics_index.json", ...)
```

#### Build and Test
```bash
# Navigate to submodule first
cd Arduino_Core_STM32

# Run build scripts (located in system/ci/)
./system/ci/build.sh tests/LittleFS_Unit_Tests --use-rtt --build-id
./system/ci/aflash.sh tests/SDFS_Unit_Tests --use-rtt --build-id

# Return to workspace
cd ..
```

---

## Quick Reference

### Workspace Helper Scripts

**update-submodules.sh** - Update both submodules to latest commits
```bash
./update-submodules.sh
# Updates Arduino_Core_STM32 (ardu_ci) and BoardManagerFiles (main)
```

**sync-workspace.sh** - Full workspace sync
```bash
./sync-workspace.sh
# Pulls workspace + initializes/updates submodules
```

**status-all.sh** - Show git status for everything
```bash
./status-all.sh
# Displays: workspace status, Arduino_Core_STM32 status, BoardManagerFiles status
```

### Typical Workflows

#### Feature Development in Arduino_Core_STM32
1. Navigate: `cd Arduino_Core_STM32`
2. Create branch: `git checkout -b feature/my-feature`
3. Make changes and commit
4. Push: `git push origin feature/my-feature`
5. Return: `cd ..`
6. Update workspace pointer: `git add Arduino_Core_STM32 && git commit -m "Update submodule"`

#### Release Management
1. Develop in Arduino_Core_STM32 (create libraries, fix bugs, etc.)
2. Tag release: `cd Arduino_Core_STM32 && git tag v1.x.x`
3. Create release archive (see BoardManagerFiles for release process)
4. Update package index: `cd BoardManagerFiles && edit package_stm32_robotics_index.json`
5. Commit and push both submodules

---

## Important Notes for Claude

### Submodule Awareness
- Always check which repository you're working in (`pwd`)
- Changes in submodules require commits in BOTH the submodule AND workspace
- Submodule pointers in workspace track specific commits

### File References
When asked to work on files, determine the correct location:
- **Build scripts**: `Arduino_Core_STM32/system/ci/`
- **Libraries**: `Arduino_Core_STM32/libraries/`
- **Tests**: `Arduino_Core_STM32/tests/`
- **Board configs**: `Arduino_Core_STM32/targets/`
- **Package index**: `BoardManagerFiles/package_stm32_robotics_index.json`

### Context Loading
For detailed information about:
- **Build systems**: Read `Arduino_Core_STM32/CLAUDE.md` (sections: Build Systems, J-Link, RTT)
- **Libraries**: Read `Arduino_Core_STM32/CLAUDE.md` (sections: Key Libraries, Completed Projects)
- **Testing**: Read `Arduino_Core_STM32/CLAUDE.md` (section: Embedded Hardware Validation Standards)
- **Board Manager**: Read `BoardManagerFiles/README.md`

---

## Development Standards

### Git Workflow

**Submodule Changes:**
1. Commit in submodule first
2. Push submodule
3. Update workspace submodule pointer
4. Commit workspace

**Never:**
- Commit workspace before pushing submodule
- Leave submodule pointer uncommitted after submodule changes

### Documentation
- **Workspace-level changes**: Update this CLAUDE.md or README.md
- **Core/library changes**: Update `Arduino_Core_STM32/CLAUDE.md` or `Arduino_Core_STM32/README.md`
- **Package changes**: Update `BoardManagerFiles/README.md`

---

## Key Submodule Details

### Arduino_Core_STM32
- **Branch**: `ardu_ci`
- **Remote**: https://github.com/geosmall/Arduino_Core_STM32
- **Focus**: STM32 core development, robotics libraries, CI/CD
- **Build System**: arduino-cli, Makefile, CMake, custom scripts
- **Primary Hardware**: NUCLEO_F411RE, BlackPill F411CE
- **Testing**: HIL framework with SEGGER RTT + J-Link

### BoardManagerFiles
- **Branch**: `main`
- **Remote**: https://github.com/geosmall/BoardManagerFiles
- **Focus**: Arduino IDE Board Manager package distribution
- **Update Frequency**: Only on releases
- **Key File**: `package_stm32_robotics_index.json`

---

## Troubleshooting

### Submodule Not Initialized
```bash
git submodule update --init --recursive
```

### Submodule Out of Sync
```bash
./sync-workspace.sh
```

### Check All Status
```bash
./status-all.sh
```

### Detached HEAD in Submodule
```bash
cd Arduino_Core_STM32
git checkout ardu_ci
cd ..
```

---

## Additional Resources

For comprehensive development information:
- **Arduino_Core_STM32/CLAUDE.md** - Complete development guide (35KB, 800+ lines)
  - Build systems and commands
  - Architecture overview
  - Libraries and examples
  - Hardware validation standards
  - Completed projects
  - Future roadmap

- **README.md** (this workspace) - Workspace usage guide
  - Submodule management
  - Helper scripts
  - Claude Code collaboration workflows

---

## Workspace-Specific Instructions

### Clean Repository Policy
The workspace itself should remain minimal:
- Only submodule pointers, helper scripts, and documentation
- No build artifacts
- No temporary files
- Clean git status before commits

### Commit Message Override
- **Workspace commits**: Clear, technical messages describing submodule updates
- **Submodule commits**: Follow individual submodule conventions (see Arduino_Core_STM32/CLAUDE.md)

### Todo Management
- Use TodoWrite for multi-step tasks
- Keep todos focused on current workspace-level work
- For submodule-specific work, work within that submodule context
