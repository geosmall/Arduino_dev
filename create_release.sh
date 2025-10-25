#!/bin/bash
# Arduino Workspace Release Script
# Creates Board Manager releases for STM32 Robotics Core
#
# Usage: ./create_release.sh <version> [--dry-run]
# Example: ./create_release.sh 1.1.0
#          ./create_release.sh 1.1.0 --dry-run

set -e  # Exit on error

# Configuration
TAG_PREFIX="robo-"
CORE_REPO="Arduino_Core_STM32"
BOARD_MGR_REPO="BoardManagerFiles"
PACKAGE_INDEX="package_stm32_robotics_index.json"
ARCHIVE_PREFIX="STM32-Robotics"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Parse arguments
VERSION=""
DRY_RUN=false

while [[ $# -gt 0 ]]; do
  case $1 in
    --dry-run)
      DRY_RUN=true
      shift
      ;;
    *)
      if [ -z "$VERSION" ]; then
        VERSION=$1
      else
        echo -e "${RED}Error: Unknown argument '$1'${NC}"
        exit 1
      fi
      shift
      ;;
  esac
done

# Functions
print_header() {
  echo -e "${BLUE}===================================================${NC}"
  echo -e "${BLUE}$1${NC}"
  echo -e "${BLUE}===================================================${NC}"
}

print_info() {
  echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
  echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
  echo -e "${RED}[ERROR]${NC} $1"
}

check_command() {
  if ! command -v $1 &> /dev/null; then
    print_error "Required command '$1' not found"
    exit 1
  fi
}

