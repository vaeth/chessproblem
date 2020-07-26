#!/usr/bin/env sh
# Copyright (c) Martin V\"ath <martin at mvath.de>
# SPDX-License-Identifier: GPL-2.0-only
set -u

project='chessproblem'
mkmake='contrib/make.sh'
make_opts='-oGeq'

LC_ALL=C
XZ_OPT=--extreme
export LC_ALL XZ_OPT
umask 022

Echo() {
	printf '%s\n' "$*"
}

Usage() {
	Echo "Usage: ${0##*/} [options] [xz|bzip2|gzip|default]
options: -fakeroot, -fakeroot-ng, -no-fakeroot (default is -$fakeroot)"
	exit ${1:-1}
}

Die() {
	Echo "${0##*/}: $1"
	exit ${2:-1}
}

Run() {
	Echo "$*"
	"$@" || Die "$* failed" $?
}

dist='dist'
fakeroot=
[ -z "${fakeroot:++}" ] && command -v fakeroot >/dev/null 2>&1 && fakeroot='fakeroot'
[ -z "${fakeroot:++}" ] && command -v fakeroot-ng >/dev/null 2>&1 && fakeroot='fakeroot-ng'

while [ $# -gt 0 ]
do	opt=${1#-}
	opt=${opt#-}
	case ${opt#dist-} in
	[xX]*)
		dist='dist-xz';;
	[bB]*)
		dist='dist-bzip2';;
	[gG]*)
		dist='dist-gzip';;
	[dD]*)
		dist='dist';;
	f*ng)
		fakeroot='fakeroot-ng';;
	f*)
		fakeroot='fakeroot';;
	F*|n*f*)
		fakeroot=;;
	[hH?]*)
		Usage 0;;
	*)
		Usage;;
	esac
	shift
done
[ -z "${dist:++}" ] && Usage

if [ -n "${mkclean:++}" ]
then test -e Makefile || test -e configure || test -e Makefile.in \
	&& test -e "$mkclean" && Run "$mkclean"
fi

unset LDFLAGS CFLAGS CXXFLAGS CPPFLAGS
if [ -n "${fakeroot:++}" ] && command -v "$fakeroot" >/dev/null 2>&1
then	Run $mkmake ${make_opts}n
	docmd="$fakeroot $mkmake ${make_opts}r"
else	docmd="$mkmake $make_opts"
fi
for i in $dist
do	Run $docmd "$i"
done

found=false
for j in tar.xz tar.bz2 tar.gz zip
do	for i in "$PWD/$project"-*".$j"
	do	test -f "$i" || continue
		$found || Echo "Available tarballs:"
		found=:
		Echo "	$i"
	done
done
$found || Die "For some reason no tarball was created"
