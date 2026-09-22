#ifndef NTH_PROTOCOL_H
#define NTH_PROTOCOL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        uint64_t BaseAddress;
        uint64_t BufferSize;
        uint32_t Width;
        uint32_t Height;
        uint32_t PixelsPerScanLine;
    } NthFramebuffer;

    typedef enum
    {
        NthEfiReservedMemoryType,
        NthEfiLoaderCode,
        NthEfiLoaderData,
        NthEfiBootServicesCode,
        NthEfiBootServicesData,
        NthEfiRuntimeServicesCode,
        NthEfiRuntimeServicesData,
        NthEfiConventionalMemory,
        NthEfiUnusableMemory,
        NthEfiACPIReclaimMemory,
        NthEfiACPIMemoryNVS,
        NthEfiMemoryMappedIO,
        NthEfiMemoryMappedIOPortSpace,
        NthEfiPalCode,
        NthEfiPersistentMemory,
        NthEfiMaxMemoryType
    } NthMemoryType;

    typedef struct
    {
        uint32_t Type;
        uint32_t Pad;
        uint64_t PhysicalStart;
        uint64_t VirtualStart;
        uint64_t NumberOfPages;
        uint64_t Attribute;
    } __attribute__((packed)) NthMemoryDescriptor;

    typedef struct
    {
        NthFramebuffer *Framebuffer;
        void *MemoryMap;
        uint64_t MapSize;
        uint64_t DescriptorSize;
        void *Rsdp;
    } NthBootInfo;

#ifdef __cplusplus
}
#endif

#endif
