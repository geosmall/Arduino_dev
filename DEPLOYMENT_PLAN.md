# Arduino Workspace Deployment Plan

> **Status**: ✅ COMPLETE - All 7 phases finished, v1.0.0 published and validated
>
> **Current Status**: See [DEPLOYMENT_STATUS.md](DEPLOYMENT_STATUS.md) for detailed progress tracking
>
> **Completion Date**: 2025-10-26

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

**Core Features:**
- ✅ Full automation: version tag → GitHub release → package index update
- ✅ Archive creation with build artifact cleanup
- ✅ SHA-256 checksum and file size calculation
- ✅ GitHub release creation via `gh` CLI
- ✅ Package index auto-update with version history
- ✅ Cross-platform (Linux/macOS)
- ✅ Dry-run mode for testing
- ✅ Interactive confirmation with `--yes` flag for automation
- ✅ Comprehensive validation and error handling

**Production Hardening (discovered during v1.0.0 release):**
1. **Configurable branch variables** - Eliminates hardcoded branch names
2. **Explicit branch targeting** - `--target` flag ensures releases point to correct branch
3. **Comprehensive tag checking** - Validates local, remote, AND GitHub releases (catches drafts)
4. **Smart tag push prevention** - Detects when `gh release create` already pushed tag
5. **Detailed cleanup instructions** - 5-step error recovery guide with verification

**Tool Bundling:**
- ✅ 221 lines of tool definitions added to package index
- ✅ 5 tools with cross-platform support (Windows, macOS, Linux variants)
- ✅ Versions verified against platform.txt tested configuration
- ✅ Eliminates external Board Manager dependencies

**Usage:**
```bash
./create_release.sh 1.1.0 --dry-run  # Test without making changes
./create_release.sh 1.1.0             # Interactive (asks confirmation)
./create_release.sh 1.1.0 --yes       # Automated (no prompt)
```

**Result:** Production-validated automation with standalone installation

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
- [x] Production test with v1.0.0 release
- [x] Add 5 production hardening improvements (branch targeting, tag checking, etc.)
- [x] Add tool bundling to package index (221 lines)
- [x] Add --yes flag for automation
- [x] Update workspace README with 6-step release workflow
- [x] Update DEPLOYMENT_STATUS with completion and lessons learned
- [x] Update DEPLOYMENT_PLAN with tool bundling
- [x] Move deployment docs to workspace root

### Phase 6: Testing ✅ COMPLETE
- [x] Test Board Manager installation in Arduino IDE (v1.8.19 fresh install)
- [x] Verify example compilation from installed package (multiple libraries tested)
- [x] Validate library auto-discovery (11 libraries visible)
- [x] Verify tool downloads (all 5 tools: gcc, openocd, STM32Tools, CMSIS, SVD)
- [x] Confirm no dependency errors or missing tool issues

### Phase 7: Documentation ✅ COMPLETE
- [x] Final DEPLOYMENT_STATUS update (Phases 1-6 complete)
- [x] Update DEPLOYMENT_PLAN with Phase 6 results
- [x] Document production validation results and lessons learned

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
- **Production testing revealed critical issues that dry-run couldn't catch**:
  - Branch targeting: `gh release create` defaults to "main" instead of "ardu_ci"
  - Draft releases: Old drafts can get re-published instead of creating new release
  - Tag push redundancy: `gh release create` automatically pushes tags
  - Must use `--target` flag and check GitHub releases (not just local/remote tags)
- **Tool bundling is mandatory**:
  - Package index must include complete tool definitions (221 lines)
  - Tool versions must exactly match platform.txt tested configuration
  - Eliminates external Board Manager dependencies
  - Without bundling: "could not find referenced tool" error breaks installation
- **Error recovery procedures are essential**:
  - 5-step cleanup process (delete release, delete remote tag, delete local tag, revert commits, verify)
  - Color-coded terminal output helps with manual recovery
- **Automation flags**: `--yes` flag enables scripted releases without interactive prompts
- Dry-run mode remains critical for validating logic before making changes

### HIL Testing
- Library examples can use SEGGER_RTT API directly
- Application sketches should use ci_log.h for dual-mode support
- Both patterns work with aflash.sh exit wildcard detection

---

## Next Steps

**All deployment phases complete!** ✅

### Future Releases (Ongoing)
1. Use `./create_release.sh <version>` for all releases
2. Always test with `--dry-run` first
3. Follow semantic versioning (X.Y.Z)
4. Maintain workspace documentation
5. Continue hardware validation on new boards/peripherals

### Release Workflow
```bash
# Step 1: Verify clean state
./status-all.sh

# Step 2: Test with dry-run
./create_release.sh 1.1.0 --dry-run

# Step 3: Create release
./create_release.sh 1.1.0

# Step 4: Update workspace submodule pointer
git add BoardManagerFiles
git commit -m "Update BoardManagerFiles submodule pointer for v1.1.0"
git push origin dev
```

---

## Resources

- **Development Repository:** https://github.com/geosmall/Arduino_Core_STM32
- **Distribution Repository:** https://github.com/geosmall/BoardManagerFiles
- **Workspace Repository:** https://github.com/geosmall/Arduino
- **Board Manager URL:** https://github.com/geosmall/BoardManagerFiles/raw/main/package_stm32_robotics_index.json
- **Issues:** https://github.com/geosmall/Arduino_Core_STM32/issues
