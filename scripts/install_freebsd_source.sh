#!/bin/bash

# FreeBSD Source Installation Script
# This script downloads FreeBSD source code and sets up the structure
# following the patching strategy outlined in STRATEGY.md

set -uo pipefail

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
    "sys/_types.h" "sys/cdefs.h" "sys/endian.h" "sys/param.h" "sys/types.h" "sys/limits.h" "sys/syslimits.h" "sys/stdint.h"
    "sys/errno.h" "sys/stat.h" "sys/fcntl.h" "sys/unistd.h" "sys/uio.h" "unistd.h" "fcntl.h"
    "sys/time.h" "time.h"
    "sys/socket.h" "sys/select.h" "netdb.h" "arpa/inet.h" "netinet/in.h"
    "stdlib.h" "stdio.h" "stddef.h" "stdint.h" "stdarg.h" "stdbool.h" "limits.h" "errno.h" "assert.h"
    "string.h" "strings.h" "memory.h"
    "ctype.h" "wctype.h" "locale.h" "xlocale.h" "runetype.h"
    "inttypes.h" "_ctype.h" "signal.h"
    "math.h" "fenv.h"
    "pthread.h" "semaphore.h" "sched.h"
)

# Function to copy header with error handling
copy_header() {
    local header="$1"
    local src_header="$FREEBSD_SOURCE_DIR/include/$header"
    local dst_header="$TARGET_INCLUDE_DIR/$header"

    # Try include/ first, then sys/sys/ for system headers
    if [ ! -f "$src_header" ] && [[ "$header" == sys/* ]]; then
        # For sys/ headers, look in sys/sys/ directory
        local sys_header_name=$(basename "$header")
        src_header="$FREEBSD_SOURCE_DIR/sys/sys/$sys_header_name"
    fi

    if [ -f "$src_header" ]; then
        # Create directory structure if needed
        local dst_dir=$(dirname "$dst_header")
        mkdir -p "$dst_dir"

        # Copy the header
        cp "$src_header" "$dst_header"
        echo "  ✓ $header"
        return 0
    else
        echo "  ⚠ $header (not found, skipping)"
        return 1
    fi
}

# Sync headers by group
echo "Copying curated headers..."
for header in "${CORE_HEADERS[@]}"; do
    copy_header "$header"
done

cp "$TARGET_INCLUDE_DIR/sys/errno.h" "$TARGET_INCLUDE_DIR/errno.h"
echo "  ✓ errno.h (symlink)"

# Copy additional sys/ headers from kernel
echo "Syncing additional sys/ headers..."
SYS_HEADERS_SRC="$FREEBSD_SOURCE_DIR/sys/sys"
if [ -d "$SYS_HEADERS_SRC" ]; then
    mkdir -p "$TARGET_INCLUDE_DIR/sys"

    COMMON_SYS_HEADERS=(
        "queue.h" "tree.h" "_null.h" "_stdint.h" "_types.h" "_stdarg.h" 
        "_visible.h" "_pthreadtypes.h" "cdefs.h" "_endian.h" "bitcount.h"
        "_bitset.h" "bitset.h" "_callout.h" "_cpuset.h" "_lock.h" "_mutex.h"
        "_rwlock.h" "_sx.h" "_task.h" "_rmlock.h" "_condvar.h" "_eventhandler.h"
        "kassert.h" "systm.h" "kernel.h" "kobj.h" "linker_set.h" "_bus_dma.h"
        "bus.h" "callout.h" "cpu.h" "cpuset.h" "domain.h" "endian.h"
        "eventhandler.h" "file.h" "filedesc.h" "jail.h" "kdb.h" "ktr.h"
        "lock.h" "lockstat.h" "malloc.h" "mbuf.h" "mutex.h" "pcpu.h"
        "proc.h" "protosw.h" "refcount.h" "resourcevar.h" "rmlock.h"
        "rwlock.h" "sbuf.h" "sdt.h" "selinfo.h" "smp.h" "socket.h"
        "socketvar.h" "sx.h" "sysctl.h" "sysent.h" "taskqueue.h"
        "timetc.h" "ucred.h" "vnode.h" "timespec.h" "_sigval.h" "signal.h" "_sigaltstack.h" "_uio.h" "_param.h" "_maxphys.h" "_clock_id.h" "sched.h" "poll.h" "resource.h" "sysmacros.h" "ioctl.h" "ioccom.h" "filio.h" "sockio.h" "termios.h" "ttycom.h" "memrange.h" "gpio.h" "wait.h" "statfs.h" "membarrier.h" "mount.h" "statvfs.h" "dirent.h" "_winsize.h" "_ucontext.h"
        "_stddef.h" "_limits.h" "_timespec.h" "_timeval.h" "_iovec.h"
        "_sigset.h" "_sockaddr.h" "_fd_set.h" "_size_t.h" "_ssize_t.h"
        "_off_t.h" "_pid_t.h" "_uid_t.h" "_gid_t.h" "_mode_t.h"
        "_termios.h"
    )
    for header in "${COMMON_SYS_HEADERS[@]}"; do
        if [ -f "$SYS_HEADERS_SRC/$header" ]; then
            cp "$SYS_HEADERS_SRC/$header" "$TARGET_INCLUDE_DIR/sys/"
            echo "  ✓ sys/$header"
        fi
    done
fi

# Copy essential architecture-specific directories
echo "Syncing architecture-specific directories..."
mkdir -p "$TARGET_INCLUDE_DIR/x86"
rsync -a "$FREEBSD_SOURCE_DIR/sys/x86/include/" "$TARGET_INCLUDE_DIR/x86/"
echo "  ✓ x86 headers"

mkdir -p "$TARGET_INCLUDE_DIR/amd64"
rsync -a "$FREEBSD_SOURCE_DIR/sys/amd64/include/" "$TARGET_INCLUDE_DIR/amd64/"
echo "  ✓ amd64 headers"

# Create machine directory for architecture-specific headers
echo "Syncing machine-specific headers..."
mkdir -p "$TARGET_INCLUDE_DIR/machine"
cp "$FREEBSD_SOURCE_DIR/sys/x86/include/_types.h" "$TARGET_INCLUDE_DIR/machine/"
cp "$FREEBSD_SOURCE_DIR/sys/x86/include/_limits.h" "$TARGET_INCLUDE_DIR/machine/"
cp "$FREEBSD_SOURCE_DIR/sys/x86/include/_stdint.h" "$TARGET_INCLUDE_DIR/machine/"
cp "$FREEBSD_SOURCE_DIR/sys/x86/include/endian.h" "$TARGET_INCLUDE_DIR/machine/"
cp "$FREEBSD_SOURCE_DIR/sys/amd64/include/param.h" "$TARGET_INCLUDE_DIR/machine/"
echo "  ✓ machine/_types.h"
echo "  ✓ machine/_limits.h"
echo "  ✓ machine/_stdint.h"
echo "  ✓ machine/endian.h"

mkdir -p "$TARGET_INCLUDE_DIR/xlocale"
cp "$FREEBSD_SOURCE_DIR/include/xlocale/_strings.h" "$TARGET_INCLUDE_DIR/xlocale/"
cp "$FREEBSD_SOURCE_DIR/include/xlocale/_string.h" "$TARGET_INCLUDE_DIR/xlocale/"
echo "  ✓ xlocale/_strings.h"
echo "  ✓ xlocale/_string.h"

# Copy SSP (Stack Smashing Protection) headers
echo "Setting up SSP headers..."
SSP_SRC="$FREEBSD_SOURCE_DIR/include/ssp"
if [ -d "$SSP_SRC" ]; then
    mkdir -p "$TARGET_INCLUDE_DIR/ssp"
    rsync -a "$SSP_SRC/" "$TARGET_INCLUDE_DIR/ssp/"
    echo "  ✓ ssp headers"
fi

# Copy essential headers from other locations
echo "Syncing headers from other locations..."

# For stdarg.h, try architecture-specific locations first (critical for platform compilation)
STDARG_LOCATIONS=(
    "$FREEBSD_SOURCE_DIR/sys/amd64/include/stdarg.h"
    "$FREEBSD_SOURCE_DIR/sys/x86/include/stdarg.h"
    "$FREEBSD_SOURCE_DIR/sys/sys/stdarg.h"
    "$FREEBSD_SOURCE_DIR/include/stdarg.h"
    "$FREEBSD_SOURCE_DIR/contrib/llvm-project/clang/lib/Headers/stdarg.h"
)

for stdarg_path in "${STDARG_LOCATIONS[@]}"; do
    if [ -f "$stdarg_path" ] && [ ! -f "$TARGET_INCLUDE_DIR/stdarg.h" ]; then
        cp "$stdarg_path" "$TARGET_INCLUDE_DIR/"
        echo "  ✓ stdarg.h (from $(basename $(dirname $stdarg_path)))"
        break
    fi
done

# Also check for stdint.h in architecture-specific locations
if [ ! -f "$TARGET_INCLUDE_DIR/stdint.h" ]; then
    STDINT_LOCATIONS=(
        "$FREEBSD_SOURCE_DIR/sys/amd64/include/stdint.h"
        "$FREEBSD_SOURCE_DIR/sys/x86/include/stdint.h"
        "$FREEBSD_SOURCE_DIR/sys/sys/stdint.h"
    )

    for stdint_path in "${STDINT_LOCATIONS[@]}"; do
        if [ -f "$stdint_path" ]; then
            cp "$stdint_path" "$TARGET_INCLUDE_DIR/"
            echo "  ✓ stdint.h (from $(basename $(dirname $stdint_path)))"
            break
        fi
    done
fi

# Handle math.h and fenv.h - might be in lib/msun or include
for search_dir in "$FREEBSD_SOURCE_DIR/lib/msun/src" "$FREEBSD_SOURCE_DIR/include"; do
    if [ -f "$search_dir/math.h" ] && [ ! -f "$TARGET_INCLUDE_DIR/math.h" ]; then
        cp "$search_dir/math.h" "$TARGET_INCLUDE_DIR/"
        echo "  ✓ math.h (from $(basename $(dirname $search_dir)))/$(basename $search_dir))"
    fi
    if [ -f "$search_dir/fenv.h" ] && [ ! -f "$TARGET_INCLUDE_DIR/fenv.h" ]; then
        cp "$search_dir/fenv.h" "$TARGET_INCLUDE_DIR/"
        echo "  ✓ fenv.h (from $(basename $(dirname $search_dir)))/$(basename $search_dir))"
    fi
done

# Fix missing headers
echo "Fixing missing headers..."
cp "$FREEBSD_SOURCE_DIR/sys/sys/fcntl.h" "$TARGET_INCLUDE_DIR/fcntl.h"
echo "  ✓ fcntl.h"
mkdir -p "$TARGET_INCLUDE_DIR/netinet"
cp "$FREEBSD_SOURCE_DIR/sys/netinet/in.h" "$TARGET_INCLUDE_DIR/netinet/in.h"
echo "  ✓ netinet/in.h"
cp "$FREEBSD_SOURCE_DIR/sys/sys/stdint.h" "$TARGET_INCLUDE_DIR/stdint.h"
echo "  ✓ stdint.h"
cp "$FREEBSD_SOURCE_DIR/sys/sys/stdarg.h" "$TARGET_INCLUDE_DIR/stdarg.h"
echo "  ✓ stdarg.h"
cp "$FREEBSD_SOURCE_DIR/sys/sys/errno.h" "$TARGET_INCLUDE_DIR/errno.h"
echo "  ✓ errno.h"
cp "$FREEBSD_SOURCE_DIR/lib/msun/src/math.h" "$TARGET_INCLUDE_DIR/math.h"
echo "  ✓ math.h"
cp "$FREEBSD_SOURCE_DIR/lib/msun/x86/fenv.h" "$TARGET_INCLUDE_DIR/fenv.h"
echo "  ✓ fenv.h"

# Step 5: Update .gitignore to exclude the large FreeBSD source tree
echo "Step 5: Updating .gitignore..."
GITIGNORE="$FREEBSD_LIBC_ROOT/.gitignore"
if ! grep -q "freebsd-src/" "$GITIGNORE" 2>/dev/null; then
    echo "" >> "$GITIGNORE"
    echo "# FreeBSD source tree (large, don't commit)" >> "$GITIGNORE"
    echo "freebsd-src/" >> "$GITIGNORE"
fi

# Step 6: Create a quick verification script
echo "Step 6: Creating verification script..."
cat > "$FREEBSD_LIBC_ROOT/scripts/verify_setup.sh" << 'EOF'
#!/bin/bash

# Quick verification that the FreeBSD source setup is correct
set -uo pipefail

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