# This file is part of the chessproblem project and distributed under the
# terms of the GNU General Public License v2.
# SPDX-License-Identifier: GPL-2.0-only
#
#   Copyright (c)
#     Martin V\"ath <martin@mvath.de>

AUTOMAKE_OPTIONS = foreign 1.11.2 no-dist-gzip dist-xz subdir-objects -Wobsolete -Wportability -Wextra-portability

CLEANFILES = chessproblem

chessproblem_SOURCES = \
src/m_likely.h \
src/formated.cc \
src/formated.h \
src/chess.cc \
src/chess.h \
src/chessproblem.cc \
src/chessproblem.h \
src/main.cc

bin_PROGRAMS = chessproblem

# Stuff from our distribution
EXTRA_DIST = \
contrib/clean.sh \
contrib/cpplint.sh \
contrib/make.sh \
contrib/tarball.sh \
contrib/test.pl \
.gitignore \
autogen.sh \
CPPLINT.cfg

ACLOCAL_AMFLAGS = -I m4 -I martinm4

# Remove things created by autogen.sh
distclean-local:
	$(AM_V_at)$(RM) -vfr "$(top_srcdir)/build" \
		"$(top_srcdir)/config" \
		"$(top_srcdir)/m4" \
		"$(top_srcdir)/autom4te.cache"
	$(AM_V_at)$(RM) -vf \
		"$(top_srcdir)/aclocal.m4" \
		"$(top_srcdir)/Makefile.in" \
		"$(top_srcdir)/configure" \
		"$(top_srcdir)/config.log" \
		"$(top_srcdir)/config.status" \
		"$(top_srcdir)/config.h.in~"

# Remove rpo files (-frepo with gcc) on clean
clean-local:
	$(AM_V_at)find "$(top_srcdir)" -name '*.rpo' -exec $(RM) -v -- '{}' '+'
