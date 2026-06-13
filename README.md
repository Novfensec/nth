# nth

nth is an open source bootloader.

# Setup

## Windows

> [!NOTE]
> You must have Visual Studio or the standalone **Visual C++ Build Tools** installed (specifically the "Desktop development with C++" workload) for CMake to successfully generate the build files.*

```powershell
# install visual studio build tools
winget install -e --id Microsoft.VisualStudio.BuildTools --override "--passive --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"

# install nasm
winget install -e --id NASM.NASM

# install qemu
winget install -e --id SoftwareFreedomConservancy.QEMU

# install cmake
winget install -e --id Kitware.CMake

```

## Linux

Run the command corresponding to your distribution. This installs NASM, QEMU, CMake, and the standard compiler toolchains (the Linux equivalent of the C++ Build Tools).

**Ubuntu / Debian**

```bash
sudo apt update
sudo apt install nasm qemu-system-x86 cmake build-essential

```

**Fedora**

```bash
sudo dnf install nasm qemu-system-x86 cmake @development-tools

```

**Arch Linux**

```bash
sudo pacman -S nasm qemu-system-x86 cmake base-devel

```

# Build Instructions

> Terminal / PowerShell
> ```shell
> mkdir build; cd build
> cmake ..
> cmake --build .
> 
> 
> ```
> 
> 

# Booting

> Terminal / PowerShell
> ```shell
> qemu-system-x86_64 -drive format=raw,file=build/bin/nth.bin -no-reboot
> 
> ```
> 
>