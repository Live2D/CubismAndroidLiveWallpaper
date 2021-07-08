#!/usr/bin/env sh

set -ue

echo '[Script] OS information.'
sw_vers
echo ''

echo '[Script] GNU Make information.'
make --version
echo ''

echo '[Script] Apple Clang information.'
clang --version
echo ''

echo '[Script] CMake information.'
cmake --version
echo ''

echo '[Script] Xcode information.'
xcodebuild -version
