#!/bin/bash

# FreeBSD Source Installation Script
# This script downloads FreeBSD source code and sets up the structure
# following the patching strategy outlined in STRATEGY.md

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

echo "=== FreeBSD libc Source Installation ==="
echo "Target version: $FREEBSD_VERSION"
echo "Branch: $FREEBSD_BRANCH"
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

# Step 2: Remove existing src if it's not a symlink
echo "Step 2: Setting up source directory structure..."
if [ -e "$TARGET_SRC_DIR" ] && [ ! -L "$TARGET_SRC_DIR" ]; then
    echo "Removing existing src directory (backing up to src.backup)..."
    mv "$TARGET_SRC_DIR" "$TARGET_SRC_DIR.backup.$(date +%Y%m%d_%H%M%S)"
elif [ -L "$TARGET_SRC_DIR" ]; then
    echo "Removing existing src symlink..."
    rm "$TARGET_SRC_DIR"
fi

# Step 3: Create symlink from src to freebsd-src/lib/libc
echo "Step 3: Creating symlink src -> freebsd-src/lib/libc..."
ln -sf "freebsd-src/lib/libc" "$TARGET_SRC_DIR"

# Verify the symlink
if [ -L "$TARGET_SRC_DIR" ] && [ -d "$TARGET_SRC_DIR" ]; then
    echo "✓ Symlink created successfully: src -> $(readlink "$TARGET_SRC_DIR")"
else
    echo "ERROR: Failed to create symlink"
    exit 1
fi

# Step 4: Set up freebsd-include directory with curated headers
echo "Step 4: Setting up FreeBSD headers..."

# Create freebsd-include directory
mkdir -p "$TARGET_INCLUDE_DIR"

# Define core headers needed for basic libc functionality
# Following the strategy of curated, minimal header set
CORE_HEADERS=(
    # Essential system headers
    "sys/_types.h"
    "sys/cdefs.h"
    "sys/endian.h"
    "sys/param.h"
    "sys/types.h"
    "sys/errno.h"
    "sys/stat.h"
    "sys/fcntl.h"
    "sys/unistd.h"
    "sys/time.h"
    "sys/socket.h"
    "sys/select.h"
    "sys/uio.h"
    
    # String and memory
    "string.h"
    "strings.h"
    "memory.h"
    
    # Standard library
    "stdlib.h"
    "stdio.h"
    "stddef.h"
    "stdint.h"
    "stdarg.h"
    "stdbool.h"
    "limits.h"
    "errno.h"
    "assert.h"
    
    # Character and locale
    "ctype.h"
    "wctype.h"
    "locale.h"
    
    # Time
    "time.h"
    
    # Math (if we include libm later)
    "math.h"
    "fenv.h"
    
    # Threading (for later stages)
    "pthread.h"
    "semaphore.h"
    
    # Networking (for later stages)  
    "netdb.h"
    "arpa/inet.h"
    "netinet/in.h"
)

echo "Copying curated headers..."
for header in "${CORE_HEADERS[@]}"; do
    src_header="$FREEBSD_SOURCE_DIR/include/$header"
    dst_header="$TARGET_INCLUDE_DIR/$header"
    
    if [ -f "$src_header" ]; then
        # Create directory structure if needed
        dst_dir=$(dirname "$dst_header")
        mkdir -p "$dst_dir"
        
        # Copy the header
        cp "$src_header" "$dst_header"
        echo "  ✓ $header"
    else
        echo "  ⚠ $header (not found, skipping)"
    fi
done

# Copy some essential sys/ headers that may be in different locations
SYS_HEADERS_SRC="$FREEBSD_SOURCE_DIR/sys/sys"
if [ -d "$SYS_HEADERS_SRC" ]; then
    echo "Copying additional sys/ headers..."
    mkdir -p "$TARGET_INCLUDE_DIR/sys"
    
    for header in queue.h tree.h; do
        if [ -f "$SYS_HEADERS_SRC/$header" ]; then
            cp "$SYS_HEADERS_SRC/$header" "$TARGET_INCLUDE_DIR/sys/"
            echo "  ✓ sys/$header"
        fi
    done
fi

# Step 5: Create upstream tracking file
echo "Step 5: Creating upstream tracking information..."
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
1. Run `scripts/install_freebsd_source.sh` 
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

# Step 6: Update .gitignore to exclude the large FreeBSD source tree
echo "Step 6: Updating .gitignore..."
GITIGNORE="$FREEBSD_LIBC_ROOT/.gitignore"
if ! grep -q "freebsd-src/" "$GITIGNORE" 2>/dev/null; then
    echo "" >> "$GITIGNORE"
    echo "# FreeBSD source tree (large, don't commit)" >> "$GITIGNORE"
    echo "freebsd-src/" >> "$GITIGNORE"
fi

# Step 7: Create a quick verification script
echo "Step 7: Creating verification script..."
cat > "$FREEBSD_LIBC_ROOT/scripts/verify_setup.sh" << 'EOF'
#!/bin/bash

# Quick verification that the FreeBSD source setup is correct
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FREEBSD_LIBC_ROOT="$(dirname "$SCRIPT_DIR")"

echo "=== FreeBSD libc Setup Verification ==="

# Check symlink
if [ -L "$FREEBSD_LIBC_ROOT/src" ] && [ -d "$FREEBSD_LIBC_ROOT/src" ]; then
    echo "✓ src symlink is valid: $(readlink "$FREEBSD_LIBC_ROOT/src")"
else
    echo "✗ src symlink is missing or broken"
    exit 1
fi

# Check key source directories
for dir in string stdio stdlib gen; do
    if [ -d "$FREEBSD_LIBC_ROOT/src/$dir" ]; then
        echo "✓ src/$dir directory exists"
    else
        echo "✗ src/$dir directory missing"
        exit 1
    fi
done

# Check headers
for header in stdio.h stdlib.h string.h sys/types.h; do
    if [ -f "$FREEBSD_LIBC_ROOT/freebsd-include/$header" ]; then
        echo "✓ freebsd-include/$header exists"
    else
        echo "✗ freebsd-include/$header missing"
    fi
done

# Check glue directory
if [ -d "$FREEBSD_LIBC_ROOT/glue" ]; then
    echo "✓ glue directory exists for compatibility layer"
else
    echo "✗ glue directory missing"
fi

echo "✓ Setup verification complete!"
EOF

chmod +x "$FREEBSD_LIBC_ROOT/scripts/verify_setup.sh"

# Final verification
echo ""
echo "=== Installation Complete ==="
echo "Running verification..."
"$FREEBSD_LIBC_ROOT/scripts/verify_setup.sh"

echo ""
echo "FreeBSD source installation successful!"
echo ""
echo "Next steps:"
echo "1. Review the symlinked source in: $TARGET_SRC_DIR"
echo "2. Check curated headers in: $TARGET_INCLUDE_DIR"  
echo "3. Implement compatibility layer in: $FREEBSD_LIBC_ROOT/glue/"
echo "4. Update Makefile.uk to use the new structure"
echo ""
echo "Remember: Follow the patching strategy - never modify files in freebsd-src/"
