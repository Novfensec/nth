# ISO Generation Scripts

This directory contains shell scripts that automate the process of creating custom, bootable UEFI ISOs for major operating systems using the `nth` bootloader.

## Overview

A standard UEFI bootable CD-ROM (ISO) requires an **El Torito boot catalog** pointing to an EFI System Partition image (usually a FAT FAT12/16/32 image file). 

These scripts completely automate the process:
1. They create a virtual FAT image (`efi.img`).
2. They compile and inject the `nth` bootloader (`BOOTX64.EFI`) into the FAT image.
3. They copy the target OS's native bootloader (or Linux kernel) and configure `nth.cfg` to chainload it.
4. They pad the image size appropriately (especially critical for Windows to ensure FAT32 compliance).
5. They use `xorriso` to bundle the FAT image and the original OS installation files into a new, hybrid-bootable ISO.
6. They automatically launch the generated ISO in QEMU for immediate testing.

## Prerequisites

To run these scripts, you need a Linux environment (or WSL on Windows) with the following packages installed:
```bash
sudo apt install build-essential cmake mtools xorriso dosfstools qemu-system-x86 gnu-efi
```

## 1. Creating an Alpine Linux ISO (`mk_alpine_iso.sh`)

This script creates a lightweight, custom Alpine Linux ISO.

### How to use:
1. Download an [Alpine Linux Standard ISO](https://alpinelinux.org/downloads/) and extract its contents to a folder.
2. Run the script:
   ```bash
   ./mk_alpine_iso.sh /path/to/extracted/alpine/iso
   ```
*(If you run the script without arguments, it will prompt you for the directory).*

### What happens under the hood:
The script copies the Alpine kernel (`vmlinuz-lts`) and the Initramfs (`initramfs-lts`) directly into the FAT image. It then writes an `nth.cfg` entry that boots the Linux kernel as an EFI stub, passing the necessary `LoadOptions` (command line arguments) to load the SquashFS root filesystem from the CD-ROM.

### Testing (QEMU / VirtualBox / Actual Hardware)
The script will automatically attempt to launch the generated `nth_alpine.iso` in QEMU. For a real test, you can also attach the ISO to a VirtualBox VM with EFI enabled, or flash it to a USB drive to boot on actual physical hardware.

## 2. Creating a Windows ISO (`mk_windows_iso.sh`)

This script replaces the default Microsoft Windows Boot Manager with `nth`, allowing you to show a custom boot menu before Windows Setup starts.

### How to use:
1. Extract a Windows 10 or Windows 11 installation ISO to a folder.
2. Run the script:
   ```bash
   ./mk_windows_iso.sh /path/to/extracted/windows/iso
   ```
*(If you run the script without arguments, it will prompt you for the directory).*

### What happens under the hood:
Unlike Alpine, we cannot fit the massive Windows installation files (`boot.wim`) onto the FAT image. 
Instead, the script:
1. Renames the original Windows Boot Manager (`efi/boot/bootx64.efi`) to `win_boot.efi`.
2. Installs `nth` as the primary bootloader (`BOOTX64.EFI`).
3. **Crucially:** Copies the `\efi\microsoft\boot` folder (containing the BCD store and fonts) into the FAT image. Without this, the Windows Boot Manager will instantly crash upon being chainloaded.
4. Generates an `nth.cfg` that chainloads the PE/COFF `win_boot.efi` application.

Once `nth` hands control over to `win_boot.efi`, the Windows Boot Manager reads the BCD and uses its own CD-ROM drivers to locate the rest of the Windows installation media (`sources/boot.wim`) on the ISO.

### Testing (QEMU / VirtualBox / Actual Hardware)
The script automatically attempts to run QEMU with software emulation if hardware virtualization (KVM) is unavailable. Because Windows is extremely heavy, software emulation will be very slow. For a real test, it is highly recommended to attach the generated `nth_windows.iso` directly to a VirtualBox VM with EFI enabled, or flash it to a USB drive to boot on actual hardware!