# Validate version format (semver)
validate_version() {
  if ! [[ $VERSION =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    print_error "Invalid version format: $VERSION"
    echo "Expected: X.Y.Z (e.g., 1.1.0)"
    exit 1
  fi
}

# Check if we're in the workspace root
check_workspace() {
  if [ ! -d "$CORE_REPO" ] || [ ! -d "$BOARD_MGR_REPO" ]; then
    print_error "This script must be run from the workspace root"
    echo "Expected directories:"
    echo "  - $CORE_REPO/"
    echo "  - $BOARD_MGR_REPO/"
    exit 1
  fi
}

# Check git status
check_git_status() {
  local repo=$1
  local branch=$2

  print_info "Checking $repo repository status..."

  cd "$repo"

  # Check if on correct branch
  local current_branch=$(git branch --show-current)
  if [ "$current_branch" != "$branch" ]; then
    print_error "$repo is on branch '$current_branch', expected '$branch'"
    cd ..
    exit 1
  fi

  # Check for uncommitted changes
  if ! git diff-index --quiet HEAD --; then
    print_error "$repo has uncommitted changes"
    git status --short
    cd ..
    exit 1
  fi

  # Check for untracked files that might matter
  local untracked=$(git ls-files --others --exclude-standard | wc -l)
  if [ $untracked -gt 0 ]; then
    print_warning "$repo has $untracked untracked files"
    git ls-files --others --exclude-standard | head -5
  fi

  cd ..
  print_info "$repo is clean on branch $branch"
}

# Check if tag exists
check_tag() {
  local tag="$TAG_PREFIX$VERSION"

  cd "$CORE_REPO"
  if git rev-parse "$tag" >/dev/null 2>&1; then
    print_error "Tag '$tag' already exists in $CORE_REPO"
    cd ..
    exit 1
  fi
  cd ..

  print_info "Tag '$tag' is available"
}

# Create archive
create_archive() {
  local archive_name="${ARCHIVE_PREFIX}-${VERSION}.tar.bz2"

  print_info "Creating archive: $archive_name"

  if [ "$DRY_RUN" = true ]; then
    print_warning "[DRY-RUN] Would create: $archive_name"
    echo "dummy" > "$archive_name"  # Create dummy file for checksum calc
    return 0
  fi

  # Clean any build artifacts before archiving
  print_info "Cleaning build artifacts in $CORE_REPO..."
  cd "$CORE_REPO"
  ./system/ci/cleanup_repo.sh >/dev/null 2>&1 || true
  cd ..

  # Create archive from Arduino_Core_STM32
  tar -cjf "$archive_name" \
    --exclude='.git' \
    --exclude='.github' \
    --exclude='.gitignore' \
    --exclude='*.md' \
    --exclude='DEPLOYMENT_*.md' \
    "$CORE_REPO"

  print_info "Archive created: $archive_name"
}

# Calculate checksum and size
calculate_metadata() {
  local archive_name="${ARCHIVE_PREFIX}-${VERSION}.tar.bz2"

  print_info "Calculating checksum and size..."

  # Cross-platform checksum
  if command -v shasum &> /dev/null; then
    CHECKSUM=$(shasum -a 256 "$archive_name" | awk '{print $1}')
  elif command -v sha256sum &> /dev/null; then
    CHECKSUM=$(sha256sum "$archive_name" | awk '{print $1}')
  else
    print_error "Neither shasum nor sha256sum found"
    exit 1
  fi

  # Cross-platform file size
  if stat -f%z "$archive_name" &> /dev/null; then
    # macOS
    SIZE=$(stat -f%z "$archive_name")
  else
    # Linux
    SIZE=$(stat -c%s "$archive_name")
  fi

  print_info "SHA-256: $CHECKSUM"
  print_info "Size: $SIZE bytes ($(numfmt --to=iec-i --suffix=B $SIZE 2>/dev/null || echo \"$SIZE bytes\"))"
}

# Create GitHub release
create_github_release() {
  local tag="$TAG_PREFIX$VERSION"
  local archive_name="${ARCHIVE_PREFIX}-${VERSION}.tar.bz2"
  local title="STM32 Robotics Core v${VERSION}"

  print_info "Creating GitHub release..."

  if [ "$DRY_RUN" = true ]; then
    print_warning "[DRY-RUN] Would create release: $tag"
    print_warning "[DRY-RUN] Would upload: $archive_name"
    RELEASE_URL="https://github.com/geosmall/Arduino_Core_STM32/releases/download/$tag/$archive_name"
    return 0
  fi

  cd "$CORE_REPO"

  # Create tag
  print_info "Creating tag: $tag"
  git tag -a "$tag" -m "$title"

  # Create release with gh CLI
  print_info "Creating GitHub release..."
  gh release create "$tag" \
    --title "$title" \
    --generate-notes \
    --repo geosmall/Arduino_Core_STM32 \
    "../$archive_name"

  # Get release URL
  RELEASE_URL=$(gh release view "$tag" --repo geosmall/Arduino_Core_STM32 --json assets --jq '.assets[0].url')

  cd ..

  print_info "Release created: $RELEASE_URL"
}

# Update package index
update_package_index() {
  local package_file="$BOARD_MGR_REPO/$PACKAGE_INDEX"
  local archive_name="${ARCHIVE_PREFIX}-${VERSION}.tar.bz2"
  local download_url="$RELEASE_URL"

  print_info "Updating package index..."

  if [ "$DRY_RUN" = true ]; then
    print_warning "[DRY-RUN] Would update: $package_file"
    print_warning "[DRY-RUN] New version entry:"
    cat <<EOF
{
  "name": "STM32 Robotics Core",
  "architecture": "stm32",
  "version": "$VERSION",
  "category": "Contributed",
  "url": "$download_url",
  "archiveFileName": "$archive_name",
  "checksum": "SHA-256:$CHECKSUM",
  "size": "$SIZE"
}
EOF
    return 0
  fi

  # Create new version entry
  local new_entry=$(jq -n \
    --arg version "$VERSION" \
    --arg url "$download_url" \
    --arg filename "$archive_name" \
    --arg checksum "SHA-256:$CHECKSUM" \
    --arg size "$SIZE" \
    '{
      name: "STM32 Robotics Core",
      architecture: "stm32",
      version: $version,
      category: "Contributed",
      url: $url,
      archiveFileName: $filename,
      checksum: $checksum,
      size: $size,
      boards: [
        {name: "NUCLEO_F411RE"},
        {name: "BLACKPILL_F411CE"},
        {name: "NOXE_V3"}
      ],
      toolsDependencies: [
        {
          packager: "STM32_Robotics",
          name: "xpack-arm-none-eabi-gcc",
          version: "12.2.1-1.2"
        },
        {
          packager: "STM32_Robotics",
          name: "xpack-openocd",
          version: "0.12.0-1"
        },
        {
          packager: "STM32_Robotics",
          name: "STM32Tools",
          version: "2.3.1"
        },
        {
          packager: "STM32_Robotics",
          name: "CMSIS",
          version: "5.9.0"
        },
        {
          packager: "STM32_Robotics",
          name: "STM32_SVD",
          version: "1.18.1"
        }
      ]
    }')

  # Update package index by prepending new version (newest first)
  jq --argjson new_entry "$new_entry" \
    '.packages[0].platforms = [$new_entry] + .packages[0].platforms' \
    "$package_file" > "${package_file}.tmp"

  mv "${package_file}.tmp" "$package_file"

  print_info "Package index updated"
}

# Commit and push changes
commit_and_push() {
  print_info "Committing and pushing changes..."

  if [ "$DRY_RUN" = true ]; then
    print_warning "[DRY-RUN] Would commit and push:"
    print_warning "  - $BOARD_MGR_REPO package index"
    print_warning "  - $CORE_REPO tag $TAG_PREFIX$VERSION"
    return 0
  fi

  # Commit BoardManagerFiles changes
  cd "$BOARD_MGR_REPO"
  git add "$PACKAGE_INDEX"
  git commit -m "Add STM32 Robotics Core v${VERSION} to package index"
  git push origin main
  cd ..

  # Push Arduino_Core_STM32 tag
  cd "$CORE_REPO"
  git push origin "$TAG_PREFIX$VERSION"
  cd ..

  print_info "Changes committed and pushed"
}

# Show summary
show_summary() {
  local tag="$TAG_PREFIX$VERSION"
  local archive_name="${ARCHIVE_PREFIX}-${VERSION}.tar.bz2"

  print_header "Release Summary"
  echo ""
  echo "Version: $VERSION"
  echo "Tag: $tag"
  echo "Archive: $archive_name"
  echo "Size: $SIZE bytes ($(numfmt --to=iec-i --suffix=B $SIZE 2>/dev/null || echo \"$SIZE bytes\"))"
  echo "Checksum: $CHECKSUM"
  echo ""
  echo "GitHub Release:"
  echo "  https://github.com/geosmall/Arduino_Core_STM32/releases/tag/$tag"
  echo ""
  echo "Board Manager URL:"
  echo "  https://github.com/geosmall/BoardManagerFiles/raw/main/$PACKAGE_INDEX"
  echo ""
  if [ "$DRY_RUN" = false ]; then
    echo -e "${GREEN}✓ Release published successfully!${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. Test Board Manager installation:"
    echo "     Arduino IDE → Preferences → Add Board Manager URL"
    echo "  2. Verify example compilation"
    echo "  3. Update Arduino_Core_STM32/DEPLOYMENT_STATUS.md"
  else
    echo -e "${YELLOW}[DRY-RUN] No changes were made${NC}"
    echo ""
    echo "To create the release for real, run:"
    echo "  ./create_release.sh $VERSION"
  fi
  echo ""
}

# Cleanup on error
cleanup_on_error() {
  print_error "Release creation failed!"

  if [ "$DRY_RUN" = false ]; then
    print_warning "Cleaning up..."

    # Remove archive if it exists
    local archive_name="${ARCHIVE_PREFIX}-${VERSION}.tar.bz2"
    if [ -f "$archive_name" ]; then
      rm "$archive_name"
      print_info "Removed archive: $archive_name"
    fi

    # Note: GitHub release and tag cleanup must be done manually if needed
    echo ""
    echo "Manual cleanup may be required:"
    echo "  - Check GitHub release: https://github.com/geosmall/Arduino_Core_STM32/releases"
    echo "  - Delete tag if created: git tag -d $TAG_PREFIX$VERSION && git push origin :refs/tags/$TAG_PREFIX$VERSION"
  fi

  exit 1
}

trap cleanup_on_error ERR

# Main execution
main() {
  print_header "Arduino Workspace Release Script"

  # Show usage if no version provided
  if [ -z "$VERSION" ]; then
    echo "Usage: $0 <version> [--dry-run]"
    echo ""
    echo "Examples:"
    echo "  $0 1.1.0              # Create release v1.1.0"
    echo "  $0 1.1.0 --dry-run    # Test release process without making changes"
    echo ""
    exit 1
  fi

  if [ "$DRY_RUN" = true ]; then
    print_warning "DRY-RUN MODE - No changes will be made"
    echo ""
  fi

  # Preflight checks
  print_info "Running preflight checks..."
  check_command git
  check_command gh
  check_command jq
  check_command tar
  validate_version
  check_workspace
  check_git_status "$CORE_REPO" "ardu_ci"
  check_git_status "$BOARD_MGR_REPO" "main"
  check_tag

  echo ""
  print_info "Creating release for version: $VERSION"

  # Confirmation prompt
  if [ "$DRY_RUN" = false ]; then
    echo ""
    read -p "Continue with release creation? (y/N): " -n 1 -r
    echo ""
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
      print_info "Release cancelled"
      exit 0
    fi
  fi

  echo ""

  # Execute release workflow
  create_archive
  calculate_metadata
  create_github_release
  update_package_index
  commit_and_push

  # Cleanup dry-run dummy archive
  if [ "$DRY_RUN" = true ]; then
    rm -f "${ARCHIVE_PREFIX}-${VERSION}.tar.bz2"
  fi

  echo ""
  show_summary
}

main
