#include "elf.h"
#include "paging.h"

BOOLEAN ValidateELF(VOID *FileBuffer)
{
    Elf64_Ehdr *Header = (Elf64_Ehdr *)FileBuffer;

    if (Header->e_ident[0] != 0x7F ||
        Header->e_ident[1] != 'E' ||
        Header->e_ident[2] != 'L' ||
        Header->e_ident[3] != 'F')
    {
        return FALSE;
    }

    if (Header->e_ident[4] != 2)
    {
        return FALSE;
    }

    if (Header->e_machine != 0x3E)
    {
        return FALSE;
    }

    return TRUE;
}

UINT64 LoadELF(VOID *FileBuffer, UINT64 *PML4)
{
    Elf64_Ehdr *Header = (Elf64_Ehdr *)FileBuffer;
    Elf64_Phdr *Phdrs = (Elf64_Phdr *)((UINT8 *)FileBuffer + Header->e_phoff);

    for (UINT16 i = 0; i < Header->e_phnum; i++)
    {
        Elf64_Phdr *Phdr = &Phdrs[i];

        if (Phdr->p_type == PT_LOAD)
        {
            UINT64 Start = Phdr->p_vaddr & ~0xFFFULL;
            UINT64 End = (Phdr->p_vaddr + Phdr->p_memsz + 0xFFF) & ~0xFFFULL;
            UINTN Pages = (End - Start) / PAGE_SIZE;

            EFI_PHYSICAL_ADDRESS PhysicalAddr;
            EFI_STATUS Status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, Pages, &PhysicalAddr);
            if (EFI_ERROR(Status))
            {
                return 0;
            }

            UINT8 *Dest = (UINT8 *)PhysicalAddr + (Phdr->p_vaddr & 0xFFF);
            UINT8 *Src = ((UINT8 *)FileBuffer) + Phdr->p_offset;

            for (UINT64 j = 0; j < Phdr->p_filesz; j++)
            {
                Dest[j] = Src[j];
            }

            for (UINT64 j = Phdr->p_filesz; j < Phdr->p_memsz; j++)
            {
                Dest[j] = 0;
            }

            for (UINT64 j = 0; j < Pages; j++)
            {
                MapPage(PML4, Start + (j * PAGE_SIZE), PhysicalAddr + (j * PAGE_SIZE), PAGE_PRESENT | PAGE_RW);
            }
        }
    }

    return Header->e_entry;
}
