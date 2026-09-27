# nth

`nth` is an open-source, UEFI bootloader and the reference implementation for the `nth` boot protocol. It provides a built-in graphical boot manager and supports booting 64-bit ELF kernels.

### Screenshots
![nth Boot Menu](assets/homepage.png?raw=true "nth Boot Menu")

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

When writing your own kernel, your linker script (`linker.ld`) can specify any virtual base address (for example, the default `nth` kernel uses `. = 0x100000;` for physical identity mapping at the 1MB mark). 

The bootloader dynamically reads the `e_entry` field from the ELF header to determine the entry point. This means you can name your entry function whatever you like (`_start`, `kernel_main`, etc.) as long as it is correctly specified via the `ENTRY()` directive in your linker script.

### The Nth Protocol

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

### Memory Mapping & Higher Half Support

Before jumping to your ELF kernel, `nth` automatically creates a new page table (PML4) and exits UEFI Boot Services. It sets up a highly convenient environment out-of-the-box:

1. **Identity Mapping**: All available physical memory (up to at least 4GB, or the highest physical address reported by UEFI) is identity mapped.
2. **Higher Half Direct Map (HHDM)**: All physical memory is also mapped to the higher half of the address space starting at `0xFFFF800000000000`.
3. **Huge Pages**: The mapping uses 2MB huge pages (`PAGE_HUGE`) to minimize TLB usage and page table overhead.

You are placed in **64-bit long mode** with the `CR3` register already pointing to this page table. You do not have a GDT or IDT configured yet. This allows you to easily design a higher-half kernel without writing a messy trampoline or initial assembly boot routine.
