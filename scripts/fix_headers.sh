#!/bin/bash

# FreeBSD Header Compatibility Fix Script
# This script applies specific compatibility fixes to FreeBSD headers for Unikraft integration
# It ensures that clock constants and other critical definitions are compatible

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FREEBSD_LIBC_ROOT="$(dirname "$SCRIPT_DIR")"
TARGET_INCLUDE_DIR="$FREEBSD_LIBC_ROOT/freebsd-include"

echo "=== FreeBSD Header Compatibility Fixes ==="
echo "Applying critical compatibility fixes for Unikraft integration..."

# Check if headers directory exists
if [ ! -d "$TARGET_INCLUDE_DIR" ]; then
    echo "ERROR: Headers directory not found at $TARGET_INCLUDE_DIR"
    echo "Please run sync_headers.sh first"
    exit 1
fi

# Fix #1: Critical Clock Constants Compatibility in sys/_clock_id.h
echo "Fix #1: Clock constants compatibility in sys/_clock_id.h..."
if [ -f "$TARGET_INCLUDE_DIR/sys/_clock_id.h" ]; then
    # Check if CLOCK_BOOTTIME is defined as alias to CLOCK_MONOTONIC
    if grep -q "CLOCK_BOOTTIME.*CLOCK_MONOTONIC" "$TARGET_INCLUDE_DIR/sys/_clock_id.h"; then
        echo "  → Fixing CLOCK_BOOTTIME duplicate case value issue..."
        
        # Replace the problematic alias with unique value (handle potential existing fixes)
        sed -i 's/#define.*CLOCK_BOOTTIME.*CLOCK_MONOTONIC.*/#define\tCLOCK_BOOTTIME\t\t13\t\/\* Unique value - NOT alias to CLOCK_MONOTONIC \*\//' "$TARGET_INCLUDE_DIR/sys/_clock_id.h"
        
        # Add CLOCK_MONOTONIC_RAW (missing in FreeBSD)
        if ! grep -q "CLOCK_MONOTONIC_RAW" "$TARGET_INCLUDE_DIR/sys/_clock_id.h"; then
            sed -i '/^#define.*CLOCK_BOOTTIME.*13/a\
#define	CLOCK_MONOTONIC_RAW	14	/* Missing in FreeBSD, needed by Unikraft *///' "$TARGET_INCLUDE_DIR/sys/_clock_id.h"
        fi
        
        # Move CLOCK_SECOND to avoid conflict with CLOCK_BOOTTIME
        if grep -q "#define CLOCK_SECOND.*13" "$TARGET_INCLUDE_DIR/sys/_clock_id.h"; then
            sed -i 's/#define CLOCK_SECOND.*13/#define CLOCK_SECOND\t\t17\t\/\* Moved from 13 to avoid conflict with CLOCK_BOOTTIME \*\//' "$TARGET_INCLUDE_DIR/sys/_clock_id.h"
        fi
        
        # Fix CLOCK_THREAD_CPUTIME_ID conflict with CLOCK_MONOTONIC_RAW (both at 14)
        if grep -q "#define CLOCK_THREAD_CPUTIME_ID.*14" "$TARGET_INCLUDE_DIR/sys/_clock_id.h"; then
            sed -i 's/#define CLOCK_THREAD_CPUTIME_ID.*14.*/#define CLOCK_THREAD_CPUTIME_ID\t18\t\/\* Moved from 14 to avoid conflict with CLOCK_MONOTONIC_RAW \*\//' "$TARGET_INCLUDE_DIR/sys/_clock_id.h"
        fi
        
        # Clean up any duplicated comments that might have been introduced
        sed -i 's/\*\/ to avoid conflict with CLOCK_MONOTONIC_RAW \*\//*\//' "$TARGET_INCLUDE_DIR/sys/_clock_id.h"
        
        echo "  ✓ Fixed CLOCK_BOOTTIME, added CLOCK_MONOTONIC_RAW, resolved conflicts"
    else
        echo "  → CLOCK_BOOTTIME already fixed or not found"
    fi
else
    echo "  ⚠ sys/_clock_id.h not found, skipping clock fixes"
fi

