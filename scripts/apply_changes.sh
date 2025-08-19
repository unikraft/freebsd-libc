#!/bin/bash

# FreeBSD libc Apply Changes Script
# This script applies compatibility fixes and synchronizes headers
# It can be run after any FreeBSD source updates or configuration changes

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FREEBSD_LIBC_ROOT="$(dirname "$SCRIPT_DIR")"
FREEBSD_SOURCE_DIR="$FREEBSD_LIBC_ROOT/freebsd-src"
TARGET_INCLUDE_DIR="$FREEBSD_LIBC_ROOT/freebsd-include"

echo "=== FreeBSD libc Apply Changes ==="
echo "Synchronizing headers and applying compatibility fixes..."

# Check if FreeBSD source exists
if [ ! -d "$FREEBSD_SOURCE_DIR" ]; then
    echo "ERROR: FreeBSD source not found. Please run setup.sh first."
    exit 1
fi

# Step 1: Sync headers from FreeBSD source
echo "Step 1: Synchronizing headers..."
if [ -f "$SCRIPT_DIR/sync_headers.sh" ]; then
    "$SCRIPT_DIR/sync_headers.sh"
else
    echo "Warning: sync_headers.sh not found, skipping header sync"
fi

# Step 2: Apply compatibility fixes
echo "Step 2: Applying compatibility fixes..."
if [ -f "$SCRIPT_DIR/fix_headers.sh" ]; then
    "$SCRIPT_DIR/fix_headers.sh"
else
    echo "Warning: fix_headers.sh not found, skipping compatibility fixes"
fi

# Step 3: Update documentation
echo "Step 3: Updating documentation..."
if [ -d "$TARGET_INCLUDE_DIR" ]; then
    # Count headers
    header_count=$(find "$TARGET_INCLUDE_DIR" -name "*.h" | wc -l)
    echo "✓ $header_count headers synchronized"
fi

# Step 4: Verify setup
echo "Step 4: Verifying setup..."
if [ -f "$SCRIPT_DIR/verify_setup.sh" ]; then
    "$SCRIPT_DIR/verify_setup.sh"
else
    echo "Warning: verify_setup.sh not found, skipping verification"
fi

echo ""
echo "=== Changes Applied Successfully! ==="
echo ""
echo "FreeBSD libc integration has been synchronized and updated."
echo ""
echo "Next steps:"
echo "1. Test compilation with your applications"
echo "2. Update glue layer if needed"
echo "3. Commit any important changes to version control"
echo ""