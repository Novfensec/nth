#include "nth_protocol.h"
#include "font.h"
#include <stdint.h>

void draw_pixel(NthFramebuffer *fb, uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= fb->Width || y >= fb->Height)
        return;
    uint32_t *video_memory = (uint32_t *)fb->BaseAddress;
    video_memory[y * fb->PixelsPerScanLine + x] = color;
}

void draw_char(NthFramebuffer *fb, char c, uint32_t x, uint32_t y, uint32_t scale, uint32_t color)
{
    if ((unsigned char)c < 32 || (unsigned char)c > 127)
        return;

    uint8_t *glyph = (uint8_t *)font8x8[(int)c - 32];

    for (uint32_t cy = 0; cy < 8; cy++)
    {
        for (uint32_t cx = 0; cx < 8; cx++)
        {
            if ((glyph[cy] & (0x80 >> cx)))
            {
                for (uint32_t sy = 0; sy < scale; sy++)
                {
                    for (uint32_t sx = 0; sx < scale; sx++)
                    {
                        draw_pixel(fb, x + (cx * scale) + sx, y + (cy * scale) + sy, color);
                    }
                }
            }
        }
    }
}

void draw_string(NthFramebuffer *fb, const char *str, uint32_t x, uint32_t y, uint32_t scale, uint32_t color)
{
    while (*str)
    {
        draw_char(fb, *str, x, y, scale, color);
        x += 8 * scale;
        str++;
    }
}

// Simple integer to string converter for our kernel
void itoa(uint64_t value, char *str)
{
    if (value == 0)
    {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    int i = 0;
    while (value != 0)
    {
        str[i++] = (value % 10) + '0';
        value /= 10;
    }
    str[i] = '\0';

    int start = 0;
    int end = i - 1;
    while (start < end)
    {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void kernel_main(NthBootInfo *boot_info)
{
    NthFramebuffer *fb = boot_info->Framebuffer;
    uint32_t *video_memory = (uint32_t *)fb->BaseAddress;

    for (uint32_t y = 0; y < fb->Height; y++)
    {
        for (uint32_t x = 0; x < fb->Width; x++)
        {
            video_memory[y * fb->PixelsPerScanLine + x] = 0x00000000;
        }
    }

    const char *message = "Nth by KARTAVYA SHUKLA";

    uint32_t msg_length = 0;
    while (message[msg_length] != '\0')
    {
        msg_length++;
    }

    uint32_t scale = 3;
    uint32_t pixel_width = msg_length * 8 * scale;
    uint32_t pixel_height = 8 * scale;

    uint32_t start_x = (fb->Width - pixel_width) / 2;
    uint32_t start_y = (fb->Height - pixel_height) / 2;

    draw_string(fb, message, start_x, start_y, scale, 0x00FFFFFF);

    // We will calculate total usable RAM in Megabytes.
    uint64_t total_usable_pages = 0;

    uint64_t num_entries = boot_info->MapSize / boot_info->DescriptorSize;

    for (uint64_t i = 0; i < num_entries; i++)
    {
        void *raw_pointer = (uint8_t *)boot_info->MemoryMap + (i * boot_info->DescriptorSize);

        NthMemoryDescriptor *desc = (NthMemoryDescriptor *)raw_pointer;

        if (desc->Type == NthEfiConventionalMemory)
        {
            total_usable_pages += desc->NumberOfPages;
        }
    }

    // A UEFI page is 4096 bytes (4KB).
    // Total Bytes = pages * 4096. Total MB = Total Bytes / (1024 * 1024)
    uint64_t usable_ram_mb = (total_usable_pages * 4096) / (1024 * 1024);

    char ram_str[32];
    itoa(usable_ram_mb, ram_str);

    draw_string(fb, "Usable RAM (MB): ", start_x, start_y + 40, 2, 0x0000FF00);
    draw_string(fb, ram_str, start_x + (17 * 16), start_y + 40, 2, 0x0000FF00);

    while (1)
    {
        __asm__("hlt");
    }
}
