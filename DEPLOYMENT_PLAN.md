# Arduino Workspace Deployment Plan

> **Status**: ✅ COMPLETE - Workspace operational, v1.0.0 published, release automation implemented
>
> **Current Status**: See [DEPLOYMENT_STATUS.md](DEPLOYMENT_STATUS.md) for detailed progress tracking

---

## Overview

Transform `Arduino_dev` repository into a **Claude development workspace** managing two submodules:
- **Arduino_Core_STM32** - Development repository (STM32 core + robotics libraries)
- **BoardManagerFiles** - Distribution repository (Board Manager package)

This provides clean separation between development, distribution, and workspace orchestration.

---

## Architecture Evolution

### Original State (Pre-Consolidation)

**Arduino_dev Repository:**
- Robotics libraries as separate directories
- Arduino_Core_STM32 as submodule (core only)
- CI/CD scripts at top level
- Mixed development and orchestration

### Phase 1: Consolidation into Arduino_Core_STM32 ✅ COMPLETE

**Goal:** Create self-contained development repository

**Moved to Arduino_Core_STM32:**
- ✅ All libraries: SerialRx, IMU, ICM42688P, LittleFS, SDFS, Storage, minIniStorage, TimerPWM, libPrintf, AUnit, STM32RTC
- ✅ CI/CD scripts → `system/ci/` (17 scripts)
- ✅ Test suites → `tests/` (10 unit test suites)
- ✅ Board configs → `targets/` (BoardConfig system)
- ✅ Tools → `extras/` (Betaflight converter with 53 tests)
- ✅ Documentation → `doc/` (technical specs)
- ✅ CMake support → `cmake/` (build system examples)
- ✅ Development guides → `CLAUDE.md`, `README.md`

**Result:** Single development repository with complete robotics stack

### Phase 2: Create BoardManagerFiles Repository ✅ COMPLETE

**Goal:** Separate distribution from development

**Created:**
- ✅ Repository: `geosmall/BoardManagerFiles`
- ✅ Package index: `package_stm32_robotics_index.json`
- ✅ Installation documentation: `README.md`
- ✅ License: BSD-3-Clause

**Result:** Clean distribution channel for end users

### Phase 3: Initial Release (v1.0.0) ✅ COMPLETE

**Goal:** Publish first Board Manager release

**Accomplished:**
- ✅ Tagged Arduino_Core_STM32 as `robo-1.0.0`
- ✅ Created release archive: `STM32-Robotics-1.0.0.tar.bz2` (27 MB)
- ✅ Published GitHub release with archive asset
- ✅ Updated package index with checksum and download URL
- ✅ Live at: `https://github.com/geosmall/BoardManagerFiles/raw/main/package_stm32_robotics_index.json`

**Result:** STM32 Robotics Core installable via Arduino IDE Board Manager

### Phase 4: Workspace Transformation ✅ COMPLETE

**Goal:** Transform Arduino_dev into orchestration workspace

**Architecture Decision:** Instead of deprecating Arduino_dev, repurpose it as a **Claude development workspace** that manages both repositories as submodules.

**Created Workspace:**
```
Arduino/ (formerly Arduino_dev)
├── Arduino_Core_STM32/    # Submodule (ardu_ci branch)
├── BoardManagerFiles/     # Submodule (main branch)
├── .claude/               # Claude Code configuration
├── CLAUDE.md              # Workspace development guide
├── DEPLOYMENT_PLAN.md     # This document
├── DEPLOYMENT_STATUS.md   # Progress tracker
├── README.md              # Workspace documentation
├── create_release.sh      # Release automation
├── update-submodules.sh   # Update helper
├── sync-workspace.sh      # Sync helper
└── status-all.sh          # Status helper
```

**Workspace Responsibilities:**
- Manage both submodules via git
- Orchestrate releases (create_release.sh)
- Provide development environment for Claude Code
- Cross-repo coordination and helpers

**Result:** Clean 3-tier architecture (workspace → development → distribution)

### Phase 5: Release Automation ✅ COMPLETE

**Goal:** Automate future release creation

**Implemented:** `create_release.sh` (workspace root)

**Features:**
- ✅ Full automation: version tag → GitHub release → package index update
- ✅ Archive creation with build artifact cleanup
- ✅ SHA-256 checksum and file size calculation
- ✅ GitHub release creation via `gh` CLI
- ✅ Package index auto-update with version history
- ✅ Cross-platform (Linux/macOS)
- ✅ Dry-run mode for testing
- ✅ Interactive confirmation
- ✅ Comprehensive validation and error handling

**Usage:**
```bash
./create_release.sh 1.1.0 --dry-run  # Test
./create_release.sh 1.1.0             # Create release
```

**Result:** One-command releases ready for v1.1.0+

---

## Current Repository Structure

### Arduino_Core_STM32 (Development)

**Repository:** https://github.com/geosmall/Arduino_Core_STM32
**Branch:** `ardu_ci`
**Purpose:** All development work

**Contents:**
- STM32 Arduino core (stm32duino fork)
- 11 robotics libraries (complete UAV stack)
- CI/CD framework (`system/ci/` with 17 scripts)
- HIL testing with RTT + J-Link
- 10 unit test suites (100% pass rate on hardware)
- BoardConfig system (multi-board support)
- Betaflight config converter (53 passing tests)
- Comprehensive documentation

### BoardManagerFiles (Distribution)

**Repository:** https://github.com/geosmall/BoardManagerFiles
**Branch:** `main`
**Purpose:** Arduino IDE Board Manager distribution

**Contents:**
- Package index JSON (version history)
- Installation documentation
- Release metadata only

### Arduino Workspace (Orchestration)

**Repository:** https://github.com/geosmall/Arduino (formerly Arduino_dev)
**Branch:** `dev` (workspace branch)
**Purpose:** Claude development environment

