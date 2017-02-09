// This file is part of the chessproblem project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_M_ASSERT_H_
#define SRC_M_ASSERT_H_ 1

#include <config.h>

#ifndef NDEBUG

#include <cassert>
#define ASSERT(a) assert(a)

#else  // NDEBUG

#ifndef ASSERT
#define ASSERT(a)
#endif

#endif  // NDEBUG

#endif  // SRC_M_ASSERT_H_
