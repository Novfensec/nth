#ifndef MEMORY_H
#define MEMORY_H

#include <efi.h>
#include <efilib.h>

EFI_STATUS ReadMemoryMap(EFI_MEMORY_DESCRIPTOR** Map, UINTN* MapSize, UINTN* MapKey, UINTN* DescriptorSize);

#endif
