/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Empty namespace.h shim. FreeBSD's real one renames every public libc symbol
 * to an internal underscore alias, which assumes the full FreeBSD build. gdtoa
 * includes it only by convention and calls no renamed symbol, so an empty shim
 * suffices (same as our empty libc_private.h).
 */
#ifndef _NAMESPACE_H_
#define _NAMESPACE_H_
#endif /* _NAMESPACE_H_ */
