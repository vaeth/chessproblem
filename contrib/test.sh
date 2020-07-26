#!/usr/bin/env sh
# Copyright (c) Martin V\"ath <martin at mvath.de>
# SPDX-License-Identifier: GPL-2.0-only
set -u
LC_ALL=C
export LC_ALL
RunTests() {
  echo "Running $*" >&2
  perl contrib/test.pl -q chessproblem/chessproblem ${1+"$@"}
}
if [ $# -ne 0 ]
then	RunTests ${1+"$@"}
	exit
fi
max=`nproc` && [ -n "$max" ] && [ "$max" -gt 0 ] || max = 4
i=0
while [ "$i" -lt "$max" ]
do	i=$(( $i + 1 ))
	RunTests "-j$i" "-J1"
	RunTests "-j$i"
done
