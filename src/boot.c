#include <efi.h>
#include <efilib.h>
#include "file.h"
#include "elf.h"
#include "gop.h"
#include "memory.h"
#include "boot_info.h"
#include "menu.h"
#include "config.h"

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    Framebuffer fb;
    InitializeGOP(&fb);

    BootEntry entries[10];
    UINTN entry_count = 0;

    VOID* ConfigBuffer = NULL;
    UINTN ConfigSize = 0;

    if (!EFI_ERROR(LoadFile(ImageHandle, L"\\nth.cfg", &ConfigBuffer, &ConfigSize))) {
        entry_count = ParseConfig(ConfigBuffer, ConfigSize, entries, 9);
        uefi_call_wrapper(BS->FreePool, 1, ConfigBuffer);
    } else {
        entries[0].Name = L"NTH OS (Default)";
        entries[0].KernelPath = L"\\kernel.elf";
        entry_count = 1;
    }

    entries[entry_count].Name = L"Reboot System";
    entries[entry_count].KernelPath = NULL;
    entry_count++;

    CHAR16* SelectedKernel = ShowGraphicalMenu(SystemTable, &fb, entries, entry_count);

    if (SelectedKernel == NULL) {
        uefi_call_wrapper(RT->ResetSystem, 4, EfiResetCold, EFI_SUCCESS, 0, NULL);
    }

    VOID* KernelBuffer = NULL;
    UINTN KernelSize = 0;
    LoadFile(ImageHandle, SelectedKernel, &KernelBuffer, &KernelSize);

    UINT64 EntryPoint = LoadELF(KernelBuffer);
    typedef void (*KernelStart)(BootInfo*);
    KernelStart kernel_main = (KernelStart)EntryPoint;

    BootFramebuffer boot_fb;
    boot_fb.BaseAddress = fb.BaseAddress;
    boot_fb.BufferSize = fb.BufferSize;
    boot_fb.Width = fb.Width;
    boot_fb.Height = fb.Height;
    boot_fb.PixelsPerScanLine = fb.PixelsPerScanLine;

    EFI_MEMORY_DESCRIPTOR* MemoryMap = NULL;
    UINTN MapSize, MapKey, DescriptorSize;
    ReadMemoryMap(&MemoryMap, &MapSize, &MapKey, &DescriptorSize);

    EFI_STATUS Status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey);
    if (EFI_ERROR(Status)) {
        ReadMemoryMap(&MemoryMap, &MapSize, &MapKey, &DescriptorSize);
        uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey);
    }

    BootInfo boot_info;
    boot_info.Framebuffer = &boot_fb;
    boot_info.MemoryMap = (void*)MemoryMap;
    boot_info.MapSize = MapSize;
    boot_info.DescriptorSize = DescriptorSize;

    kernel_main(&boot_info);

    while(1) {
        __asm__("hlt");
    }
    
    return EFI_SUCCESS;
}
