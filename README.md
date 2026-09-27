# nth

[![Support Development](https://img.shields.io/github/sponsors/Novfensec?style=for-the-badge&label=Support%20Development&logo=github&color=000000)](https://github.com/sponsors/Novfensec)
[![Support via PayPal](https://img.shields.io/badge/Support-PayPal-00457C?style=for-the-badge&logo=paypal&logoColor=white)](https://www.paypal.me/KARTAVYASHUKLA)
[![Support via Wise](https://img.shields.io/badge/Support-Wise-9FE870?style=for-the-badge&logo=wise&labelColor=163300)](https://wise.com/pay/business/kartavyashukla)

`nth` is an open-source UEFI bootloader and the reference implementation for the `nth` boot protocol. It provides a built-in graphical boot manager and supports booting both **64-bit ELF kernels** and **standard Linux EFI stub kernels**.

### Screenshots
![nth Boot Menu](assets/homepage.png?raw=true "nth Boot Menu")

## Table of Contents
- [Setup](#setup)
  - [Prerequisites (Linux / WSL)](#prerequisites-linux--wsl)
- [Build Instructions](#build-instructions)
- [Booting Manually](#booting-manually)
- [Custom Kernels & OS Integration](#custom-kernels--os-integration)
  - [Boot Manager Configuration (`nth.cfg`)](#boot-manager-configuration-nthcfg)
  - [Linux Boot Support](#linux-boot-support)
  - [Linking Your Kernel (`linker.ld`)](#linking-your-kernel-linkerld)
  - [The Nth Protocol](#the-nth-protocol)
  - [Parsing the Memory Map](#parsing-the-memory-map)
  - [Memory Mapping & Higher Half Support](#memory-mapping--higher-half-support)


### Supported architectures
* x86-64 (UEFI)

### Supported boot protocols
* [nth protocol](#the-nth-protocol)

### Supported filesystems
* FAT32

### Minimum system requirements
* An x86-64 system, virtual machine with UEFI firmware (e.g., QEMU with OVMF, Virtualbox)

## Setup

### Prerequisites (Linux / WSL)

The project requires an `x86_64-elf` cross-compiler and the `gnu-efi` library. The build system relies on standard Ubuntu/Debian paths for `gnu-efi`.

1. **Install dependencies:**

   **Ubuntu / Debian**
   ```bash
   sudo apt update
   sudo apt install -y build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo wget cmake mtools xorriso dosfstools qemu-system-x86 gnu-efi
   ```
   *Note: `dosfstools` provides `mkfs.vfat`, `mtools` provides `mcopy`/`mmd`, and `gnu-efi` provides the UEFI headers and libraries.*

2. **Build the Cross-Compiler:**

   Run the provided script to build `x86_64-elf-gcc` and `x86_64-elf-ld`:
   ```bash
   ./build_gnu_cc.sh
   ```
   Once the build is complete, you must add the cross-compiler to your environment variable `PATH` (the script will output the exact export command, usually `export PATH="$HOME/opt/cross/bin:$PATH"`).

## Build Instructions

To see the bootloader in action, you can build it along with an example kernel provided in the `example_kernel` directory. The included script will compile both the bootloader and the test kernel, pack them into a bootable ISO image (`nth_os.iso`), and automatically launch QEMU:

```bash
cd example_kernel
./mk_iso.sh
```

This script will automatically:
- Compile the example kernel into an ELF executable (`kernel.elf`).
- Configure and compile the bootloader (`BOOTX64.EFI`) via CMake.
- Create an EFI system partition image (`efi.img`).
- Copy the compiled EFI bootloader and kernel into the partition.
- Generate the final `nth_os.iso` using `xorriso`.
- Launch the ISO in QEMU using OVMF UEFI firmware.

Alternatively, if you only want to build the bootloader and run QEMU directly via a local FAT directory (without creating an ISO), you can run the following from the root directory (ensure your `esp/` directory has a kernel!):
```bash
./build_and_run.sh
```

## Booting Manually

<details>
<summary><b>QEMU Setup</b></summary>

To manually launch the generated ISO using QEMU, you need the OVMF UEFI firmware files. Run the following command from the root directory:

```bash
qemu-system-x86_64 \
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
    -drive if=pflash,format=raw,file=OVMF_VARS_4M.fd \
    -cdrom nth_os.iso \
    -m 256M
```

- If you need to access the OVMF UEFI firmware settings (the built-in UEFI Boot Manager), press the `ESC` key rapidly as soon as the QEMU window appears.

</details>

<details>
<summary><b>VirtualBox Setup</b></summary>

To boot the ISO in VirtualBox, you must enable EFI.
1. Create a new virtual machine and select the generated ISO (`nth_os.iso`).
    - Select  the `iso` image file you generated in the `mk_iso.sh` script.
    - Set OS to `Other`
    - Set OS Version to `Other/Unknow (64-bit)`

    ![VirtualBox Machine](assets/vmachine.png?raw=true "VirtualBox Machine")

2. In the VM settings, go to the **System** tab.
    - Increase base memory to `1024 MB`.
    - Enable `UEFI`.
    - Enable `I/O APIC`.

    ![VirtualBox Settings 2](assets/vboxset2.png?raw=true "VirtualBox Settings 2")


3. In the VM settings, go to the **Display** tab.
    - Increase Video Memory to the maximum.

    ![VirtualBox Settings 1](assets/vboxset1.png?raw=true "VirtualBox Settings 1")

4. Boot up the VM.


</details>

## Custom Kernels & OS Integration

`nth` can boot any custom OS kernel that adheres to the **nth boot protocol** (via a 64-bit ELF executable) or standard **Linux EFI stub** kernels (via PE/COFF).

> [!TIP]
> If you want a quick start, check out the [nth-c-template](https://github.com/Novfensec/nth-c-template) repository to instantly bootstrap your C kernel development.

### Boot Manager Configuration (`nth.cfg`)

The bootloader features a built-in graphical boot manager. It populates its menu by parsing an `nth.cfg` file located in the root of the EFI partition. You can define up to 9 kernel entries in this file, using the simple `Name=Path` format:

```ini
NTH OS=\kernel.elf
Alpine Linux=\vmlinuz-virt|vmlinuz-virt initrd=\initramfs-virt modules=loop,squashfs,sd-mod,usb-storage console=tty0 quiet
Memory Tester=\memtest.elf
```

If `nth.cfg` is missing or fails to load, the boot manager will default to attempting to load `\kernel.elf`. A "Reboot System" option is always appended to the end of the menu automatically.

### Linux Boot Support

Modern Linux kernels are typically compiled with the EFI stub (acting as PE/COFF UEFI applications). `nth` seamlessly detects these kernels. 

When writing an entry for Linux in `nth.cfg`, use the pipe character (`|`) to separate the kernel path from the command line arguments. The bootloader will:
1. Load the kernel using the UEFI `LoadImage` service.
2. Pass the everything after the `|` to the kernel as `LoadOptions` (which the Linux EFI stub parses as its command line, e.g., to load the `initrd`).
3. Boot the kernel natively using `StartImage`.

```ini
Alpine Linux=\vmlinuz-virt|vmlinuz-virt initrd=\initramfs-virt modules=loop,squashfs,sd-mod,usb-storage console=tty0 quiet
```

### Linking Your Kernel (`linker.ld`)

When writing your own kernel, your linker script (`linker.ld`) dictates exactly where your kernel expects to execute in virtual memory.

Because `nth` automatically creates a **Higher Half Direct Map (HHDM)** and puts you in 64-bit long mode, you should link your kernel in the higher half of the address space. For example, our provided [`example_kernel/linker.ld`](example_kernel/linker.ld) links the kernel at `0xFFFFFFFF80100000` (the classic -2GB higher half mark). This maps cleanly down to the physical `0x100000` (1MB) mark, which safely avoids legacy BIOS areas, IVTs, and memory-mapped IO that clutter the lower 1MB of physical RAM.

Here is the exact reference linker script used by our example kernel:

```ld
ENTRY(kernel_main)
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

SECTIONS
{
    /* Set the virtual base load address to the Higher Half */
    . = 0xFFFFFFFF80100000;

    .text : ALIGN(4K) {
        *(.text .text.*)
    }

    .rodata : ALIGN(4K) {
        *(.rodata .rodata.*)
    }

    .data : ALIGN(4K) {
        *(.data .data.*)
    }

    .bss : ALIGN(4K) {
        *(COMMON)
        *(.bss .bss.*)
    }

    /DISCARD/ : {
        *(.eh_frame)
        *(.note .note.*)
        *(.comment)
    }
}
```

The bootloader dynamically reads the `e_entry` field from your compiled ELF header to determine the entry point. This means you can name your entry function whatever you like (e.g., `_start`, `kmain`, `kernel_main`) as long as you explicitly define it using the `ENTRY()` directive at the top of your linker script.

### The Nth Protocol

When the bootloader transfers control to your kernel's entry point, it passes a pointer to an `NthBootInfo` structure as the first argument (System V ABI, passed in the `%rdi` register).

You can find the exact definitions for these structures in [`src/nth_protocol.h`](src/nth_protocol.h). To interface with the bootloader, simply copy this header file into your kernel project and include it:

```c
#include "nth_protocol.h"
```

Your kernel entry point must accept this parameter. For example:

```c
void kernel_main(NthBootInfo *boot_info) {
    // Use boot_info->Framebuffer to draw to the screen
    // Parse boot_info->MemoryMap to set up physical memory management
    // Read boot_info->Rsdp to initialize ACPI

    while (1) {
        __asm__("hlt");
    }
}
```

### Parsing the Memory Map

You might notice that `MemoryMap` is a `void *` instead of an `NthMemoryDescriptor *`. This is because the UEFI specification does not guarantee that the descriptors in the array are contiguous by exactly `sizeof(NthMemoryDescriptor)`. Motherboards often add extra padding bytes between entries. 

To safely iterate the map and find usable RAM without crashing, you **must** use `DescriptorSize` for pointer arithmetic before casting to the struct. Here is how you do it:

```c
uint64_t num_entries = boot_info->MapSize / boot_info->DescriptorSize;

for (uint64_t i = 0; i < num_entries; i++) {
    // Calculate the exact byte offset using DescriptorSize
    void *raw_pointer = (uint8_t *)boot_info->MemoryMap + (i * boot_info->DescriptorSize);

    // Safely cast to our struct
    NthMemoryDescriptor *desc = (NthMemoryDescriptor *)raw_pointer;

    // Check if this chunk is safe to use as system RAM
    if (desc->Type == NthEfiConventionalMemory) {
        // We found usable memory! 
        // Start: desc->PhysicalStart
        // Size: desc->NumberOfPages * 4096
    }
}
```

### Memory Mapping & Higher Half Support

Before jumping to your ELF kernel, `nth` automatically creates a new page table (PML4) and exits UEFI Boot Services:

1. **Identity Mapping**: All available physical memory (up to at least 4GB, or the highest physical address reported by UEFI) is identity mapped.
2. **Higher Half Direct Map (HHDM)**: All physical memory is also mapped to the higher half of the address space starting at `0xFFFF800000000000`.
3. **Huge Pages**: The mapping uses 2MB huge pages (`PAGE_HUGE`) to minimize TLB usage and page table overhead.

You are placed in **64-bit long mode** with the `CR3` register already pointing to this page table. You do not have a GDT or IDT configured yet.
