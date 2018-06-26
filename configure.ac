# This file is part of the chessproblem project and distributed under the
# terms of the GNU General Public License v2.
#
# Copyright (c)
#  Martin V\"ath <martin@mvath.de>

dnl keep version in same line as AC_INIT for possible usage in scripts
AC_INIT([chessproblem], [2.8],
  [https://github.com/vaeth/chessproblem/issues/],
  [chessproblem],
  [https://github.com/vaeth/chessproblem/])
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
AM_INIT_AUTOMAKE()
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
	[AS_VAR_SET([new_dialect], [:])
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
AS_CASE([" $CXXFLAGS $my_cxxadd"],
	[*" -std="*], [],
	[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_LINK],
		[$try_dialect], [$my_cxxfatal], [], [break])])
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
			-DNDEBUG \
			-DHAVE_EXCEPTIONS=0 \
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
			-DNO_DEBUG \
			-DG_DISABLE_ASSERT \
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
		], [$my_cxxfatal], [$CPPFLAGS], [:])])
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
			-Wsuggest-attribute=pure \
			-Wsuggest-attribute=const \
			-Wsuggest-attribute=noreturn \
			-Wunreachable-code \
			-Waggregate-return \
		], [$my_cxxfatal], [], [:])
	MV_ADDFLAGS([my_ldadd], [LDFLAGS], [MV_RUN_IFELSE_FALSE], [ \
			-Wl,-z,defs \
			-Wl,--no-undefined \
			-Wl,--no-allow-shlib-undefined \
			-Wl,--no-undefined-version \
			$warn_common \
		], [$my_cxxfatal], [], [:])])
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
			-fsanitize=thread
			-fcheck-pointer-bounds
		"])
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

# The attribute tests are special: They should even break on warnings:
AS_VAR_COPY([CXXFLAGS], [oricxxflags])
AS_VAR_COPY([LDFLAGS], [orildflags])
MV_APPEND([CXXFLAGS], [$my_cxxdialect])
MV_APPEND([CXXFLAGS], [$my_cxxfatal])
MV_APPEND([LDFLAGS], [$my_ldfatal])

# We even switch on explicitly -Wattribute although this should be default:
MV_ADDFLAGS([CXXFLAGS], [CXXFLAGS], [AC_COMPILE_IFELSE], [ \
		-Wattributes \
	])

# Check if [[maybe_deprecated]] works
AC_MSG_CHECKING([[whether [[deprecated]] can be used]])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
[[deprecated("a reason")]] void a();
void a() {
}
		]], [[
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([c11attr_deprecated], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([c11attr_deprecated], [false])])
AS_IF([$c11attr_deprecated],
	[AC_DEFINE([HAVE_C11ATTRIBUTE_DEPRECATED],
		[1],
		[[Define if [[deprecated]] can be used]])])

# Check if __attribute__ ((deprecated)) works
AC_MSG_CHECKING([whether __attribute__ ((deprecated)) can be used])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
__attribute__ ((deprecated("a reason"))) void a();
void a() {
}
		]], [[
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_deprecated], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_deprecated], [false])])
AS_IF([$attr_deprecated],
	[AC_DEFINE([HAVE_ATTRIBUTE_DEPRECATED],
		[1],
		[[Define if __attribute__ ((deprecated("a"))) can be used]])])

# Check if [[noreturn]] works
AC_MSG_CHECKING([[whether [[noreturn]] can be used]])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <cstdlib>
[[noreturn]] static void dummy();
static void
dummy()
{ std::exit(0); }
		]], [[
dummy();
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([c11attr_noreturn], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([c11attr_noreturn], [false])])
AS_IF([$c11attr_noreturn],
	[AC_DEFINE([HAVE_C11ATTRIBUTE_NORETURN],
		[1],
		[[Define if [[noreturn]] can be used]])])

# Check if __attribute__ ((noreturn)) works
AC_MSG_CHECKING([whether __attribute__ ((noreturn)) can be used])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <cstdlib>
__attribute__ ((noreturn)) static void dummy();
static void
dummy()
{ std::exit(0); }
		]], [[
dummy();
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_noreturn], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_noreturn], [false])])
AS_IF([$attr_noreturn],
	[AC_DEFINE([HAVE_ATTRIBUTE_NORETURN],
		[1],
		[Define if __attribute__ ((noreturn)) can be used])])

