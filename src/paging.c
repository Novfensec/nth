#include "paging.h"

static UINT64 *AllocatePageTable(void)
{
    EFI_PHYSICAL_ADDRESS Addr;
    EFI_STATUS Status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, 1, &Addr);
    if (EFI_ERROR(Status))
    {
        return NULL;
    }
    UINT64 *Ptr = (UINT64 *)Addr;
    for (int i = 0; i < 512; i++)
    {
        Ptr[i] = 0;
    }
    return Ptr;
}

void MapPage(UINT64 *PML4, UINT64 VirtualAddress, UINT64 PhysicalAddress, UINT64 Flags)
{
    if (PML4 == NULL)
        return;

    UINT16 pml4_index = (VirtualAddress >> 39) & 0x1FF;
    UINT16 pdpt_index = (VirtualAddress >> 30) & 0x1FF;
    UINT16 pd_index = (VirtualAddress >> 21) & 0x1FF;
    UINT16 pt_index = (VirtualAddress >> 12) & 0x1FF;

    if (!(PML4[pml4_index] & PAGE_PRESENT))
    {
        UINT64 *PDPT = AllocatePageTable();
        PML4[pml4_index] = (UINT64)PDPT | PAGE_PRESENT | PAGE_RW;
    }
    UINT64 *PDPT = (UINT64 *)(PML4[pml4_index] & ~0xFFFULL);

    if (!(PDPT[pdpt_index] & PAGE_PRESENT))
    {
        UINT64 *PD = AllocatePageTable();
        PDPT[pdpt_index] = (UINT64)PD | PAGE_PRESENT | PAGE_RW;
    }
    UINT64 *PD = (UINT64 *)(PDPT[pdpt_index] & ~0xFFFULL);

    if (Flags & PAGE_HUGE)
    {
        PD[pd_index] = PhysicalAddress | Flags | PAGE_PRESENT;
    }
    else
    {
        if (!(PD[pd_index] & PAGE_PRESENT))
        {
            UINT64 *PT = AllocatePageTable();
            PD[pd_index] = (UINT64)PT | PAGE_PRESENT | PAGE_RW;
        }
        UINT64 *PT = (UINT64 *)(PD[pd_index] & ~0xFFFULL);
        PT[pt_index] = PhysicalAddress | Flags | PAGE_PRESENT;
    }
}

UINT64* SetupPaging(EFI_MEMORY_DESCRIPTOR *MemoryMap, UINTN MapSize, UINTN DescriptorSize)
{
    UINT64 *PML4 = AllocatePageTable();

    UINT64 HighestAddress = 0;
    UINTN NumEntries = MapSize / DescriptorSize;
    for (UINTN i = 0; i < NumEntries; i++)
    {
        EFI_MEMORY_DESCRIPTOR *Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)MemoryMap + (i * DescriptorSize));
        UINT64 EndAddr = Desc->PhysicalStart + (Desc->NumberOfPages * PAGE_SIZE);
        if (EndAddr > HighestAddress)
        {
            HighestAddress = EndAddr;
        }
    }

    if (HighestAddress < 0x100000000ULL)
    {
        HighestAddress = 0x100000000ULL;
    }

    for (UINT64 Addr = 0; Addr < HighestAddress; Addr += PAGE_SIZE_2MB)
    {
        MapPage(PML4, Addr, Addr, PAGE_PRESENT | PAGE_RW | PAGE_HUGE);
        MapPage(PML4, HHDM_BASE + Addr, Addr, PAGE_PRESENT | PAGE_RW | PAGE_HUGE);
    }
    
    return PML4;
}

void SwitchPageTable(UINT64 *PML4)
{
    if (PML4 != NULL)
    {
        __asm__ volatile("mov %0, %%cr3" : : "r"(PML4));
    }
}
