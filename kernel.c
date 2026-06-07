#include <stdint.h>
#include <stddef.h>
#include "psf.h"
#include "multiboot.h"

extern char _binary_font_psf_start[];
extern char _binary_font_psf_end[];
char *fb = 0;
int scanline = 0;
int screen_width = 0;
int screen_height = 0;

#define PIXEL uint32_t
#define USHRT_MAX 65535

uint16_t *unicode;
size_t console_row;
size_t console_col;

void* memcpy(void *dest, const void *src, size_t n)
{
    unsigned char *d = (unsigned char*)dest;
    unsigned char *s = (unsigned char*)src;

    for(size_t i = 0; i < n; i++)
        d[i] = s[i];

    return dest;
}

void* memset(void *dest, int value, size_t n)
{
    unsigned char *d = (unsigned char*)dest;

    for(size_t i = 0; i < n; i++)
        d[i] = (unsigned char)value;

    return dest;
}

size_t strlen(const char* str)
{
    size_t len = 0;
    while(str[len])
        len++;
    return len;
}

void psf_init()
{
    uint16_t glyph = 0;
    // cast address to PSF header struct
    PSF_font *font = (PSF_font*)&_binary_font_psf_start;
    // check if there is a unicode table
    if (font->flags == 0)
    {
        unicode = NULL;
        return;
    }

    char *s = (char*)(
    (unsigned char*)&_binary_font_psf_start +
    font->headersize + font->numglyph * font->bytesperglyph);

    static uint16_t unicode_table[USHRT_MAX];
    unicode = unicode_table;

    while((unsigned char*)s < (unsigned char*)_binary_font_psf_end)
    {
        uint16_t uc = (uint16_t)(unsigned char)s[0];
        if(uc == 0xFF) {
            glyph++;
            s++;
            continue;
        }   else if(uc & 128)
        {
            if((uc & 32) == 0)
            {
                uc = ((s[0] & 0x1F)<<6)+(s[1] & 0x3F);
                s++;
            }   else
            if((uc & 16) == 0)
            {
                uc = ((((s[0] & 0xF)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F);
                s+=2;
            }   else
            if((uc & 8) == 0)
            {
                uc = ((((((s[0] & 0x7)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F))<<6)+(s[3] & 0x3F);
                s+=3;
            }   else
                uc = 0;
        }
        unicode[uc] = glyph;
        s++;
    }
}

void putchar(unsigned short int c, int cursor_x, int cursor_y, uint32_t fg, uint32_t bg)
{
    PSF_font *font = (PSF_font*)&_binary_font_psf_start;
    if(unicode != NULL)
    {
        c = unicode[c];
    }

    unsigned char *glyph = (unsigned char*)&_binary_font_psf_start +
    font->headersize +
    (c > 0 && c < font->numglyph?c:0)*font->bytesperglyph;

    int offs = (cursor_y * font->height * scanline) +
    (cursor_x * (font->width + 1) * sizeof(PIXEL));

    uint32_t bytesPerGlyphLine = (font->width + 7) / 8;
    uint32_t x, y;
    int line;
    for(y = 0; y < font->height; y++)
    {
        line = offs;
        unsigned char* currentByte = glyph + (bytesPerGlyphLine * y);
        uint8_t mask = 1 << 7;
        for(x = 0; x < font->width; x++)
        {
            *((PIXEL*)(fb + line)) = (*currentByte & mask) ? fg:bg;
            mask >>= 1;
            if(mask == 0)
            {
                mask = 1 << 7;
                currentByte += 1;
            }
            line += sizeof(PIXEL);
        }
        offs += scanline;
    }
}

void console_init()
{
    console_row = 0;
    console_col = 0;
    psf_init();
}

void console_scroll()
{
    PSF_font *font = (PSF_font*)&_binary_font_psf_start;

    size_t total_rows = screen_height / font->height;

    memcpy(fb,
           fb + (font->height * scanline),
           ((screen_height - font->height) * scanline));
    
    memset(fb + ((console_row - 1) * font->height * scanline),
           0x00,
           font->height * scanline
    );

    console_row = total_rows - 1;
}

void console_putchar(char c)
{
    PSF_font *font = (PSF_font*)&_binary_font_psf_start;

    size_t total_rows = screen_height / font->height;

    if(c == '\n')
    {
        console_col = 0;
        console_row++;
        if(console_row >= total_rows)
            console_scroll();
        return;
    }

    putchar((unsigned short int)c,
            console_col,
            console_row,
            0xFFFFFF,
            0x000000);

    if(++console_col == (scanline / ((font->width + 1) * sizeof(PIXEL))))
    {
        console_col = 0;
        console_row++;
        if(console_row >= total_rows)
            console_scroll();
    }
}

void console_write(const char* data, size_t size)
{
    for(size_t i = 0; i < size; i++)
        console_putchar(data[i]);
}

void console_writestring(const char* data)
{
    console_write(data, strlen(data));
}

void kernel_main(multiboot_info_t *mbi)
{
    fb           = (char*)(uint32_t)mbi->framebuffer_addr;
    scanline     = mbi->framebuffer_pitch;
    screen_width  = mbi->framebuffer_width;
    screen_height = mbi->framebuffer_height;

    console_init();
    console_writestring("Testing!");
}