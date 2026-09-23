#ifndef PAGING_H
#define PAGING_H

#include <efi.h>
#include <efilib.h>

#define PAGE_SIZE 4096
#define PAGE_SIZE_2MB (2 * 1024 * 1024)

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_RW (1ULL << 1)
#define PAGE_USER (1ULL << 2)
#define PAGE_HUGE (1ULL << 7)

#define HHDM_BASE 0xFFFF800000000000ULL

UINT64* SetupPaging(EFI_MEMORY_DESCRIPTOR *MemoryMap, UINTN MapSize, UINTN DescriptorSize);
void MapPage(UINT64 *PML4, UINT64 VirtualAddress, UINT64 PhysicalAddress, UINT64 Flags);
void SwitchPageTable(UINT64 *PML4);

#endif
