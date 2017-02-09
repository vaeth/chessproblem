#!/usr/bin/env sh
set -u

Echo() {
	printf '%s\n' "$*" >&2
}

Die() {
	Echo "$*"
	exit 1
}

Run() {
	Echo ">>> $*"
	"$@" || Die 'failure'
}

Run mkdir -p -m 755 config
Run aclocal -I m4 -I martinm4
Run autoconf
Run autoheader
Run automake -a --copy ${1+"$@"}
