#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
printf '\nBuilt successfully. Run: sh play.sh\n'
