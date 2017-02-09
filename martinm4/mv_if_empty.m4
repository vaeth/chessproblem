dnl This file is part of the chessproblem project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin V\"ath <martin@mvath.de>
dnl
dnl MV_IF_EMPTY([ARG], [if-true], [if-false])
dnl Expands to if-true or if-false, depending on whether ARG is empty.
dnl Empty means either m4-empty or M4SH-empty (i.e. after variable expansion).
dnl Do not quote ARG!
AC_DEFUN([MV_IF_EMPTY],
	[m4_ifval([$1],
		[dnl If content does not contain variables, we need no shell:
		AS_LITERAL_IF([$1],
			[$3],
			[dnl We need shell only for nonempty content with vars:
			AS_IF([test x"$1" = x""],
				[$2],
				[$3])])],
		[$2])])
dnl
dnl MV_IF_NONEMPTY([ARG], [if-true], [if-false])
dnl Expands to if-true or if-false, depending on whether ARG is M4SH-nonempty
dnl (i.e. after variable expansion).
dnl Do not quote ARG!
AC_DEFUN([MV_IF_NONEMPTY],
	[MV_IF_EMPTY([$1], [$3], [$2])])
