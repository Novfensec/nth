#!/bin/bash
set -e

mkdir -p build
mkdir -p esp/EFI/BOOT

cd build
cmake ..
make
cd ..

cp build/BOOTX64.EFI esp/EFI/BOOT/
qemu-system-x86_64 \
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
    -drive if=pflash,format=raw,file=OVMF_VARS_4M.fd \
    -drive file=fat:rw:esp,format=raw