# Check if [[nodiscard]] works
AC_MSG_CHECKING([[whether [[nodiscard]] can be used]])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
[[nodiscard]] static int dummy();
static int dummy() { return 0; }
		]], [[
return dummy();
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([c11attr_nodiscard], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([c11attr_nodiscard], [false])])
AS_IF([$c11attr_nodiscard],
	[AC_DEFINE([HAVE_ATTRIBUTE_NODISCARD],
		[1],
		[[Define if [[nodiscard]] can be used]])])

# Check if __attribute__ ((warn_unused_result)) works
AC_MSG_CHECKING([whether __attribute__ ((warn_unused_result)) can be used])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
__attribute__ ((warn_unused_result)) static int dummy();
static int dummy() { return 0; }
		]], [[
return dummy();
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_warn_unused_result], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_warn_unused_result], [false])])
AS_IF([$attr_warn_unused_result],
	[AC_DEFINE([HAVE_ATTRIBUTE_WARN_UNUSED_RESULT],
		[1],
		[Define if __attribute__ ((warn_unused_result)) can be used])])

# Check if [[fallthrough]] works
AC_MSG_CHECKING([[whether [[fallthrough]] can be used]])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
		]], [[
int i = 0;
switch(i) {
	case 0:
		--i;
		[[fallthrough]];
	default:
		return i + 1;
}
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_fallthrough], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_fallthrough], [false])])
AS_IF([$attr_fallthrough],
	[AC_DEFINE([HAVE_ATTRIBUTE_FALLTHROUGH],
		[1],
		[Define if [[fallthrough]] can be used])])

# Check if __attribute__ ((const)) works
AC_MSG_CHECKING([whether __attribute__ ((const)) can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
class b {
public:
	static int c(int i) __attribute__ ((const)) { return i; }
};
int a(int i) __attribute__ ((const));
int a(int i) { b x; return x.c(i); }
		]], [[
return !((a(4) == 4) && (a(5) == 5));
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_const], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_const], [false])])
AS_IF([$attr_const],
	[AC_DEFINE([HAVE_ATTRIBUTE_CONST],
		[1],
		[Define if __attribute__ ((const)) can be used])])

# Check if __attribute__ ((pure)) works
AC_MSG_CHECKING([whether __attribute__ ((pure)) can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
class b {
public:
	static int c(int *i) __attribute__ ((pure)) { return *i; }
};
__attribute__ ((pure)) int a(int i);
int a(int i) { b x; return x.c(&i); }
		]], [[
return !((a(4) == 4) && (a(5) == 5));
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_pure], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_pure], [false])])
AS_IF([$attr_pure],
	[AC_DEFINE([HAVE_ATTRIBUTE_PURE],
		[1],
		[Define if __attribute__ ((pure)) can be used])])

# Check if __attribute__ ((nonnull)) works
AC_MSG_CHECKING([whether __attribute__ ((nonnull)) can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
class b {
public:
	__attribute__ ((nonnull)) int c(int *i) { return *i; }
};
int a(int *i) __attribute__ ((nonnull));
int a(int *i) { b x; return x.c(i); }
		]], [[
int i = 4, j = 5;
return !((a(&i) == 4) && (a(&j) == 5));
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_nonnull_], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_nonnull_], [false])])
AS_IF([$attr_nonnull_],
	[AC_DEFINE([HAVE_ATTRIBUTE_NONNULL_],
		[1],
		[Define if __attribute__ ((nonnull)) can be used])])

