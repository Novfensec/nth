#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <stdint.h>

typedef struct {
    uint64_t BaseAddress;
    uint64_t BufferSize;
    uint32_t Width;
    uint32_t Height;
    uint32_t PixelsPerScanLine;
} BootFramebuffer;

typedef struct {
    BootFramebuffer* Framebuffer;
    void* MemoryMap;
    uint64_t MapSize;
    uint64_t DescriptorSize;
} BootInfo;

#endif