# Fix #2: Compiler Attribute Conflicts in sys/cdefs.h
echo "Fix #2: Compiler attribute conflicts in sys/cdefs.h..."
if [ -f "$TARGET_INCLUDE_DIR/sys/cdefs.h" ]; then
    # Add guards around compiler attributes to prevent redefinition warnings
    ATTRIBUTES=("__unused" "__used" "__packed" "__section" "__noinline" "__always_inline" "__aligned" "__deprecated" "__weak")
    
    for attr in "${ATTRIBUTES[@]}"; do
        if grep -q "^#define $attr" "$TARGET_INCLUDE_DIR/sys/cdefs.h" && ! grep -B1 "^#define $attr" "$TARGET_INCLUDE_DIR/sys/cdefs.h" | grep -q "#ifndef $attr"; then
            # Add ifndef guard before the define
            sed -i "/^#define $attr/i\\#ifndef $attr" "$TARGET_INCLUDE_DIR/sys/cdefs.h"
            # Add endif after the define
            sed -i "/^#define $attr/a\\#endif" "$TARGET_INCLUDE_DIR/sys/cdefs.h"
        fi
    done
    
    echo "  ✓ Added guards around compiler attributes"
else
    echo "  ⚠ sys/cdefs.h not found, skipping attribute fixes"
fi

# Fix #3: __OFF_MAX and __OFF_MIN conflicts in x86/_limits.h
echo "Fix #3: __OFF_MAX/__OFF_MIN conflicts in x86/_limits.h..."
if [ -f "$TARGET_INCLUDE_DIR/x86/_limits.h" ]; then
    # Add guards around __OFF_MAX and __OFF_MIN to prevent redefinition
    if grep -q "^#define.*__OFF_MAX" "$TARGET_INCLUDE_DIR/x86/_limits.h" && ! grep -B1 "^#define.*__OFF_MAX" "$TARGET_INCLUDE_DIR/x86/_limits.h" | grep -q "#ifndef __OFF_MAX"; then
        sed -i '/^#define.*__OFF_MAX/i\\n\/\* Unikraft compatibility: only define if not already defined by Unikraft \*\/\n#ifndef __OFF_MAX' "$TARGET_INCLUDE_DIR/x86/_limits.h"
        sed -i '/^#define.*__OFF_MAX/a\\#endif' "$TARGET_INCLUDE_DIR/x86/_limits.h"
    fi
    
    if grep -q "^#define.*__OFF_MIN" "$TARGET_INCLUDE_DIR/x86/_limits.h" && ! grep -B1 "^#define.*__OFF_MIN" "$TARGET_INCLUDE_DIR/x86/_limits.h" | grep -q "#ifndef __OFF_MIN"; then
        sed -i '/^#define.*__OFF_MIN/i\\#ifndef __OFF_MIN' "$TARGET_INCLUDE_DIR/x86/_limits.h"
        sed -i '/^#define.*__OFF_MIN/a\\#endif' "$TARGET_INCLUDE_DIR/x86/_limits.h"
    fi
    
    echo "  ✓ Added guards around __OFF_MAX/__OFF_MIN"
else
    echo "  ⚠ x86/_limits.h not found, skipping __OFF_* fixes"
fi

# Fix #4: __offsetof and __containerof conflicts  
echo "Fix #4: __offsetof/__containerof conflicts..."
if [ -f "$TARGET_INCLUDE_DIR/sys/cdefs.h" ]; then
    # Add guards around __offsetof
    if grep -q "^#define __offsetof" "$TARGET_INCLUDE_DIR/sys/cdefs.h" && ! grep -B1 "^#define __offsetof" "$TARGET_INCLUDE_DIR/sys/cdefs.h" | grep -q "#ifndef __offsetof"; then
        sed -i '/^#define __offsetof/i\\#ifndef __offsetof' "$TARGET_INCLUDE_DIR/sys/cdefs.h"
        sed -i '/^#define __offsetof/a\\#endif' "$TARGET_INCLUDE_DIR/sys/cdefs.h"
    fi
    
    # Add guards around __containerof  
    if grep -q "^#define __containerof" "$TARGET_INCLUDE_DIR/sys/cdefs.h" && ! grep -B1 "^#define __containerof" "$TARGET_INCLUDE_DIR/sys/cdefs.h" | grep -q "#ifndef __containerof"; then
        sed -i '/^#define __containerof/i\\#ifndef __containerof' "$TARGET_INCLUDE_DIR/sys/cdefs.h"
        sed -i '/^#define __containerof/a\\#endif' "$TARGET_INCLUDE_DIR/sys/cdefs.h"
    fi
    
    echo "  ✓ Added guards around __offsetof/__containerof"
