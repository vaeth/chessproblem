# This file is part of the chessproblem project and distributed under the
# terms of the GNU General Public License v2.
#
#   Copyright (c)
#     Martin V\"ath <martin@mvath.de>

AUTOMAKE_OPTIONS = foreign 1.11.2 no-dist-gzip dist-xz subdir-objects -Wobsolete -Wportability -Wextra-portability

ACLOCAL_AMFLAGS = -I m4 -I martinm4

bin_PROGRAMS = chessproblem/chessproblem

chessproblem_chessproblem_SOURCES = \
chessproblem/m_likely.h \
chessproblem/chess.cc \
chessproblem/chess.h \
chessproblem/chessproblem.cc \
chessproblem/chessproblem.h \
chessproblem/main.cc

chessproblem_chessproblem_CXXFLAGS = $(OSFORMAT_CFLAGS)

chessproblem_chessproblem_LDADD = $(OSFORMAT_LIBS) $(MULTITHREAD_LIBS)

TESTS = contrib/test.sh

# Stuff from our distribution
EXTRA_DIST = \
.gitignore \
CPPLINT.cfg \
autogen.sh \
contrib/cpplint.sh \
contrib/make.sh \
contrib/tarball.sh \
contrib/test.pl \
contrib/test.sh

CLEANFILES = chessproblem/chessproblem

AUTOCLEANFILES = \
Makefile.in \
aclocal.m4 \
ar-lib \
config.guess \
config.h.in~ \
config.log \
config.status \
config.sub \
configure \
compile \
depcomp \
install-sh \
ltmain.sh \
missing \
test-driver \
chessproblem*.asc \
chessproblem*.tar.bz2 \
chessproblem*.tar.gz \
chessproblem*.tar.xz \
chessproblem*.zip

AUTOCLEANDIRS = \
autom4te.cache \
build \
config \
m4 \
tmpwrk[0-9][0-9]*

# Remove autotool created stuff
.PHONY: autoclean
autoclean: maintainer-clean
	$(AM_V_at)cd "$(top_src_dir)"
	$(AM_V_at)$(RM) -vfr $(AUTOCLEANDIRS)
	$(AM_V_at)$(RM) -vf $(AUTOCLEANFILES)
