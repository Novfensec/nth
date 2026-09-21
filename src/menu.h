#ifndef MENU_H
#define MENU_H

#include <efi.h>
#include <efilib.h>
#include "gop.h"

typedef struct {
    CHAR16* Name;
    CHAR16* KernelPath;
} BootEntry;

CHAR16* ShowGraphicalMenu(EFI_SYSTEM_TABLE *SystemTable, Framebuffer* fb, BootEntry* Entries, UINTN EntryCount);

#endif
