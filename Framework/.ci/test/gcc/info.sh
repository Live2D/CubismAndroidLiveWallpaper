#!/usr/bin/env sh

set -ue

echo '[Script] OS information.'
cat /etc/os-release
echo ''

echo '[Script] GNU Make information.'
make --version
echo ''

echo '[Script] GCC information.'
gcc --version
echo ''

echo '[Script] CMake information.'
cmake --version
echo ''
