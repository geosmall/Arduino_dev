# Arduino Development Workspace

> **Purpose**: Claude-assisted development workspace for managing [Arduino_Core_STM32](https://github.com/geosmall/Arduino_Core_STM32) and [BoardManagerFiles](https://github.com/geosmall/BoardManagerFiles) as submodules.

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
├── README.md                 # This file
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
