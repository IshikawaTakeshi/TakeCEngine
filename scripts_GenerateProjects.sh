#!/usr/bin/env bash
set -euo pipefail

mkdir -p build

# GNU Make (gmake2) 生成。Xcode などにしたい場合は premake5 xcode4 などへ変更
echo "Generating gmake2 project files..."
premake5 gmake2

echo
echo "Generated makefiles in: build/"