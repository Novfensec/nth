# nth

`nth` is an open-source, UEFI bootloader and the reference implementation for the `nth` boot protocol. It provides a built-in graphical boot manager and supports booting 64-bit ELF kernels.

### Screenshots
![nth Boot Menu](assets/homepage.png?raw=true "nth Boot Menu")

### Supported architectures
* x86-64 (UEFI)

### Supported boot protocols
* [nth protocol](#the-handoff-protocol)

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

To build the UEFI bootloader (`BOOTX64.EFI`) and kernel (`kernel.elf`), and then pack them into a bootable ISO image (`nth_os.iso`):

```bash
./mk_iso.sh
```

This script will automatically:
1. Configure and compile the project via CMake in the `build/` directory.
2. Create an EFI system partition image (`efi.img`).
3. Copy the compiled EFI bootloader and kernel into the partition.
4. Generate the final `nth_os.iso` using `xorriso`.
5. Launch the ISO in QEMU using OVMF UEFI firmware.

Alternatively, if you only want to build the files and run QEMU directly via a local FAT directory (without creating an ISO), you can run:
```bash
./build_and_run.sh
```

## Booting Manually

To manually launch the generated ISO using QEMU, you need the OVMF UEFI firmware files. Run the following command from the root directory:

```bash
qemu-system-x86_64 \
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
    -drive if=pflash,format=raw,file=OVMF_VARS_4M.fd \
    -cdrom nth_os.iso \
    -m 256M
```

> [!TIP]
> If you need to access the OVMF UEFI firmware settings (the built-in UEFI Boot Manager), press the `ESC` key rapidly as soon as the QEMU window appears.

## Kernels / Boot Protocol

`nth` can boot any kernel that adheres to its universal boot protocol. The kernel must be compiled as a **64-bit ELF executable** (`x86_64-elf`).

### Boot Manager Tricks (`nth.cfg`)

The bootloader features a built-in graphical boot manager. It populates its menu by parsing an `nth.cfg` file located in the root of the EFI partition. You can define up to 9 kernel entries in this file, using the simple `Name=Path` format:

```ini
NTH OS=\kernel.elf
My Kernel=\vmlinuz
Memory Tester=\memtest.elf
```

If `nth.cfg` is missing or fails to load, the boot manager will default to attempting to load `\kernel.elf`. A "Reboot System" option is always appended to the end of the menu automatically.

### Linking Your Kernel (`linker.ld`)

When writing your own kernel, your linker script (`linker.ld`) can specify any virtual base address (for example, the default `nth` kernel uses `. = 0x100000;` for physical identity mapping at the 1MB mark). 

The bootloader dynamically reads the `e_entry` field from the ELF header to determine the entry point. This means you can name your entry function whatever you like (`_start`, `kernel_main`, etc.) as long as it is correctly specified via the `ENTRY()` directive in your linker script.

### The Handoff Protocol

When the bootloader transfers control to your kernel's entry point, it passes a pointer to an `NthBootInfo` structure as the first argument (System V ABI, passed in the `%rdi` register).

You should include the following definitions in your kernel code to interface with the bootloader:

```c
#include <stdint.h>

typedef struct {
    uint64_t BaseAddress;      // Physical base address of the framebuffer
    uint64_t BufferSize;       // Size of the framebuffer in bytes
    uint32_t Width;            // Screen width in pixels
    uint32_t Height;           // Screen height in pixels
    uint32_t PixelsPerScanLine;// Pixels per scanline (may include padding)
} NthFramebuffer;

typedef struct {
    NthFramebuffer *Framebuffer;
    void *MemoryMap;           // Pointer to the UEFI Memory Map
    uint64_t MapSize;          // Total size of the memory map in bytes
    uint64_t DescriptorSize;   // Size of each memory descriptor entry
    void *Rsdp;                // Pointer to the ACPI RSDP table (for ACPI 2.0+)
} NthBootInfo;
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

> [!Note]
> By the time your kernel executes, `nth` will have successfully exited UEFI Boot Services and initialized the linear GOP framebuffer. You are in 64-bit long mode without a GDT or IDT set up.
