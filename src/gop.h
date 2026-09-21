#ifndef GOP_H
#define GOP_H

#include <efi.h>
#include <efilib.h>

typedef struct
{
    UINT64 BaseAddress;
    UINT64 BufferSize;
    UINT32 Width;
    UINT32 Height;
    UINT32 PixelsPerScanLine;
} Framebuffer;

EFI_STATUS InitializeGOP(Framebuffer *fb);

#endif
