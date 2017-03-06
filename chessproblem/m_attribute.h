// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef CHESSPROBLEM_M_ATTRIBUTE_H_
#define CHESSPROBLEM_M_ATTRIBUTE_H_ 1

#include <config.h>

#ifdef HAVE_C11ATTRIBUTE_DEPRECATED
#define ATTRIBUTE_DEPRECATED(a) [[deprecated(a)]]
#else
#ifdef HAVE_ATTRIBUTE_DEPRECATED
#define ATTRIBUTE_DEPRECATED(a) __attribute__ ((deprecated(a)))
#else
#define ATTRIBUTE_DEPRECATED(a)
#endif  // HAVE_ATTRIBUTE_DEPRECATED
#endif  // HAVE_C11ATTRIBUTE_DEPRECATED

#ifdef HAVE_C11ATTRIBUTE_NORETURN
#define ATTRIBUTE_NORETURN [[noreturn]]  // NOLINT(whitespace/braces)
#else
#ifdef HAVE_ATTRIBUTE_NORETURN
#define ATTRIBUTE_NORETURN __attribute__ ((noreturn))
#else
#define ATTRIBUTE_NORETURN
#endif  // HAVE_ATTRIBUTE_NORETURN
#endif  // HAVE_C11ATTRIBUTE_NORETURN

#ifdef HAVE_ATTRIBUTE_NODISCARD
#define ATTRIBUTE_NODISCARD [[nodiscard]]  // NOLINT(whitespace/braces)
#else
#ifdef HAVE_ATTRIBUTE_WARN_UNUSED_RESULT
#define ATTRIBUTE_NODISCARD __attribute__ ((warn_unused_result))
#else
#define ATTRIBUTE_NODISCARD
#endif  // HAVE_ATTRIBUTE_WARN_UNUSED_RESULT
#endif  // HAVE_ATTRIBUTE_NODISCARD

#ifdef HAVE_ATTRIBUTE_FALLTHROUGH
#define ATTRIBUTE_FALLTHROUGH [[fallthrough]];  // NOLINT(whitespace/braces)
#else
#define ATTRIBUTE_FALLTHROUGH
#endif

#ifdef HAVE_ATTRIBUTE_CONST
#define ATTRIBUTE_CONST __attribute__ ((const))
#else
#define ATTRIBUTE_CONST
#endif

#ifdef HAVE_ATTRIBUTE_PURE
#define ATTRIBUTE_PURE __attribute__ ((pure))
#else
#define ATTRIBUTE_PURE
#endif

#ifdef HAVE_ATTRIBUTE_NONNULL_
#define ATTRIBUTE_NONNULL_ __attribute__ ((nonnull))
#else
#define ATTRIBUTE_NONNULL_
#endif

#ifdef HAVE_ATTRIBUTE_NONNULL
#define ATTRIBUTE_NONNULL(a) __attribute__ ((nonnull a))
#else
#define ATTRIBUTE_NONNULL(a)
#endif


#endif  // CHESSPROBLEM_M_ATTRIBUTE_H_