**Contents:**
- Both repositories as submodules
- Release automation (`create_release.sh`)
- Workspace helpers (update, sync, status)
- Deployment documentation
- Claude Code integration

---

## User Workflows

### End Users (Arduino IDE)

1. Add Board Manager URL in Arduino IDE preferences
2. Install "STM32 Robotics Core" via Board Manager
3. Select board from **Tools → Board** menu
4. Use libraries from **File → Examples** menu
5. Compile and upload sketches

**Board Manager URL:**
```
https://github.com/geosmall/BoardManagerFiles/raw/main/package_stm32_robotics_index.json
```

### Developers (Direct Development)

1. Clone Arduino_Core_STM32 repository
2. Use build scripts: `./system/ci/build.sh <sketch>`
3. Run HIL tests: `./system/ci/aflash.sh <sketch> --use-rtt --build-id`
4. Develop libraries in `libraries/` directory
5. Run unit tests in `tests/` directory

### Maintainers (Releases)

1. Develop features in Arduino_Core_STM32 submodule
2. Commit and push changes to `ardu_ci` branch
3. From workspace root: `./create_release.sh <version>`
4. Script automates: tag → release → package index update
5. Test Board Manager installation
6. Update DEPLOYMENT_STATUS.md

---

## Deployment Checklist

### Phase 1: Consolidation ✅ COMPLETE
- [x] Move all libraries to Arduino_Core_STM32
- [x] Move CI/CD scripts to system/ci/
- [x] Move tests, targets, extras, doc to Arduino_Core_STM32
- [x] Update all internal paths and documentation
- [x] Commit and push to ardu_ci branch

### Phase 2: BoardManagerFiles ✅ COMPLETE
- [x] Create GitHub repository
- [x] Create package index JSON
- [x] Add installation documentation
- [x] Publish to GitHub

### Phase 3: Initial Release ✅ COMPLETE
- [x] Tag Arduino_Core_STM32 as robo-1.0.0
- [x] Create release archive
- [x] Upload to GitHub releases
- [x] Update package index with checksum/size
- [x] Publish to Board Manager

### Phase 4: Workspace Transformation ✅ COMPLETE
- [x] Add BoardManagerFiles as submodule
- [x] Remove consolidated directories from workspace
- [x] Create workspace helper scripts
- [x] Update documentation for workspace purpose
- [x] Clean .gitignore and .gitmodules
- [x] Update Claude Code settings

### Phase 5: Release Automation ✅ COMPLETE
- [x] Create create_release.sh script
- [x] Test with dry-run mode
- [x] Update workspace README with release workflow
- [x] Update DEPLOYMENT_STATUS with completion
- [x] Move deployment docs to workspace root

### Phase 6: Testing 📋 PENDING
- [ ] Test Board Manager installation in Arduino IDE
- [ ] Verify example compilation from installed package
- [ ] Validate library auto-discovery
- [ ] Test CI/CD scripts from installed package

### Phase 7: Documentation 📋 PENDING
- [ ] Final DEPLOYMENT_STATUS update
- [ ] Archive deployment documentation
- [ ] Update external references if needed

---

## Key Decisions & Rationale

### Workspace Architecture

**Decision:** Repurpose Arduino_dev as workspace instead of deprecating

**Rationale:**
- Provides natural orchestration layer for cross-repo operations
- Claude Code development environment with both repos
- Helper scripts manage submodules cleanly
- Clean separation: workspace (orchestration) → submodules (content)

### Release Script Location

**Decision:** Place create_release.sh in workspace root

**Rationale:**
- Releases span both repositories (tag core, update package index)
- Workspace is natural orchestration point
- Consistent with other workspace helpers
- Direct access to both submodules as siblings

### Three-Tier Architecture

**Decision:** Workspace → Development → Distribution

**Rationale:**
- **Workspace** - Orchestration, release management, Claude environment
- **Development** - All development work (Arduino_Core_STM32)
- **Distribution** - End user package delivery (BoardManagerFiles)
- Clean separation of concerns
- Scalable for future growth

---

## Lessons Learned

### Git Workflow
- Always commit submodule first, then parent workspace
- Workspace tracks specific submodule commits (pointers)
- Use workspace helpers to keep both repos in sync

### Build Artifacts
- Workspace cleanup script doesn't automatically clean submodules
- Run cleanup_repo.sh in submodules before commits
- Exclude build/ and build_id.h from version control

### Release Process
- Manual releases are error-prone (v1.0.0 experience)
- Automation essential for consistency (create_release.sh)
- Dry-run mode critical for testing
- GitHub CLI (`gh`) eliminates manual web UI steps

### HIL Testing
- Library examples can use SEGGER_RTT API directly
- Application sketches should use ci_log.h for dual-mode support
- Both patterns work with aflash.sh exit wildcard detection

---

## Next Steps

1. **Phase 6: Testing** (2-3 hours)
   - Test Board Manager installation end-to-end
   - Verify example compilation
   - Validate library auto-discovery

2. **Phase 7: Documentation** (30 minutes)
   - Final deployment status update
   - Archive this document as reference

3. **Future Releases** (ongoing)
   - Use `./create_release.sh <version>` for all releases
   - Follow semantic versioning (X.Y.Z)
   - Test with dry-run first

---

## Resources

- **Development Repository:** https://github.com/geosmall/Arduino_Core_STM32
- **Distribution Repository:** https://github.com/geosmall/BoardManagerFiles
- **Workspace Repository:** https://github.com/geosmall/Arduino
- **Board Manager URL:** https://github.com/geosmall/BoardManagerFiles/raw/main/package_stm32_robotics_index.json
- **Issues:** https://github.com/geosmall/Arduino_Core_STM32/issues
