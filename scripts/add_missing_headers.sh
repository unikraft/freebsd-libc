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

# Create sys directory if it doesn't exist
mkdir -p "$FREEBSD_INCLUDE/sys"

# Copy missing sys/_sockaddr_storage.h
if [ ! -f "$FREEBSD_INCLUDE/sys/_sockaddr_storage.h" ]; then
    if [ -f "$FREEBSD_SRC/sys/sys/_sockaddr_storage.h" ]; then
        echo "Adding sys/_sockaddr_storage.h..."
        cp "$FREEBSD_SRC/sys/sys/_sockaddr_storage.h" "$FREEBSD_INCLUDE/sys/"
    else
        echo "Warning: Source file not found: $FREEBSD_SRC/sys/sys/_sockaddr_storage.h"
    fi
fi

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