#include "elf.h"

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

UINT64 LoadELF(VOID *FileBuffer)
{
    Elf64_Ehdr *Header = (Elf64_Ehdr *)FileBuffer;
    Elf64_Phdr *Phdrs = (Elf64_Phdr *)((UINT8 *)FileBuffer + Header->e_phoff);

    for (UINT16 i = 0; i < Header->e_phnum; i++)
    {
        Elf64_Phdr *Phdr = &Phdrs[i];

        if (Phdr->p_type == PT_LOAD)
        {
            UINT8 *Dest = (UINT8 *)Phdr->p_paddr;
            UINT8 *Src = ((UINT8 *)FileBuffer) + Phdr->p_offset;

            for (UINT64 j = 0; j < Phdr->p_filesz; j++)
            {
                Dest[j] = Src[j];
            }

            for (UINT64 j = Phdr->p_filesz; j < Phdr->p_memsz; j++)
            {
                Dest[j] = 0;
            }
        }
    }

    return Header->e_entry;
}
