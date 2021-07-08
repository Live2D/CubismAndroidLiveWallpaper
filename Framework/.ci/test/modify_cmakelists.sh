#!/usr/bin/env sh

set -ue

# リポジトリ単体で Core のヘッダを参照できないので .ci に用意したヘッダを参照する
sed -i -e 's|../Core/include|.ci/test|g' CMakeLists.txt

# 環境依存の Redering をビルドから除外
sed -i -e '/add_subdirectory(${FRAMEWORK_SOURCE})/d' src/Rendering/CMakeLists.txt
