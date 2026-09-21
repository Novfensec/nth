#include "gop.h"

EFI_STATUS InitializeGOP(Framebuffer *fb)
{
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_STATUS Status;

    Status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (VOID **)&gop);
    if (EFI_ERROR(Status))
    {
        return Status;
    }

    fb->BaseAddress = gop->Mode->FrameBufferBase;
    fb->BufferSize = gop->Mode->FrameBufferSize;
    fb->Width = gop->Mode->Info->HorizontalResolution;
    fb->Height = gop->Mode->Info->VerticalResolution;
    fb->PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;

    return EFI_SUCCESS;
}
