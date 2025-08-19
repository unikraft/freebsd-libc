#!/bin/bash

# FreeBSD libc Setup Script for Unikraft
# This script sets up the complete FreeBSD libc integration from scratch
# Following the patching strategy - never modify FreeBSD source

set -euo pipefail

# Configuration
FREEBSD_VERSION="15.0-CURRENT"
FREEBSD_BRANCH="main"
FREEBSD_REPO="https://github.com/freebsd/freebsd-src.git"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FREEBSD_LIBC_ROOT="$(dirname "$SCRIPT_DIR")"
FREEBSD_SOURCE_DIR="$FREEBSD_LIBC_ROOT/freebsd-src"
TARGET_SRC_DIR="$FREEBSD_LIBC_ROOT/src"
TARGET_INCLUDE_DIR="$FREEBSD_LIBC_ROOT/freebsd-include"

echo "=== FreeBSD libc Setup for Unikraft ==="
echo "Target version: $FREEBSD_VERSION"
echo "Destination: $FREEBSD_LIBC_ROOT"
echo ""

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check dependencies
echo "Checking dependencies..."
for cmd in git rsync; do
    if ! command_exists "$cmd"; then
        echo "ERROR: $cmd is required but not installed"
        exit 1
    fi
done

# Step 1: Clone or update FreeBSD source
echo "Step 1: Acquiring FreeBSD source code..."
if [ -d "$FREEBSD_SOURCE_DIR" ]; then
    echo "FreeBSD source directory exists, updating..."
    cd "$FREEBSD_SOURCE_DIR"
    git fetch origin
    git checkout "$FREEBSD_BRANCH"
    git pull origin "$FREEBSD_BRANCH"
else
    echo "Cloning FreeBSD source repository..."
    git clone --depth 1 --branch "$FREEBSD_BRANCH" "$FREEBSD_REPO" "$FREEBSD_SOURCE_DIR"
fi

# Record the commit hash for reproducibility
cd "$FREEBSD_SOURCE_DIR"
COMMIT_HASH=$(git rev-parse HEAD)
echo "FreeBSD source at commit: $COMMIT_HASH"

# Step 2: Set up source symlink
echo "Step 2: Setting up source directory structure..."
if [ -e "$TARGET_SRC_DIR" ] && [ ! -L "$TARGET_SRC_DIR" ]; then
    echo "Removing existing src directory (backing up)..."
    mv "$TARGET_SRC_DIR" "$TARGET_SRC_DIR.backup.$(date +%Y%m%d_%H%M%S)"
elif [ -L "$TARGET_SRC_DIR" ]; then
    echo "Removing existing src symlink..."
    rm "$TARGET_SRC_DIR"
fi

# Create symlink from src to freebsd-src/lib/libc
echo "Creating symlink src -> freebsd-src/lib/libc..."
ln -sf "freebsd-src/lib/libc" "$TARGET_SRC_DIR"

# Verify the symlink
if [ -L "$TARGET_SRC_DIR" ] && [ -d "$TARGET_SRC_DIR" ]; then
    echo "✓ Symlink created successfully: src -> $(readlink "$TARGET_SRC_DIR")"
else
    echo "ERROR: Failed to create symlink"
    exit 1
fi

# Step 3: Run header synchronization
echo "Step 3: Setting up headers..."
if [ -f "$SCRIPT_DIR/sync_headers.sh" ]; then
    "$SCRIPT_DIR/sync_headers.sh"
else
    echo "Warning: sync_headers.sh not found, skipping header sync"
fi

# Step 4: Set up glue directory
echo "Step 4: Setting up glue layer..."
mkdir -p "$FREEBSD_LIBC_ROOT/glue"

# Step 5: Update .gitignore
echo "Step 5: Updating .gitignore..."
GITIGNORE="$FREEBSD_LIBC_ROOT/.gitignore"
if ! grep -q "freebsd-src/" "$GITIGNORE" 2>/dev/null; then
    echo "" >> "$GITIGNORE"
    echo "# FreeBSD source tree (large, don't commit)" >> "$GITIGNORE"
    echo "freebsd-src/" >> "$GITIGNORE"
fi

# Step 6: Create/update upstream tracking
echo "Step 6: Creating upstream tracking information..."
cat > "$FREEBSD_LIBC_ROOT/UPSTREAM.md" << EOF
# FreeBSD Source Tracking

## Current Source Version
- **Version**: $FREEBSD_VERSION
- **Branch**: $FREEBSD_BRANCH  
- **Commit**: $COMMIT_HASH
- **Repository**: $FREEBSD_REPO
- **Installation Date**: $(date -u '+%Y-%m-%d %H:%M:%S UTC')

## Directory Structure
- `freebsd-src/` - Complete FreeBSD source tree (DO NOT MODIFY)
- `src/` - Symlink to `freebsd-src/lib/libc` (libc source)
- `freebsd-include/` - Curated headers copied from FreeBSD
- `glue/` - Unikraft compatibility layer (OUR CODE)

## Update Process
To update to a newer FreeBSD version:
1. Run `scripts/setup.sh` 
2. Test compilation with existing glue layer
3. Update glue layer for any API changes
4. Update this file with new version info

## Patching Strategy
Following STRATEGY.md:
- **NEVER modify files in freebsd-src/**
- All Unikraft-specific changes go in glue/ directory
- Use symbol redirection and compatibility shims
- Keep patches minimal and well-documented
EOF

# Step 7: Run verification
echo "Step 7: Running verification..."
if [ -f "$SCRIPT_DIR/verify_setup.sh" ]; then
    "$SCRIPT_DIR/verify_setup.sh"
else
    echo "Warning: verify_setup.sh not found, skipping verification"
fi

echo ""
echo "=== Setup Complete! ==="
echo ""
echo "FreeBSD libc has been set up for Unikraft integration."
echo ""
echo "Next steps:"
echo "1. Implement glue layer in glue/ directory"
echo "2. Update Config.uk and Makefile.uk for your needs"
echo "3. Test with applications like c-hello"
echo "4. Run scripts/apply_changes.sh to synchronize any updates"
echo ""
echo "Remember: Follow the patching strategy - FreeBSD source is never modified."
