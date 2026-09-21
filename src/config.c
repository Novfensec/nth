#include "config.h"

VOID AsciiToUnicode(CHAR8* AsciiStr, CHAR16* UnicodeStr, UINTN MaxLen) {
    UINTN i = 0;
    while (AsciiStr[i] != '\0' && AsciiStr[i] != '\r' && AsciiStr[i] != '\n' && i < MaxLen - 1) {
        UnicodeStr[i] = (CHAR16)AsciiStr[i];
        i++;
    }
    UnicodeStr[i] = L'\0';
}

UINTN ParseConfig(VOID* FileBuffer, UINTN FileSize, BootEntry* Entries, UINTN MaxEntries) {
    CHAR8* Buffer = (CHAR8*)FileBuffer;
    UINTN EntryCount = 0;
    UINTN i = 0;

    while (i < FileSize && EntryCount < MaxEntries) {
        while (i < FileSize && (Buffer[i] == '\r' || Buffer[i] == '\n' || Buffer[i] == ' ' || Buffer[i] == '\t')) i++;
        if (i >= FileSize) break;

        CHAR8 NameBuf[64] = {0};
        UINTN n = 0;
        while (i < FileSize && Buffer[i] != '=' && Buffer[i] != '\n' && Buffer[i] != '\r' && n < 63) {
            NameBuf[n++] = Buffer[i++];
        }
        NameBuf[n] = '\0';

        if (i < FileSize && Buffer[i] == '=') i++;

        CHAR8 PathBuf[64] = {0};
        UINTN p = 0;
        while (i < FileSize && Buffer[i] != '\r' && Buffer[i] != '\n' && p < 63) {
            PathBuf[p++] = Buffer[i++];
        }
        PathBuf[p] = '\0';

        while (p > 0 && (PathBuf[p-1] == ' ' || PathBuf[p-1] == '\r' || PathBuf[p-1] == '\n')) {
            PathBuf[--p] = '\0';
        }

        if (n > 0 && p > 0) {
            uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, 128, (VOID**)&Entries[EntryCount].Name);
            uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, 128, (VOID**)&Entries[EntryCount].KernelPath);

            AsciiToUnicode(NameBuf, Entries[EntryCount].Name, 64);
            AsciiToUnicode(PathBuf, Entries[EntryCount].KernelPath, 64);

            EntryCount++;
        }
    }
    
    return EntryCount;
}
