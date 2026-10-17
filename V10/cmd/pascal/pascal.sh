#!/bin/sh

P_CMD=pix

case "$1" in
	-c)	P_CMD=pi; shift;;
	-x)	P_CMD=px; shift;;
esac

exec /usr/lib/pascal/$P_CMD "$@"
