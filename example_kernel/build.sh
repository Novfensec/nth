#!/bin/bash
set -e

echo "Building kernel..."
x86_64-elf-gcc -ffreestanding -mcmodel=kernel -mno-red-zone -Wall -Wextra -O2 -I../src -c kernel.c -o kernel.o
x86_64-elf-ld -T linker.ld kernel.o -o kernel.elf

echo "Copying kernel.elf to esp/"
mkdir -p ../esp
cp kernel.elf ../esp/kernel.elf

echo "Kernel build complete!"