# Check if __attribute__ ((nonnull (x, y))) works
AC_MSG_CHECKING([whether __attribute__ ((nonnull (x, y))) can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
class b {
public:
	__attribute__ ((nonnull (1, 3))) static int c(int *i, int *j, int *k) {
		return (j ? ((*i) - (*k)) : ((*i) + (*k)));
	}
	__attribute__ ((nonnull (2, 4))) int d(int *i, int *j, int *k) {
		return c(i,j,k);
	}
};
__attribute__ ((nonnull (1, 3))) int a(int *i, int *j, int *k);
int a(int *i, int *j, int *k) {
	b x; return x.d(i, j, k) + x.d(i, 0, k);
}
		]], [[
int i = 1;
b x;
return !((x.d(&i, 0, &i) == 2) && (x.d(&i, 0, &i) == 2) && (a(&i, 0, &i) == 4));
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_nonnull], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_nonnull], [false])])
AS_IF([$attr_nonnull],
	[AC_DEFINE([HAVE_ATTRIBUTE_NONNULL],
		[1],
		[Define if __attribute__ ((nonnull (list))) can be used])])

# reset the CXXFLAGS, LDFLAGS for the normal tests
AS_VAR_COPY([CXXFLAGS], [my_cxxflags])
AS_VAR_COPY([LDFLAGS], [my_ldflags])

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

# What about multithreading?
AC_MSG_CHECKING([whether multithreading should be used])
AS_VAR_SET([support_multithreading], [:])
AS_VAR_SET([requested_multithreading], [false])
AC_ARG_WITH([multithreading],
	[AS_HELP_STRING([--with-multithreading],
		[Compile in support for cache method multithreading])],
	[AS_CASE(["$withval"],
		[no], [AS_VAR_SET([support_multithreading], [false])],
		[yes], [AS_VAR_SET([requested_multithreading], [:])])])
AS_IF([$support_multithreading],
  [AS_VAR_SET([MULTITHREAD_LIBS], ["-lpthread"]);
  AS_VAR_COPY([saveldflags], [LDFLAGS])
  AS_VAR_APPEND([LDFLAGS], [" $MULTITHREAD_LIBS"])
  MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>
std::atomic_int ret(1);
std::mutex m;
class Dummy {
public:
void f(int i);
int c(int i) {
	std::vector<std::thread> threads;
	threads.emplace_back(&Dummy::f, this, i);
	threads.back().join();
	return ret.load(std::memory_order_consume);
}
};
void Dummy::f(int i) {
	m.lock();
	ret.store(i, std::memory_order_release);
	m.unlock();
}
]], [[
	Dummy dummy;
	return dummy.c(0);
		]])],
	[],
	[AS_VAR_SET([support_multithreading], [false])])
AS_VAR_COPY([LDFLAGS], [saveldflags])])
AS_IF([$support_multithreading],
	[MV_MSG_RESULT([yes])],
	[MV_MSG_RESULT([no])
	AS_IF([$requested_multithreading],
		[AC_MSG_WARN(multithreading requested but not available)])
	AC_DEFINE([NO_CHESSPROBLEM_THREADS],
		[1],
		[Define if multithreading is not wanted/possible])
	AS_VAR_SET([MULTITHREAD_LIBS], [])])
AC_SUBST([MULTITHREAD_LIBS])

AC_MSG_CHECKING([maximal parallel threads default])
AC_ARG_WITH([max-parallel-default],
	[AS_HELP_STRING([--with-max-parallel-default=STR],
		[default maximal parallel threads])],
	[AS_VAR_COPY([MAX_PARALLEL_DEFAULT], [withval])],
	[AS_VAR_SET([MAX_PARALLEL_DEFAULT], [])])
