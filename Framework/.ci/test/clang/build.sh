#!/usr/bin/env sh

set -ue

cmake -S . -B build -G "Unix Makefiles" -D CMAKE_BUILD_TYPE="Release"
cd build && make
