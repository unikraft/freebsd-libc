#!/bin/bash

# FreeBSD libc Apply Patches Script
# This script applies all compatibility patches from the patches/ directory.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FREEBSD_LIBC_ROOT="$(dirname "$SCRIPT_DIR")"
PATCH_DIR="$FREEBSD_LIBC_ROOT/patches"

log() {
    echo "==> $1"
}

echo "=== FreeBSD libc Apply Patches ==="

if [ ! -d "$PATCH_DIR" ]; then
    echo "ERROR: Patches directory not found: $PATCH_DIR"
    exit 1
fi

cd "$FREEBSD_LIBC_ROOT"

log "Cleaning up old source..."
rm -rf freebsd-src freebsd-include

bash "$SCRIPT_DIR/install_freebsd_source.sh"

find "$PATCH_DIR" -type f -name "*.patch" | sort | while read patch_file;
do
    echo "Applying patch: $(basename "$patch_file")"
    patch -p1 --forward -d "$FREEBSD_LIBC_ROOT" < "$patch_file" 
    if [ $? -ne 0 ]; then
        echo "ERROR: Failed to apply patch $(basename "$patch_file")."
        exit 1
    fi
done

echo ""
echo "✓ All patches applied successfully!"
