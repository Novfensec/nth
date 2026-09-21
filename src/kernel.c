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
    if (c < 32 || c > 127)
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

    while (1)
    {
        __asm__("hlt");
    }
}
