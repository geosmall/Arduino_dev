# Arduino Development Workspace

> **Purpose**: Claude-assisted development workspace for managing [Arduino_Core_STM32](https://github.com/geosmall/Arduino_Core_STM32) and [BoardManagerFiles](https://github.com/geosmall/BoardManagerFiles) as submodules.

---

## Architecture Overview

This workspace implements a **3-tier architecture** for STM32 Arduino development:

**Workspace (Arduino)** - Orchestration Layer
- Manages both repositories as git submodules
- Provides release automation (`create_release.sh`)
- Helper scripts for cross-repo coordination
- Claude Code development environment

**Development (Arduino_Core_STM32)** - Development Layer
- Complete STM32 Arduino core (stm32duino fork)
- 11 robotics libraries (SerialRx, IMU, Storage, etc.)
- CI/CD infrastructure with HIL testing
- All active development happens here

**Distribution (BoardManagerFiles)** - Distribution Layer
- Arduino IDE Board Manager package index
- Release metadata and download URLs
- End-user installation point

This separation provides clean boundaries: workspace orchestrates, development implements, distribution delivers.

---

## Prerequisites

**Required Tools:**
- **Git** (2.13+) - Submodule support
- **Bash** - Shell scripting (standard on Linux/macOS/WSL)
- **GitHub CLI (`gh`)** - Release creation (install from https://cli.github.com, then `gh auth login`)
- **jq** - JSON manipulation (install via package manager: `apt install jq`)
- **tar** - Archive creation (standard on Linux/macOS)

**For Development in Arduino_Core_STM32:**
- See [Arduino_Core_STM32/CLAUDE.md](Arduino_Core_STM32/CLAUDE.md) for complete build environment requirements

---

## Quick Start

### Initial Clone
```bash
# Clone workspace with submodules
git clone --recurse-submodules https://github.com/geosmall/Arduino.git
cd Arduino

# Or if already cloned without submodules
git submodule update --init --recursive
```

### Daily Workflow
```bash
# Sync workspace and all submodules
./sync-workspace.sh

# Check status of workspace and all submodules
./status-all.sh

# Update submodules to latest commits
./update-submodules.sh
```

---

## Repository Structure

```
Arduino/
├── Arduino_Core_STM32/       # Submodule: STM32 Arduino core + libraries
├── BoardManagerFiles/        # Submodule: Arduino Board Manager package
├── .claude/                  # Claude Code configuration
├── CLAUDE.md                 # Development guide for Claude Code
├── DEPLOYMENT_PLAN.md        # Deployment plan and architecture
├── DEPLOYMENT_STATUS.md      # Current deployment progress
├── README.md                 # This file
├── create_release.sh         # Automated release creation
├── update-submodules.sh      # Update both submodules
├── sync-workspace.sh         # Sync workspace + submodules
└── status-all.sh             # Show git status for all repos
```

---

## Submodules

### Arduino_Core_STM32
**Repository**: https://github.com/geosmall/Arduino_Core_STM32
**Branch**: `ardu_ci`
**Purpose**: Custom STM32 Arduino core with robotics libraries and CI/CD infrastructure

**Contents**:
- STM32 Arduino core (fork of stm32duino)
- Robotics libraries (SerialRx, IMU, ICM42688P, LittleFS, SDFS, Storage, TimerPWM, etc.)
- CI/CD scripts and HIL testing framework
- Unit tests and board configurations
- Documentation and development tools

### BoardManagerFiles
**Repository**: https://github.com/geosmall/BoardManagerFiles
**Branch**: `main`
**Purpose**: Arduino IDE Board Manager package distribution

**Contents**:
- Package index JSON (`package_stm32_robotics_index.json`)
- Installation documentation
- Release management

---

## Development Workflow

### Working in Arduino_Core_STM32
```bash
cd Arduino_Core_STM32

# Create feature branch
git checkout -b feature/my-feature

# Make changes, commit
git add <files>
git commit -m "Add feature"

# Push to remote
git push origin feature/my-feature

# Return to workspace root
cd ..

# Update workspace submodule pointer (if needed)
git add Arduino_Core_STM32
git commit -m "Update Arduino_Core_STM32 submodule pointer"
```

### Working in BoardManagerFiles
```bash
cd BoardManagerFiles

# Update package index for new release
vim package_stm32_robotics_index.json

# Commit and push
git add package_stm32_robotics_index.json
git commit -m "Update package index for v1.1.0"
git push origin main

# Return to workspace
cd ..

# Update workspace submodule pointer
git add BoardManagerFiles
git commit -m "Update BoardManagerFiles submodule pointer"
```

### Keeping Submodules Up to Date
```bash
# Pull latest changes from both submodules
./update-submodules.sh

# Commit the updated submodule pointers in workspace
git add Arduino_Core_STM32 BoardManagerFiles
git commit -m "Update submodule pointers to latest"
git push origin dev
```

---

## Claude Code Collaboration

This workspace is designed for collaborative development with **Claude Code** (https://claude.com/code).

### Key Features
- **Submodule-aware**: Claude can work across both repositories
- **Helper scripts**: Simplified management of multiple repos
- **Workspace CLAUDE.md**: Development guidelines for Claude-assisted coding
- **Clean separation**: Core development vs. distribution management

### Working with Claude Code
1. **Start Claude Code session** in the workspace root (`/home/geo/Arduino`)
2. **CLAUDE.md** provides context for:
   - Project structure and architecture
   - Build systems and workflows
   - Testing and validation standards
   - Development best practices
3. **Navigate submodules** as needed:
   ```bash
   cd Arduino_Core_STM32  # Core development
   cd BoardManagerFiles    # Package distribution
   ```

### Claude Code Tips
- Reference submodule files: `Arduino_Core_STM32/libraries/SerialRx/...`
- Use helper scripts: `./status-all.sh` shows complete repository state
- Keep CLAUDE.md updated with new patterns and decisions

---

## Documentation

### Workspace Documentation

**[DEPLOYMENT_PLAN.md](DEPLOYMENT_PLAN.md)** - Architecture evolution and key decisions
- Complete history from consolidation through workspace transformation
- Phases 1-5: Consolidation → BoardManagerFiles → v1.0.0 Release → Workspace → Automation
- Key architectural decisions with rationale (why 3-tier, script placement, etc.)
- Lessons learned from deployment process

**[DEPLOYMENT_STATUS.md](DEPLOYMENT_STATUS.md)** - Current deployment progress
- Executive summary: Phases 1-5 complete ✅, Testing/Documentation pending 📋
- Phase-by-phase accomplishments and validation results
- Current workspace structure with submodule branches
- Key accomplishments: Infrastructure, Libraries, Deployment, Testing
- Next steps for Phases 6-7

**[README.md](README.md)** (this file) - Workspace quick reference
- Architecture overview and development workflows
- Helper scripts and release automation
- Quick start for new developers

### Submodule Documentation

**[Arduino_Core_STM32/CLAUDE.md](Arduino_Core_STM32/CLAUDE.md)** - Technical development guide
- Complete build systems (Arduino CLI, CMake, bash scripts)
- HIL testing framework with RTT debugging
- Hardware validation standards (critical for embedded work)
- Library architecture and completed/future projects
- Development best practices and collaboration notes

**[Arduino_Core_STM32/README.md](Arduino_Core_STM32/README.md)** - Core repository overview
- Repository purpose and features
- Installation instructions
- Basic usage examples

**[BoardManagerFiles/README.md](BoardManagerFiles/README.md)** - Installation guide
- How to install via Arduino IDE Board Manager
- Package index structure
- End-user documentation

---

## Helper Scripts

### `update-submodules.sh`
Updates both submodules to latest commits on their tracked branches.
```bash
./update-submodules.sh
```

### `sync-workspace.sh`
Complete workspace sync: pulls latest workspace changes and updates all submodules.
```bash
./sync-workspace.sh
```

### `status-all.sh`
Shows git status for workspace and all submodules in one view.
```bash
./status-all.sh
```

---

## Release Workflow

### Creating a New Release - Step by Step

**Step 1: Verify Workspace State**
```bash
./status-all.sh
# Verify: Arduino_Core_STM32 (ardu_ci), BoardManagerFiles (main), all clean
```

**Step 2: Test with Dry-Run**
```bash
./create_release.sh 1.1.0 --dry-run
```

**Step 3: Create Release**
```bash
./create_release.sh 1.1.0         # Interactive (asks confirmation)
./create_release.sh 1.1.0 --yes   # Automated (no prompt)
```

**Step 4: Update Workspace**
```bash
git add BoardManagerFiles
git commit -m "Update BoardManagerFiles submodule pointer for v1.1.0 release"
git push origin dev
```

**Step 5: Verify Release**
```bash
gh release view robo-1.1.0 --repo geosmall/Arduino_Core_STM32
```

**Step 6: Test Installation**
- Arduino IDE → Boards Manager → Search "STM32 Robotics" → Install v1.1.0

### What the Script Does

1. **Validation**
   - Checks version format (semantic versioning: X.Y.Z)
   - Verifies both repositories are clean and on correct branches
   - Checks if version tag already exists

2. **Archive Creation**
   - Cleans build artifacts from Arduino_Core_STM32
   - Creates `STM32-Robotics-{version}.tar.bz2` archive
   - Calculates SHA-256 checksum and file size

3. **GitHub Release**
   - Tags Arduino_Core_STM32 with `robo-{version}`
   - Creates GitHub release with auto-generated notes
   - Uploads archive as release asset

4. **Package Index Update**
   - Updates `package_stm32_robotics_index.json` with new version
   - Adds download URL, checksum, and file size
   - Maintains version history (newest first)

5. **Publish**
   - Commits package index changes to BoardManagerFiles
   - Pushes tag to Arduino_Core_STM32
   - Shows installation URLs and next steps

### Prerequisites

- Both repositories must be clean (no uncommitted changes)
- Arduino_Core_STM32 must be on `ardu_ci` branch
- BoardManagerFiles must be on `main` branch
- GitHub CLI (`gh`) must be authenticated

### Release Checklist

Before creating a release:
- [ ] All changes committed to Arduino_Core_STM32
- [ ] All tests passing on hardware
- [ ] CHANGELOG or release notes prepared
- [ ] Version number decided (follow semantic versioning)

After creating a release:
- [ ] Test Board Manager installation in Arduino IDE
- [ ] Verify example compilation from installed package
- [ ] Update DEPLOYMENT_STATUS.md
- [ ] Announce release (if applicable)

### Troubleshooting

**Tag already exists:**
```bash
gh release delete robo-X.Y.Z --repo geosmall/Arduino_Core_STM32 --yes
cd Arduino_Core_STM32 && git push origin --delete robo-X.Y.Z && git tag -d robo-X.Y.Z
```

**Uncommitted changes:**
```bash
./status-all.sh  # Identify which repo has changes
cd <repo> && git status  # Review and commit changes
```

**Authentication required:**
```bash
gh auth status  # Check authentication
gh auth login   # Re-authenticate if needed
```

---

## Installing the Arduino Core

End users can install the STM32 Robotics Core via Arduino IDE Board Manager:

1. Open **Arduino IDE**
2. Go to **File → Preferences**
3. Add Board Manager URL:
   ```
   https://github.com/geosmall/BoardManagerFiles/raw/main/package_stm32_robotics_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for **"STM32 Robotics"**
6. Click **Install**

---

## Development vs. Distribution

| Aspect | Arduino_Core_STM32 | BoardManagerFiles |
|--------|-------------------|-------------------|
| Purpose | Development | Distribution |
| Branch | `ardu_ci` | `main` |
| Updates | Frequent | Release only |
| Contents | Source code, tests, CI/CD | Package metadata |
| Users | Developers | End users |

---

## Resources

- **Arduino_Core_STM32**: https://github.com/geosmall/Arduino_Core_STM32
- **BoardManagerFiles**: https://github.com/geosmall/BoardManagerFiles
- **Claude Code**: https://claude.com/code
- **Issues**: https://github.com/geosmall/Arduino_Core_STM32/issues

---

## License

See individual submodule repositories for license information.
