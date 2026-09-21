#ifndef NTH_PROTOCOL_H
#define NTH_PROTOCOL_H

#include <stdint.h>

typedef struct
{
    uint64_t BaseAddress;
    uint64_t BufferSize;
    uint32_t Width;
    uint32_t Height;
    uint32_t PixelsPerScanLine;
} NthFramebuffer;

typedef struct
{
    NthFramebuffer *Framebuffer;
    void *MemoryMap;
    uint64_t MapSize;
    uint64_t DescriptorSize;
    void *Rsdp;
} NthBootInfo;

#endif
