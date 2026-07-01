/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal <fenv.h> shim. gdtoa includes it unconditionally but only calls
 * fegetround() under -DHonor_FLT_ROUNDS, which this port doesn't set. nolibc
 * ships no <fenv.h>, so we provide just enough to resolve the include plus a
 * round-to-nearest fegetround().
 */
#ifndef _FENV_H_
#define _FENV_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int fenv_t;
typedef int fexcept_t;

#define FE_TONEAREST  0x0000
#define FE_DOWNWARD   0x0400
#define FE_UPWARD     0x0800
#define FE_TOWARDZERO 0x0c00

static inline int fegetround(void) { return FE_TONEAREST; }
static inline int fesetround(int r) { (void)r; return 0; }

#ifdef __cplusplus
}
#endif

#endif /* _FENV_H_ */