MV_MSG_RESULT_VAR([MAX_PARALLEL_DEFAULT])
MV_IF_NONEMPTY([$MAX_PARALLEL_DEFAULT],
	[AC_DEFINE_UNQUOTED([MAX_PARALLEL_DEFAULT],
		[$MAX_PARALLEL_DEFAULT],
		[default maximal parallel threads if defined])])

AC_MSG_CHECKING([minimal half move depth for a new thread])
AC_ARG_WITH([min-half-move-depth-default],
	[AS_HELP_STRING([--with-min-half-move-depth-default=STR],
		[default minimal half move depth for a new thread])],
	[AS_VAR_COPY([MIN_HALF_MOVE_DEPTH_DEFAULT], [withval])],
	[AS_VAR_SET([MIN_HALF_MOVE_DEPTH_DEFAULT], [])])
MV_MSG_RESULT_VAR([MIN_HALF_MOVE_DEPTH_DEFAULT])
MV_IF_NONEMPTY([$MIN_HALF_MOVE_DEPTH_DEFAULT],
	[AC_DEFINE_UNQUOTED([MIN_HALF_MOVE_DEPTH_DEFAULT],
		[$MIN_HALF_MOVE_DEPTH_DEFAULT],
		[default minimal half move depth for a new thread if defined])])

AC_MSG_CHECKING([propagate-signal])
AC_ARG_ENABLE([propagate-signal],
	[AS_HELP_STRING([--enable-propagate-signal],
		[use memory intensive (faster?/slower?) thread policy])],
	[MV_ENABLE([propagate_signal])],
	[AS_VAR_SET([propagate_signal], [false])])
MV_MSG_RESULT_BIN([$propagate_signal])
AS_IF([$propagate_signal],
	[AC_DEFINE([PROPAGATE_SIGNAL],
		[1],
		[Define if memory intensive (faster?/slower?) thread policy should be used])])

AC_MSG_CHECKING([unlimited])
AC_ARG_ENABLE([unlimited],
	[AS_HELP_STRING([--enable-unlimited],
		[enable if number of thread should not be limited by hardware_concurrency])],
	[MV_ENABLE([unlimited])],
	[AS_VAR_SET([unlimited], [false])])
MV_MSG_RESULT_BIN([$unlimited])
AS_IF([$unlimited],
	[AC_DEFINE([UNLIMITED],
		[1],
		[Define if number of threads should not be limited by hardware_concurrency])])

AS_VAR_SET([have_osformat], [false])
m4_ifdef([PKG_CHECK_MODULES],
	[PKG_CHECK_MODULES([OSFORMAT], [osformat],
		[AS_VAR_SET([have_osformat], [:])],
		[AC_WARN(OSFORMAT not found by pkg-config or pkgconf.
You might have to adjust PKG_CONFIG_PATH)])],
	[AC_WARN([PKG_CHECK_MODULES not available.
Attempting only a poor heuristics])])
AC_MSG_CHECKING([whether osformat can be used])
AS_IF([$have_osformat],
	[MV_MSG_RESULT([yes])],
	[AS_VAR_SET([OSFORMAT_LIBS], ["-losformat"])
	AS_VAR_SET([OSFORMAT_CFLAGS], ["-I/usr/include/osformat"])
	MV_APPEND([CXXFLAGS], [$OSFORMAT_CFLAGS])
	MV_APPEND([LDFLAGS], [$OSFORMAT_LIBS])
	MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
#include <osformat>
	]], [[
Format("%.*s") % 2 % 1;
		]])],
		[MV_MSG_RESULT([yes], [using guessed paths])],
		[AC_ERROR([
The osformat library is not installed or cannot be used.
Perhaps pkgconf/pkgconfig is not available or PKG_CONFIG_PATH is wrong.
Or perhaps https://github.com/vaeth/osformat is not installed.
This is required to build the executable demo program])])])
AC_SUBST([OSFORMAT_LIBS])
AC_SUBST([OSFORMAT_CFLAGS])

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
