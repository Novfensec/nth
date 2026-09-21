#include "menu.h"
#include "font.h"

static void DrawPixel(Framebuffer* fb, UINT32 x, UINT32 y, UINT32 color) {
    if (x >= fb->Width || y >= fb->Height) return;
    UINT32* video_memory = (UINT32*)fb->BaseAddress;
    video_memory[y * fb->PixelsPerScanLine + x] = color;
}

static void DrawChar(Framebuffer* fb, char c, UINT32 x, UINT32 y, UINT32 scale, UINT32 color) {
    if (c < 32 || c > 127) return;
    UINT8* glyph = (UINT8*)font8x8[(int)c - 32];

    for (UINT32 cy = 0; cy < 8; cy++) {
        for (UINT32 cx = 0; cx < 8; cx++) {
            if ((glyph[cy] & (0x80 >> cx))) {
                for (UINT32 sy = 0; sy < scale; sy++) {
                    for (UINT32 sx = 0; sx < scale; sx++) {
                        DrawPixel(fb, x + (cx * scale) + sx, y + (cy * scale) + sy, color);
                    }
                }
            }
        }
    }
}

static void DrawString(Framebuffer* fb, const CHAR16* str, UINT32 x, UINT32 y, UINT32 scale, UINT32 color) {
    while (*str) {
        char ascii = (*str < 128) ? (char)*str : '?';
        DrawChar(fb, ascii, x, y, scale, color);
        x += 8 * scale;
        str++;
    }
}

CHAR16* ShowGraphicalMenu(EFI_SYSTEM_TABLE *SystemTable, Framebuffer* fb, BootEntry* Entries, UINTN EntryCount) {
    UINTN SelectedIndex = 0;
    EFI_INPUT_KEY Key;
    EFI_STATUS Status;

    uefi_call_wrapper(SystemTable->ConIn->Reset, 2, SystemTable->ConIn, FALSE);

    while (1) {

        UINT32* video_memory = (UINT32*)fb->BaseAddress;
        for (UINT32 y = 0; y < fb->Height; y++) {
            for (UINT32 x = 0; x < fb->Width; x++) {
                video_memory[y * fb->PixelsPerScanLine + x] = 0x00000000;
            }
        }

        DrawString(fb, L"NTH BOOT MANAGER", 80, 60, 2, 0x00FFFFFF);

        UINT32 start_y = 130;
        for (UINTN i = 0; i < EntryCount; i++) {
            UINT32 color = (i == SelectedIndex) ? 0x0000FF00 : 0x00FFFFFF; 
            CHAR16 buffer[128];

            if (i == SelectedIndex) {
                StrCpy(buffer, L"> ");
                StrCat(buffer, Entries[i].Name);
            } else {
                StrCpy(buffer, L"  ");
                StrCat(buffer, Entries[i].Name);
            }

            DrawString(fb, buffer, 80, start_y + (i * 24), 1, color);
        }

        DrawString(fb, L"Use Up/Down arrows to navigate, Enter to boot.", 80, fb->Height - 50, 1, 0x00777777);

        while (1) {
            Status = uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, &Key);
            if (!EFI_ERROR(Status)) {
                break;
            }
            uefi_call_wrapper(BS->Stall, 1, 20000);
        }

        if (Key.ScanCode == SCAN_UP) {
            if (SelectedIndex > 0) SelectedIndex--;
        } else if (Key.ScanCode == SCAN_DOWN) {
            if (SelectedIndex < EntryCount - 1) SelectedIndex++;
        } else if (Key.UnicodeChar == L'\r' || Key.UnicodeChar == L'\n') {
            return Entries[SelectedIndex].KernelPath;
        }
    }
}
