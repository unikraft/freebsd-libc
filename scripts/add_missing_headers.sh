#!/bin/bash
#
# Script to add missing FreeBSD headers to the curated header collection
# This follows the patching strategy - we don't modify FreeBSD source, 
# but we ensure all necessary headers are available in freebsd-include/
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASE_DIR="$(dirname "$SCRIPT_DIR")"
FREEBSD_SRC="$BASE_DIR/freebsd-src"
FREEBSD_INCLUDE="$BASE_DIR/freebsd-include"

echo "Adding missing FreeBSD headers to curated collection..."

# Check if source directories exist
if [ ! -d "$FREEBSD_SRC" ]; then
    echo "Error: FreeBSD source directory not found: $FREEBSD_SRC"
    exit 1
fi

if [ ! -d "$FREEBSD_INCLUDE" ]; then
    echo "Error: FreeBSD include directory not found: $FREEBSD_INCLUDE"
    exit 1
fi

# Helper function to copy a header if it's missing
copy_header() {
    local src_path="$1"
    local dest_dir="$2"
    local header_name="$(basename "$src_path")"

    mkdir -p "$dest_dir"

    if [ ! -f "$dest_dir/$header_name" ]; then
        if [ -f "$src_path" ]; then
            echo "Adding $header_name to $dest_dir..."
            cp "$src_path" "$dest_dir/"
        else
            echo "Warning: Source file not found: $src_path"
        fi
    fi
}

# Copy sys/_sockaddr_storage.h
copy_header "$FREEBSD_SRC/sys/sys/_sockaddr_storage.h" "$FREEBSD_INCLUDE/sys"

# Copy param.h files
copy_header "$FREEBSD_SRC/sys/sys/param.h" "$FREEBSD_INCLUDE/sys"
copy_header "$FREEBSD_SRC/sys/amd64/include/param.h" "$FREEBSD_INCLUDE/machine"
copy_header "$FREEBSD_SRC/sys/arm64/include/param.h" "$FREEBSD_INCLUDE/machine"
copy_header "$FREEBSD_SRC/sys/i386/include/param.h" "$FREEBSD_INCLUDE/machine"
copy_header "$FREEBSD_SRC/sys/powerpc/include/param.h" "$FREEBSD_INCLUDE/machine"
copy_header "$FREEBSD_SRC/sys/riscv/include/param.h" "$FREEBSD_INCLUDE/machine"


# Create ssp directory if it doesn't exist
mkdir -p "$FREEBSD_INCLUDE/ssp"

# Copy missing SSP headers that are needed for compilation
missing_ssp_headers=("socket.h" "random.h" "wchar.h")

for header in "${missing_ssp_headers[@]}"; do
    if [ ! -f "$FREEBSD_INCLUDE/ssp/$header" ]; then
        if [ -f "$FREEBSD_SRC/include/ssp/$header" ]; then
            echo "Adding ssp/$header..."
            cp "$FREEBSD_SRC/include/ssp/$header" "$FREEBSD_INCLUDE/ssp/"
        else
            echo "Warning: Source file not found: $FREEBSD_SRC/include/ssp/$header"
        fi
    fi
done

echo "Missing headers added successfully."
