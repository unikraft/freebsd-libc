#!/bin/bash
#
# Master setup script for FreeBSD libc integration.
# This script calls all other necessary scripts in the correct order.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# --- Helper Functions ---
log() {
    echo "==> $1"
}

run_script() {
    if [ -f "${SCRIPT_DIR}/$1" ]; then
        log "Running $1..."
        bash "${SCRIPT_DIR}/$1"
    else
        echo "ERROR: Script not found: $1" >&2
        exit 1
    fi
}

# --- Main Workflows ---

do_setup() {
    log "Starting complete FreeBSD libc setup..."
    run_script "install_freebsd_source.sh"
    run_script "add_missing_headers.sh"
    run_script "apply_patches.sh"
    run_script "verify_setup.sh"
    log "Setup complete."
}

do_update() {
    log "Updating FreeBSD libc integration..."
    run_script "add_missing_headers.sh"
    run_script "apply_patches.sh"
    run_script "verify_setup.sh"
    log "Update complete."
}

# --- Script Entry Point ---

# Check if any arguments were provided before trying to access them.
if [ "$#" -eq 0 ]; then
    echo "Usage: $0 {setup|update}"
    echo "  setup:  Performs a full, clean setup."
    echo "  update: Updates headers and applies fixes."
    exit 1
fi

case "$1" in
    setup)
        do_setup
        ;;
    update)
        do_update
        ;;
    *)
        echo "Invalid command: $1" >&2
        echo "Usage: $0 {setup|update}"
        exit 1
        ;;
esac
