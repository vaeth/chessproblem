// This file is part of the chessproblem project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_M_LIKELY_H_
#define SRC_M_LIKELY_H_ 1

#include <config.h>

#ifdef HAVE___BUILTIN_EXPECT
#define LIKELY(x)    __builtin_expect((x), 1)
#define UNLIKELY(x)  __builtin_expect((x), 0)
#else
#define LIKELY(x)    (x)
#define UNLIKELY(x)  (x)
#endif

#endif  // SRC_M_LIKELY_H_
