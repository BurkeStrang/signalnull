#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")"
if [ ! -x build/signal-null ]; then sh build.sh; fi
exec ./build/signal-null "$@"