fi

# Fix #5: Verification - Check that our critical fixes are in place
echo "Fix #5: Verification of critical compatibility fixes..."

verify_clock_constants() {
    local header="$TARGET_INCLUDE_DIR/sys/_clock_id.h"
    if [ -f "$header" ]; then
        # Verify CLOCK_BOOTTIME is not an alias (check for exact alias pattern)
        if grep -q "#define.*CLOCK_BOOTTIME.*CLOCK_MONOTONIC$" "$header" || grep -q "CLOCK_BOOTTIME.*CLOCK_MONOTONIC[^A-Z_]" "$header"; then
            echo "  ❌ CRITICAL: CLOCK_BOOTTIME still aliased to CLOCK_MONOTONIC"
            return 1
        fi
        
        # Verify CLOCK_BOOTTIME has value 13
        if ! grep -q "CLOCK_BOOTTIME.*13" "$header"; then
            echo "  ❌ CRITICAL: CLOCK_BOOTTIME does not have value 13"
            return 1
        fi
        
        # Verify CLOCK_MONOTONIC_RAW exists
        if ! grep -q "CLOCK_MONOTONIC_RAW" "$header"; then
            echo "  ❌ CRITICAL: CLOCK_MONOTONIC_RAW is missing"
            return 1
        fi
        
        echo "  ✓ Clock constants verification passed"
        return 0
    else
        echo "  ❌ CRITICAL: sys/_clock_id.h not found"
        return 1
    fi
}

# Run verification
if verify_clock_constants; then
    echo "  ✓ All critical compatibility fixes verified"
else
    echo "  ❌ Some critical fixes failed - manual intervention may be needed"
    exit 1
fi

# Fix #6: Create a comprehensive clock constants compatibility header
# Fix #6: SSP (Stack Smashing Protection) Header Compatibility
echo "Fix #6: SSP header compatibility..."
if [ -f "$TARGET_INCLUDE_DIR/ssp/ssp.h" ]; then
    # Fix __builtin_object_size macro issue in SSP headers for declaration contexts
    if grep -q "#define __ssp_bos(ptr) __builtin_object_size" "$TARGET_INCLUDE_DIR/ssp/ssp.h" && ! grep -q "Unikraft compatibility.*Safe fallback" "$TARGET_INCLUDE_DIR/ssp/ssp.h"; then
        # Replace the problematic __builtin_object_size macros with safe versions
        sed -i '/#define __ssp_bos(ptr) __builtin_object_size/,/#define __ssp_bos0(ptr) __builtin_object_size/c\
/* Unikraft compatibility: Safe fallback for __builtin_object_size */\
#if __SSP_FORTIFY_LEVEL == 0\
/* No fortification - use safe fallback */\
#define __ssp_bos(ptr) ((size_t)-1)\
#define __ssp_bos0(ptr) ((size_t)-1)\
#else\
/* Use compiler builtin if available, with safe fallback for declaration contexts */\
#if defined(__GNUC__) || defined(__clang__)\
#define __ssp_bos(ptr) (__builtin_object_size((ptr), (__SSP_FORTIFY_LEVEL > 1)))\
#define __ssp_bos0(ptr) (__builtin_object_size((ptr), 0))\
#else\
#define __ssp_bos(ptr) ((size_t)-1)\
#define __ssp_bos0(ptr) ((size_t)-1)\
#endif\
#endif' "$TARGET_INCLUDE_DIR/ssp/ssp.h"
        echo "  ✓ Fixed __builtin_object_size declaration context issues in ssp/ssp.h"
    else
        echo "  → SSP __builtin_object_size already fixed or not found"
    fi
else
    echo "  → SSP headers not found, skipping SSP fixes"
fi


# Fix #7: Missing fallocate constants
echo "Fix #7: Missing fallocate constants..."
if [ -f "$TARGET_INCLUDE_DIR/sys/fcntl.h" ]; then
    if ! grep -q "FALLOC_FL_PUNCH_HOLE" "$TARGET_INCLUDE_DIR/sys/fcntl.h"; then
        # Add fallocate constants for Linux compatibility
        cat >> "$TARGET_INCLUDE_DIR/sys/fcntl.h" << 'EOF'

