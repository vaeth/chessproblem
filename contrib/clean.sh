#!/usr/bin/env sh
set -u

LC_ALL=C
export LC_ALL
umask 022

Echo() {
	printf '%s\n' "$*"
}

Die() {
	Echo "$1"
	exit ${2:-1}
}

if ! test -e Makefile
then	test -e Makefile.in && test -e configure \
		|| ./autogen.sh || Die "./autogen.sh failed"
	./configure || Die "./configure failed"
fi
if test -e Makefile
then	make maintainer-clean || Die "make maintainer-clean failed"
fi
for i in tar.xz tar.bz2 tar.gz zip
do	rm -vf -- chessproblem-*."$i"
done
find . '(' -type f '(' \
		-name 'Makefile.in' -o \
		-name 'svn-commit*.tmp' \
		')' -exec rm -vf -- '{}' '+' ')' -o \
	'(' -type d '(' \
		-name 'tmpcvs[0-9][0-9]*' -o \
		-name 'tmpwrk[0-9][0-9]*' \
		')' -exec rm -rf -- '{}' '+' ')'

