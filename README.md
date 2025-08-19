# FreeBSD libc for Unikraft

## Overview

FreeBSD-libc is a standard C library (libc) derived from the FreeBSD project, adapted to work with Unikraft. It provides a POSIX-compatible interface layer on top of the Linux system call API, enabling applications originally written for BSD-like environments to run within Unikraft unikernels. It is currently in progress to be integrated with Unikraft and, at this stage, can successfully build the `c-hello` application from the `catalog-core`.

While Musl is the default and recommended libc in Unikraft due to its lightweight design and built-in multithreading support, and Newlib is commonly used in embedded systems (requiring `pthread-embedded` for threads), FreeBSD-libc fills the gap for workloads that need stronger compatibility with BSD semantics.

Networking requires `lwip`, and additional Unikraft libraries may be combined as needed. Though still experimental, it lays the groundwork for running BSD-oriented applications and exploring libc diversity within unikernels.

## Using FreeBSD-libc with Unikraft

FreeBSD-libc is intended to serve as the standard C library for Unikraft applications, such as `c-hello`. To build applications with it, the build system must be configured to recognize the path to the locally cloned FreeBSD-libc repository. When FreeBSD-libc is enabled, Unikraft’s internal `nolibc` and `musl` libraries are automatically deselected.

### Steps to use freebsd-libc as an external library:

1.  **Clone the `freebsd-libc` repository into `catalog-core`**:

    Add the following lines to `catalog-core/setup.sh`:

    ```bash
    test -d repos/libs/freebsd-libc || \
git clone -b working_freebsd_libc --single-branch \
    https://github.com/PiyushPatle26/freebsd-libc.git repos/libs/freebsd-libc
    ```

2.  **Create the necessary directories and symlinks**:

    Add these lines to `catalog-core/setup.sh` and run `./setup.sh`:

    ```bash
    if ! test -d workdir/libs; then
        mkdir workdir/libs
    fi

    check_exists_and_create_symlink "libs/freebsd-libc"
    ```

3.  **Configure the `c-hello` application**:

    Modify `c-hello/Makefile` to use `freebsd-libc`. The final file should look like this:

    ```makefile
    UK_ROOT ?= $(PWD)/workdir/unikraft
    UK_BUILD ?= $(PWD)/workdir/build
    UK_APP ?= $(PWD)
    LIBS_BASE = $(PWD)/workdir/libs
    UK_LIBS ?= $(LIBS_BASE)/freebsd-libc

    .PHONY: all fetch-freebsd-libc

    all:
     @$(MAKE) -C $(UK_ROOT) L=$(UK_LIBS) A=$(UK_APP) O=$(UK_BUILD)

    fetch-freebsd-libc:
     @$(MAKE) -C $(UK_ROOT) L=$(UK_LIBS) A=$(UK_APP) O=$(UK_BUILD) libfreebsd-libc/fetch

    $(MAKECMDGOALS):
     @$(MAKE) -C $(UK_ROOT) L=$(UK_LIBS) A=$(UK_APP) O=$(UK_BUILD) $(MAKECMDGOALS)
    ```

4.  **Build and run the application**:

    ```bash
    # Configure the application
    make menuconfig

    # Select "freebsd-libc" under "Library Configuration"

    # Build the application
    make -j$(nproc)

    # Run the application with QEMU
    qemu-system-x86_64 -nographic -kernel workdir/build/c-hello_qemu-x86_64
    ```

## Directory Structure

The `freebsd-libc` repository is organized as follows:

*   `freebsd-src/`: Contains a clone of the FreeBSD source code. This directory is managed by the scripts in the `scripts/` directory and should not be modified manually.
*   `freebsd-include/`: A curated collection of headers from the FreeBSD source, patched for compatibility with Unikraft. This directory is also managed by the scripts.
*   `glue/`: Contains the "glue" code that provides the compatibility layer between FreeBSD libc and Unikraft. This includes syscall redirections, TLS and TCB implementations, and compatibility headers.
*   `patches/`: Contains patch files that are applied to the FreeBSD source code to fix compatibility issues.
*   `scripts/`: Contains a consolidated script, `manage.sh`, for setting up, updating, and verifying the FreeBSD libc integration.
*   `Config.uk`: The Kconfig file that defines the `menuconfig` options for `freebsd-libc`.
*   `Makefile.uk`: The makefile that integrates `freebsd-libc` with the Unikraft build system.
*   `README.md`: This file.
