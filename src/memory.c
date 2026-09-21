#include "memory.h"

EFI_STATUS ReadMemoryMap(EFI_MEMORY_DESCRIPTOR** Map, UINTN* MapSize, UINTN* MapKey, UINTN* DescriptorSize) {
    UINT32 DescriptorVersion;
    *MapSize = 0;
    *Map = NULL;

    uefi_call_wrapper(BS->GetMemoryMap, 5, MapSize, *Map, MapKey, DescriptorSize, &DescriptorVersion);

    *MapSize += *DescriptorSize * 2;

    EFI_STATUS Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, *MapSize, (VOID**)Map);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    Status = uefi_call_wrapper(BS->GetMemoryMap, 5, MapSize, *Map, MapKey, DescriptorSize, &DescriptorVersion);

    return Status;
}
