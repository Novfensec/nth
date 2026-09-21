#include "file.h"

EFI_STATUS LoadFile(EFI_HANDLE ImageHandle, CHAR16* FileName, VOID** Buffer, UINTN* BufferSize) {
    EFI_LOADED_IMAGE *LoadedImage = NULL;
    EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE *FileSystem = NULL;
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE *Root = NULL;
    EFI_FILE *File = NULL;
    EFI_FILE_INFO *FileInfo = NULL;
    EFI_GUID fiGuid = EFI_FILE_INFO_ID;
    UINTN FileInfoSize = 0;
    EFI_STATUS Status;

    uefi_call_wrapper(BS->HandleProtocol, 3, ImageHandle, &lipGuid, (VOID**)&LoadedImage);
    uefi_call_wrapper(BS->HandleProtocol, 3, LoadedImage->DeviceHandle, &fsGuid, (VOID**)&FileSystem);
    uefi_call_wrapper(FileSystem->OpenVolume, 2, FileSystem, &Root);

    Status = uefi_call_wrapper(Root->Open, 5, Root, &File, FileName, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    uefi_call_wrapper(File->GetInfo, 4, File, &fiGuid, &FileInfoSize, NULL);
    uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, FileInfoSize, (VOID**)&FileInfo);
    uefi_call_wrapper(File->GetInfo, 4, File, &fiGuid, &FileInfoSize, FileInfo);

    *BufferSize = FileInfo->FileSize;
    uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, *BufferSize, Buffer);
    uefi_call_wrapper(File->Read, 3, File, BufferSize, *Buffer);

    uefi_call_wrapper(BS->FreePool, 1, FileInfo);
    uefi_call_wrapper(File->Close, 1, File);
    uefi_call_wrapper(Root->Close, 1, Root);

    return EFI_SUCCESS;
}
