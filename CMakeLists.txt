cmake_minimum_required(VERSION 3.15)
project(nth_modern C)

set(GNU_EFI_INCLUDE "/usr/include/efi")
set(GNU_EFI_ARCH_INCLUDE "/usr/include/efi/x86_64")
set(GNU_EFI_LIB "/usr/lib")
set(EFI_CRT0 "/usr/lib/crt0-efi-x86_64.o")
set(EFI_LDS "/usr/lib/elf_x86_64_efi.lds")

set(CMAKE_C_FLAGS "-fpic -ffreestanding -fno-stack-protector -fno-strict-aliasing -fshort-wchar -mno-red-zone -Wall")
add_library(bootloader SHARED src/boot.c src/file.c src/elf.c src/gop.c src/memory.c src/menu.c src/config.c)
target_include_directories(bootloader PRIVATE ${GNU_EFI_INCLUDE} ${GNU_EFI_ARCH_INCLUDE})

target_link_options(bootloader PRIVATE 
    -nostdlib -Wl,-nostdlib -Wl,--warn-common -Wl,--no-undefined -Wl,--fatal-warnings 
    -Wl,-shared -Wl,-Bsymbolic -L${GNU_EFI_LIB} -T${EFI_LDS} ${EFI_CRT0}
)
target_link_libraries(bootloader efi gnuefi)

add_custom_command(TARGET bootloader POST_BUILD
    COMMAND objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel 
            -j .rela -j .reloc --target=efi-app-x86_64 
            $<TARGET_FILE:bootloader> ${CMAKE_BINARY_DIR}/BOOTX64.EFI
)

set(KERNEL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/kernel.c")
set(KERNEL_LINKER "${CMAKE_CURRENT_SOURCE_DIR}/src/linker.ld")
set(KERNEL_ELF "${CMAKE_CURRENT_SOURCE_DIR}/esp/kernel.elf")

set(KERNEL_C_FLAGS -ffreestanding -mno-red-zone -Wall -Wextra -O2)

add_custom_command(
    OUTPUT ${KERNEL_ELF}
    COMMAND x86_64-elf-gcc ${KERNEL_C_FLAGS} -c ${KERNEL_SRC} -o ${CMAKE_CURRENT_BINARY_DIR}/kernel.o
    COMMAND x86_64-elf-ld -T ${KERNEL_LINKER} ${CMAKE_CURRENT_BINARY_DIR}/kernel.o -o ${KERNEL_ELF}
    DEPENDS ${KERNEL_SRC} ${KERNEL_LINKER}
)

add_custom_target(kernel ALL DEPENDS ${KERNEL_ELF})
