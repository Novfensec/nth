#ifndef FILE_H
#define FILE_H

#include <efi.h>
#include <efilib.h>

EFI_STATUS LoadFile(EFI_HANDLE ImageHandle, CHAR16 *FileName, VOID **Buffer, UINTN *BufferSize);

#endif
