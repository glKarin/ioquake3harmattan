#!/bin/sh


# This script is for "EXEC" field of desktop file.

USER_SH="/home/user/baseq3/Quake_III_Arena"
DEFAULT_SH="/usr/lib/ioquake3-touch/Quake_III_Arena.default"

if [ -x ${USER_SH} ]; then
	${USER_SH}
else
	${DEFAULT_SH}
fi
