# 開発用 Jenkins CI

## 概要

Framework は単体でビルドの確認が行えるため、Jenkins による CI の対象としている

ただし、**Framework リポジトリ単体では Core の参照が行えない**ため、Sample 層の `Core/include/Live2DCubismCore.h` を `Framework/.ci/test` にコピーしてテスト時にそちらを参照させている

GCC / Clang / VC++ のそれぞれ（なるべく）最新のコンパイラでビルドが通るか確認を行う
