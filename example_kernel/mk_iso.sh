#!/bin/bash
set -e

cd "$(dirname "$0")"

echo "Building kernel..."
bash build.sh

echo "Building bootloader..."
cd ..
mkdir -p build
cd build
cmake ..
make
cd ../example_kernel

echo "Preparing ISO staging environment..."
rm -rf iso_root
mkdir -p iso_root

dd if=/dev/zero of=iso_root/efi.img bs=1M count=32 status=none
mkfs.vfat -F 32 iso_root/efi.img > /dev/null

mmd -i iso_root/efi.img ::/EFI
mmd -i iso_root/efi.img ::/EFI/BOOT

mcopy -o -s -i iso_root/efi.img ../esp/* ::/
mcopy -o -i iso_root/efi.img ../build/BOOTX64.EFI ::/EFI/BOOT/BOOTX64.EFI

echo "Generating standard UEFI ISO..."
xorriso -as mkisofs \
    -R -f \
    -e efi.img \
    -no-emul-boot \
    -isohybrid-gpt-basdat \
    -o nth_os.iso \
    iso_root > /dev/null 2>&1

echo "Booting ISO in QEMU..."
cp /usr/share/OVMF/OVMF_VARS_4M.fd . 2>/dev/null || true

qemu-system-x86_64 \
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
    -drive if=pflash,format=raw,file=OVMF_VARS_4M.fd \
    -cdrom nth_os.iso \
    -m 32M
