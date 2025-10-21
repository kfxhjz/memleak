#!/bin/bash

echo "Building memleak for x64_64 Linux..."
gcc -static -Wall -Wextra -Werror -O0 -o memleak_linux memleak.c
if [ $? -eq 0 ]; then
    echo "Build successful: memleak_linux"
    echo "File info:"
    file memleak_linux
else
    echo "Build failed: memleak_linux"
    exit 1
fi

# check aarch64-linux-gnu-gcc exists
if command -v aarch64-linux-gnu-gcc &>/dev/null; then
    echo -e "\nBuilding memleak for aarch64 Linux..."
    aarch64-linux-gnu-gcc -static -Wall -Wextra -Werror -O0 -o memleak_linux_aarch64 memleak.c
    if [ $? -eq 0 ]; then
        echo "Build successful: memleak_linux_aarch64"
        echo "File info:"
        file memleak_linux_aarch64
    else
        echo "Build failed: memleak_linux_aarch64"
        exit 1
    fi
fi