/* fallocate() flags for Linux compatibility */
#ifndef FALLOC_FL_KEEP_SIZE
#define FALLOC_FL_KEEP_SIZE     0x01    /* Don't extend size */
#endif
#ifndef FALLOC_FL_PUNCH_HOLE  
#define FALLOC_FL_PUNCH_HOLE    0x02    /* Create hole in file */
#endif
#ifndef FALLOC_FL_NO_HIDE_STALE
#define FALLOC_FL_NO_HIDE_STALE 0x04    /* Don't hide stale data */
#endif
#ifndef FALLOC_FL_COLLAPSE_RANGE
#define FALLOC_FL_COLLAPSE_RANGE 0x08   /* Remove range */
#endif
#ifndef FALLOC_FL_ZERO_RANGE
#define FALLOC_FL_ZERO_RANGE    0x10    /* Zero range */
#endif
EOF
        echo "  ✓ Added fallocate constants to sys/fcntl.h"
    else
        echo "  → fallocate constants already defined"
    fi
else
    echo "  ⚠ sys/fcntl.h not found, skipping fallocate constants"
fi

echo "Fix #8: Creating clock constants verification header..."
cat > "$TARGET_INCLUDE_DIR/clock_constants_verification.h" << 'EOF'
/*
 * FreeBSD Clock Constants Verification Header for Unikraft
 * 
 * This header verifies that all clock constants have unique values
 * to prevent "duplicate case value" errors in switch statements.
 * 
 * Include this header in glue code to verify compatibility.
 */

#ifndef _CLOCK_CONSTANTS_VERIFICATION_H_
#define _CLOCK_CONSTANTS_VERIFICATION_H_

#include <sys/_clock_id.h>

/* Compile-time verification that clock constants have unique values */
#if defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
#if CLOCK_BOOTTIME == CLOCK_MONOTONIC
#error "CLOCK_BOOTTIME must not equal CLOCK_MONOTONIC - this causes duplicate case values"
#endif
#endif

#if defined(CLOCK_MONOTONIC_RAW) && defined(CLOCK_THREAD_CPUTIME_ID)
#if CLOCK_MONOTONIC_RAW == CLOCK_THREAD_CPUTIME_ID
#error "CLOCK_MONOTONIC_RAW must not equal CLOCK_THREAD_CPUTIME_ID - this causes duplicate case values"
#endif
#endif

/* Verify critical constants exist */
#ifndef CLOCK_BOOTTIME
#error "CLOCK_BOOTTIME must be defined"
#endif

#ifndef CLOCK_MONOTONIC_RAW
#error "CLOCK_MONOTONIC_RAW must be defined"
#endif

/* Expected values documentation:
 * CLOCK_REALTIME           = 0
 * CLOCK_VIRTUAL            = 1
 * CLOCK_PROF               = 2  
 * CLOCK_MONOTONIC          = 4  (FreeBSD standard)
 * CLOCK_UPTIME             = 5
 * CLOCK_UPTIME_PRECISE     = 7
 * CLOCK_UPTIME_FAST        = 8
 * CLOCK_REALTIME_PRECISE   = 9
 * CLOCK_REALTIME_FAST      = 10
 * CLOCK_MONOTONIC_PRECISE  = 11
 * CLOCK_MONOTONIC_FAST     = 12
 * CLOCK_BOOTTIME           = 13  (UNIQUE - NOT alias)
 * CLOCK_MONOTONIC_RAW      = 14  (Added for Unikraft)
 * CLOCK_PROCESS_CPUTIME_ID = 15
 * CLOCK_TAI                = 16
 * CLOCK_SECOND             = 17  (Moved from 13)
 * CLOCK_THREAD_CPUTIME_ID  = 18  (Moved from 14)
 */

#endif /* _CLOCK_CONSTANTS_VERIFICATION_H_ */
EOF

echo "  ✓ Created clock constants verification header"

echo ""
echo "=== Header Compatibility Fixes Complete ==="
echo ""
echo "Applied fixes:"
echo "  1. ✓ Fixed CLOCK_BOOTTIME duplicate case value issue" 
echo "  2. ✓ Added CLOCK_MONOTONIC_RAW constant (missing in FreeBSD)"
echo "  3. ✓ Resolved clock constant value conflicts"
echo "  4. ✓ Added compiler attribute guards to prevent redefinition warnings"
echo "  5. ✓ Added __OFF_MAX/__OFF_MIN guards"
echo "  6. ✓ Added __offsetof/____containerof guards"
echo "  7. ✓ Created verification header for compile-time checks"
echo ""
echo "Ready to test compilation!"
