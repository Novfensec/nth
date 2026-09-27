#!/bin/bash
set -e

cd "$(dirname "$0")"

bash build.sh

echo "Building bootloader..."
cd ..
mkdir -p build
cd build
cmake ..
make
cd ../example_kernel

echo "Preparing ISO staging environment..."
mkdir -p ../esp/EFI/BOOT
cp ../build/BOOTX64.EFI ../esp/EFI/BOOT/BOOTX64.EFI

rm -rf iso_root
mkdir -p iso_root
cp -r ../esp/* iso_root/

dd if=/dev/zero of=efi.img bs=1M count=64 status=none
mkfs.vfat -F 32 efi.img > /dev/null

mcopy -s -i efi.img iso_root/* ::/

mv efi.img iso_root/efi.img

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

OVMF_CODE=""
for p in /usr/share/OVMF/OVMF_CODE_4M.fd /usr/share/OVMF/OVMF_CODE.fd /usr/share/edk2-ovmf/x64/OVMF_CODE.fd; do
    if [ -f "$p" ]; then
        OVMF_CODE="$p"
        break
    fi
done

if [ -z "$OVMF_CODE" ]; then
    echo "Error: OVMF_CODE not found. Please install ovmf."
    exit 1
fi

qemu-system-x86_64 \
    -drive if=pflash,format=raw,readonly=on,file="$OVMF_CODE" \
    -drive if=pflash,format=raw,file=OVMF_VARS_4M.fd \
    -cdrom nth_os.iso \
    -m 256M \
    -net none
