# This file is part of the chessproblem project and distributed under the
# terms of the GNU General Public License v2.
#
# Copyright (c)
#  Martin V\"ath <martin@mvath.de>

AC_INIT([chessproblem], [1.0], [https://github.com/vaeth/chessproblem/issues/], [chessproblem], [https://github.com/vaeth/chessproblem/])
AC_PREREQ([2.64])

m4_ifdef([AC_CONFIG_MACRO_DIR],
	[AC_CONFIG_MACRO_DIR([m4])])
m4_ifdef([AC_CONFIG_MACRO_DIRS],
	[AC_CONFIG_MACRO_DIRS([m4 martinm4])])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_FILES([
		Makefile
	])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_TARGET()

AM_MAINTAINER_MODE()
AM_INIT_AUTOMAKE([no-dependencies])
# disable --disable-silent-rules to omit useless output with Gentoo's EAPI=5:
AS_VAR_SET([enable_silent_rules], [yes])
AM_SILENT_RULES([yes])

AC_LANG([C++])
AC_PROG_CXX()

# Now our flag mangling options:

AC_ARG_ENABLE([debugging],
	[AS_HELP_STRING([--enable-debugging],
		[activate paranoic asserts and flags for debugging])],
	[MV_ENABLE([debugging])
	AS_VAR_SET([cmt_debugging], ["on request"])],
	[AS_VAR_SET([debugging], [false])
	AS_VAR_SET([cmt_debugging], ["default"])])

AC_ARG_ENABLE([new_dialect],
	[AS_HELP_STRING([--enable-new-dialect],
		[enable most current C++ dialect])],
	[MV_ENABLE([new_dialect])
	AS_VAR_SET([cmt_new_dialect], ["on request"])],
	[AS_VAR_SET([new_dialect], [false])
	AS_VAR_SET([cmt_new_dialect], ["default"])])

AC_ARG_ENABLE([optimization],
	[AS_HELP_STRING([--enable-optimization],
		[append optimization flags])],
	[MV_ENABLE([optimization])
	AS_VAR_SET([cmt_optimization], ["on request"])],
	[AS_VAR_SET([optimization], [false])
	AS_VAR_SET([cmt_optimization], ["default"])])

AC_ARG_ENABLE([strong_optimization],
	[AS_HELP_STRING([--enable-strong-optimization],
		[use also optimization flags which might not work on broken compilers])],
	[MV_ENABLE([strong-optimization])
	AS_VAR_SET([cmt_strong_optimization], ["on request"])],
	[AS_VAR_SET([strong_optimization], [false])
	AS_VAR_SET([cmt_strong_optimization], ["default"])])

AC_ARG_ENABLE([warnings],
	[AS_HELP_STRING([--enable-warnings],
		[append warning/testing flags; might produce worse code])],
	[MV_ENABLE([warnings])
	AS_VAR_SET([cmt_warnings], ["on request"])],
	[AS_VAR_SET([warnings], [false])
	AS_VAR_SET([cmt_warnings], ["default"])])

# Some implicit dependencies of these options.
# We do this here explicitly and output the results

AC_MSG_CHECKING([whether debugging asserts/flags should be used])
MV_MSG_RESULT_BIN([$debugging], [$cmt_debugging])

AC_MSG_CHECKING([whether strong optimization options should be used])
MV_MSG_RESULT_BIN([$strong_optimization], [$cmt_strong_optimization])

AC_MSG_CHECKING([whether optimization options should be used])
AS_IF([$strong_optimization],
	[AS_VAR_SET([optimization], [:])
	AS_VAR_SET([cmt_optimization], ["forced by strong optimization"])])
AS_IF([$debugging],
	[AS_VAR_SET([optimization], [false])
	AS_VAR_SET([cmt_optimization], ["disabled by debugging"])])
MV_MSG_RESULT_BIN([$optimization], [$cmt_optimization])

AC_MSG_CHECKING([whether most current C++ dialect options should be used])
AS_IF([$optimization],
	[AS_VAR_SET([new_dialect], [:])
	AS_VAR_SET([cmt_new_dialect], ["forced by optimization"])])
MV_MSG_RESULT_BIN([$new_dialect], [$cmt_new_dialect])

AC_MSG_CHECKING([whether warning options should be used])
MV_MSG_RESULT_BIN([$warnings], [$cmt_warnings])

# Now start the flag mangling:
AS_IF([$debugging],
	[AS_VAR_SET([CFLAGS], [])
	AS_VAR_SET([CXXFLAGS], [])
	AS_VAR_SET([LDFLAGS], [])])
AS_VAR_COPY([oricxxflags], [CXXFLAGS])
AS_VAR_COPY([orildflags], [LDFLAGS])

# Note that the (potential) resetting of the flags was necessary *before*
# (potentially) modifying flags for system extensions.
# On the other hand, we must check for system extensions before we compile
# test programs to find out the other flags.
AC_USE_SYSTEM_EXTENSIONS()

# Only now the flag mangling can continue:
#
# At first, we must calculate our [fatal-flags] and [mode] arguments
AS_VAR_SET([my_cxxfatal], [])
AS_VAR_SET([my_ldfatal], [])
MV_ADDFLAGS([my_cxxfatal], [CXXFLAGS], [AC_COMPILE_IFELSE], [ \
		-Werror \
		-Werror=unknown-warning-option \
		-Wunknown-warning-option \
	])
MV_ADDFLAGS([my_ldfatal], [LDFLAGS], [AC_LINK_IFELSE], [ \
		$my_cxxfatal \
		-Wl,--fatal-warnings \
	])

# Now the actual testing:
AS_VAR_SET([my_cxxadd], [])
AS_VAR_SET([my_ldadd], [])

# Prefer the newest available most fancy C++ dialect to detect problems early
AS_IF([$new_dialect],
	[AS_VAR_SET([try_dialect], ["
		-std=gnu++17
		-std=c++17
		-std=gnu++1z
		-std=c++1z
		-std=gnu++14
		-std=c++14
		-std=gnu++1y
		-std=c++1y
		-std=gnu++11
		-std=c++11
"])],
	[AS_VAR_SET([try_dialect], ["
		-std=gnu++11
		-std=c++11
"])])
AS_IF([$new_dialect],
	[AS_CASE([" $CXXFLAGS $my_cxxadd"],
		[*" -std="*], [],
		[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_LINK],
			[$try_dialect], [$my_cxxfatal], [], [break])])])
AS_VAR_COPY([my_cxxdialect], [my_cxxadd])

AS_IF([$debugging],
	[MV_ADDFLAGS([my_ldadd], [CXXFLAGS], [MV_RUN_IFELSE_LINK], [ \
			-g \
			-ggdb3 \
		], [$my_ldfatal], [], [:])
	MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_LINK], [ \
			-g \
			-ggdb3 \
		], [$my_cxxfatal], [], [:])])
AS_IF([$optimization],
	[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_LINK], [ \
			-O2 \
			-fomit-frame-pointer \
			-fno-common \
			-fstrict-aliasing \
			-fstrict-enums \
			-fmerge-all-constants \
			-finline-functions \
			-fvisibility=hidden \
			-fvisibility-inlines-hidden \
			-ftree-pre \
			-ftree-partial-pre \
			-fnothrow-opt \
			-frename-registers \
			-funsafe-loop-optimizations \
			-fgcse-sm \
			-fgcse-las \
			-fgcse-after-reload \
			-fpredictive-commoning \
			-ftree-switch-conversion \
			-fno-ident \
			-freorder-functions \
			-fdevirtualize-speculatively \
			-fdevirtualize-at-ltrans \
			-fno-semantic-interposition \
			-fira-loop-pressure \
			-ftree-loop-distribution \
			-ftree-loop-distribute-patterns \
			-ftree-loop-im \
			-fivopts \
			-fdelete-null-pointer-checks \
		], [$my_cxxfatal], [], [:])
	MV_ADDFLAGS([my_ldadd], [LDFLAGS], [MV_RUN_IFELSE_FALSE], [ \
			-Wl,-O1 \
			-Wl,--relax \
			-Wl,--as-needed \
			-Wl,--sort-common \
			-Wl,-z,combreloc \
		], [$my_ldfatal], [], [:])])
AS_IF([$strong_optimization],
	[AS_VAR_SET([flto], ["
		-flto \
		-flto-partition=none \
		-flto-odr-type-merging \
		-emit-llvm \
		"])
# We first modify the LDFLAGS: This is important because of -flto:
# Without -flto in LDFLAGS, -flto in CXXFLAGS might not fail even if it
# should produce broken code.
	MV_ADDFLAGS([my_ldadd], [LDFLAGS], [MV_RUN_IFELSE_FALSE], [ \
			-Wl,-O9 \
			$flto \
			-fwhole-program \
			-Wl,-z,noexecstack \
		], [$my_ldfatal], [], [:])
	MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_FALSE], [ \
			$flto \
		], [$my_cxxfatal], [], [:])])
# If -flto is used during compilation, we can also use -fwhole-program and
# -fno-fat-lto-objects during compilation and should specify -flto during
# linking.
AS_VAR_SET([fwhole_program], [])
AS_VAR_SET([warn_common], [-Wl,--warn-common])
AS_CASE([" $CXXFLAGS $my_cxxadd "],
	[*" -flto "*],
		[AS_VAR_SET([fwhole_program], ["
			-fwhole-program
			-fno-fat-lto-objects
			"])])
AS_IF([$strong_optimization],
	[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_FALSE], [ \
			-fweb \
			-fgraphite \
			-fgraphite-identity \
			-floop-interchange \
			-floop-strip-mine \
			-floop-block \
			-fno-enforce-eh-specs \
			$fwhole_program \
			-ftree-vectorize \
			-fvect-cost-model \
			-fno-rtti \
			-fno-threadsafe-statics \
		], [$my_cxxfatal], [$CPPFLAGS], [:])
	AS_IF([$strong_security],
		[],
		[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_LINK], [ \
			-DNDEBUG \
			-DNO_DEBUG \
			-DG_DISABLE_ASSERT \
		], [$my_cxxfatal], [$CPPFLAGS], [:])])])
dnl We do not use:
dnl Would not change anything:
dnl			-ffast-math \
dnl Too much memory/time for compilation:
dnl			-fipa-pta
dnl Default (e.g. with -O2):
dnl			-fipa-icf
dnl			-fipa-ra
AS_IF([$warnings],
	[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [AC_COMPILE_IFELSE], [ \
			-fno-common \
			-funsigned-char \
			-fdirectives-only \
			-ftracer \
			-fconcepts \
		], [$my_cxxfatal], [$CPPFLAGS], [:])
	MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [AC_COMPILE_IFELSE], [ \
			-Wpedantic \
			-pedantic \
			-Wall \
			-Weverything \
			-Wextra \
			-Wformat=2 \
			-Wunused-variable \
			-Wunused-parameter \
			-Wstring-plus-int \
			-WNSObject-attribute \
			-Winit-self \
			-Wmissing-include-dirs \
			-Wswitch-default \
			-Wunused \
			-Wunused-parameter \
			-Wstrict-aliasing=1 \
			-Wstrict-overflow=5 \
			-Wfloat-equal \
			-Wundef \
			-Wshadow \
			-Wpointer-arith \
			-Wcast-qual \
			-Wcast-align \
			-Wwrite-strings \
			-Wsign-compare \
			-Wmissing-field-initializers \
			-Wnormalized=nfkc \
			-Wpacked \
			-Wredundant-decls \
			-Winvalid-pch \
			-Wvolatile-register-var \
			-Wdisabled-optimization \
			-Wlogical-op \
			-Wvla \
			-Wabi \
			-Wctor-dtor-privacy \
			-Wstrict-null-sentinel \
			-Wstring-plus-int \
			-Woverloaded-virtual \
			-Wsign-promo \
			-Wnoexcept \
			-Wmissing-declarations \
			-Wmissing-format-attribute \
			-Wunused-but-set-variable \
			-Wunused-but-set-parameter \
			-Wdelete-non-virtual-dtor \
			-Wzero-as-null-pointer-constant \
			-Wint-to-pointer-cast \
			-Wcomment \
			-WNSObject-attribute \
			-fdiagnostic-color=always \
			-Wswitch-bool \
			-Wlogical-not-parantheses \
			-Wsizeof-array-argument \
			-Wbool-compare \
			-Wodr-type-mismatch \
			-Wshift-negative-value \
			-Wshift-overflow \
			-Wshift-overflow=2 \
			-Wnull-dereference \
			-Wduplicated-cond \
			-Wsign-conversion \
			-Wold-style-cast \
			-Wmissing-noreturn \
		], [$my_cxxfatal], [], [:])
	MV_ADDFLAGS([my_ldadd], [LDFLAGS], [MV_RUN_IFELSE_FALSE], [ \
			-Wl,-z,defs \
			-Wl,--no-undefined \
			-Wl,--no-allow-shlib-undefined \
			-Wl,--no-undefined-version \
			$warn_common \
		], [$my_cxxfatal], [], [:])])
dnl We do not use:
dnl			-Wsuggest-attribute=pure \
dnl			-Wsuggest-attribute=const \
dnl			-Wsuggest-attribute=noreturn \
dnl Not needed anymore with current clang:
dnl			-Wno-unknown-warning-option \
dnl Output is too confusing:
dnl			-fmem-report \
dnl Too many useless warnings or only for C:
dnl			-Wtraditional \
dnl			-Wc++-compat \
dnl			-Wstrict-prototypes \
dnl			-Wold-style-definition \
dnl			-Wmissing-prototypes \
dnl			-Wnested-externs \
dnl			-Wpointer-sign \
dnl Included in -Wall (in gcc-6; not available earlier, anyway)
dnl			-Wtautological-compare \
dnl			-Wmisleading-indentation \
dnl
dnl We do not use (since it gives too many useless warnings or is only for C):
dnl			-Wconversion \
dnl			-Wswitch-enum \
dnl			-Wunsafe-loop-optimizations \
dnl			-Wpadded \
dnl			-Wunreachable-code \
dnl			-Waggregate-return \
dnl			-Winline \
dnl			-Weffc++ \
AS_IF([$debugging],
	# We try to add -fsanitize=... to LDFLAGS before adding to CXXFLAGS
	# since on some systems it requires also linking with a library.
	[AS_VAR_SET([my_sanitize], ["
			-faddress-sanitizer
			-fsanitize=address
			-fsanitize=leak
			-fsanitize=undefined
			-fsanitize=vla-bound
			-fsanitize=shift
			-fsanitize=signed-integer-overflow
			-fsanitize=integer-divide-by-zero
			-fsanitize=unreachable
			-fsanitize=null
			-fsanitize=return
			-fsanitize=float-divide-by-zero
			-fsanitize=float-cast-overflow
			-fsanitize=bounds
			-fsanitize=bounds-strict
			-fsanitize=alignment
			-fsanitize=object-size
			-fsanitize=vptr
			-fcheck-pointer-bounds
		"])
	# The following -fsanitize do not work properly
	# -fsanitize=threads
	MV_ADDFLAGS([my_ldadd], [LDFLAGS], [MV_RUN_IFELSE_LINK], [ \
			$my_sanitize \
		], [$my_ldfatal], [], [:])
	MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_LINK], [ \
			$my_sanitize \
		], [$my_cxxfatal], [$CPPFLAGS], [:])])

# We are done with testing, the real flag mangling takes place:
MV_PREPEND([CXXFLAGS], [$my_cxxadd])
AS_CASE([" $LDFLAGS $my_ldadd "],
	[*" -flto "*], [MV_APPEND([my_ldadd], [$CXXFLAGS])])
MV_PREPEND([LDFLAGS], [$my_ldadd])

AS_VAR_COPY([my_cxxflags], [CXXFLAGS])
AS_VAR_COPY([my_ldflags], [LDFLAGS])

# Provide external tools
AC_PROG_MKDIR_P()
dnl  AC_PROG_SED()

# Check if __builtin_expect works
AC_MSG_CHECKING([whether __builtin_expect can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[]], [[
int zero = 0;
int one = 1;
if(!((__builtin_expect(one, 0)) && (__builtin_expect(one, 1)) &&
	(!__builtin_expect(zero, 0)) && (!__builtin_expect(zero, 1))))
	return 1;
		]])],
	[MV_MSG_RESULT([yes])
	AS_VAR_SET([bi_expect_works], [:])],
	[MV_MSG_RESULT([no])
	AS_VAR_SET([bi_expect_works], [false])])
AS_IF([$bi_expect_works],
	[AC_DEFINE([HAVE___BUILTIN_EXPECT],
		[1],
		[Define if __builtin_expect can be used])])

# Done!
AC_OUTPUT()

AS_ECHO(["

$PACKAGE-$VERSION configured successfully.

"])
MV_IF_NONEMPTY([$CXX],
	[AS_ECHO(["CXX: $CXX
"])])
AS_ECHO(["CXXFLAGS: $oricxxflags"])
MV_IF_NONEMPTY([$my_cxxadd],
	[AS_ECHO(["PREPEND_CXXFLAGS: $my_cxxadd
"])])
AS_ECHO(["LDFLAGS: $orildflags"])
MV_IF_NONEMPTY([$my_ldadd],
	[AS_ECHO(["PREPEND_LDFLAGS: $my_ldadd"])])
AS_ECHO([])
