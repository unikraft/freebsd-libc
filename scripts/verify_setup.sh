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
