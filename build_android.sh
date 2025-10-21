#!/bin/bash

# 配置
NDK_HOME=${1:-~/Android/android-ndk-r27c/}
TOOLCHAIN="$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64"
TARGET="aarch64-linux-android"
API_LEVEL="21"

# 检查 NDK 是否存在
if [ ! -d "$NDK_HOME" ]; then
    echo "Error: NDK not found at $NDK_HOME"
    echo "Please download Android NDK and set the correct path"
    exit 1
fi

# 编译
echo -e "\nBuilding memleak for Android..."
"${TOOLCHAIN}/bin/${TARGET}${API_LEVEL}-clang" \
    -target ${TARGET}${API_LEVEL} \
    -fpie -pie \
    -Wl,--hash-style=both \
    -Wl,--no-undefined \
    -O0 \
    -o memleak_android \
    memleak.c

if [ $? -eq 0 ]; then
    echo "Build successful: memleak_android"
    echo "File info:"
    file memleak_android
else
    echo "Build failed: memleak_android"
    exit 1
fi
