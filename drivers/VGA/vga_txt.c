/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2015 Diego Gallizioli
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <ia32/ports.h>
#include <diegos/devices.h>
#include <errno.h>

#include "vga_private.h"
#include "vga_txt.h"
#include "vga_palettes.h" 
#include "vga_fonts.h"

#define X_MAX (80)
#define Y_MAX (25)

/*
 * VGA Video memory is word sized (16 bit wide)
 */
#define VRAM_SIZEW (X_MAX * Y_MAX)
#define VRAM_SIZEB (X_MAX * Y_MAX * 2)

#define CRT_START_ADDR_HI_IDX (0xC)
#define CRT_START_ADDR_LO_IDX (0xD)

/*
 * VGA Video buffer is 32kbyte wide; it is split into
 * 8 segments, 4kbytes wide, each of which can address
 * 4000 bytes (that is, 80*25*2 bytes).
 */
#define VRAM_FRAME_SIZE (0x1000UL)
#define VRAM_FRAME_NUM  (4)

#define VGA_NUM_SEQ_REGS    (5)
#define VGA_NUM_CRTC_REGS   (25)
#define VGA_NUM_GC_REGS     (9)
#define VGA_NUM_AC_REGS     (21)
#define VGA_NUM_DAC_PALS    (16)
#define VGA_NUM_REGS        (1 + VGA_NUM_SEQ_REGS + \
                                 VGA_NUM_CRTC_REGS + \
                                 VGA_NUM_GC_REGS + \
                                 VGA_NUM_AC_REGS)

static unsigned char mode_80x25_text[] = {
    /* MISC */
    0x67,
    /* SEQ */
    0x03, 0x00, 0x03, 0x00, 0x00,
    /* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
    0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
    0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
    0xFF,
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x0C, 0x00, 0x0F, 0x08, 0x00
};

#if 0
unsigned char mode_80x50_text[] = {
    /* MISC */
    0x67,
    /* SEQ */
    0x03, 0x00, 0x03, 0x00, /*0x02,*/ 0x00,
    /* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
    0x00, 0x47, 0x06, 0x07, 0x00, 0x00, 0x01, 0x40,
    0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
    0xFF,
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x0C, 0x00, 0x0F, 0x08, 0x00,
};
unsigned char mode_90x30_text[] = {
    /* MISC */
    0xE7,
    /* SEQ */
    0x03, 0x01, 0x03, 0x00, /*0x02,*/ 0x00,
    /* CRTC */
    0x6B, 0x59, 0x5A, 0x82, 0x60, 0x8D, 0x0B, 0x3E,
    0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00,
    0xEA, 0x0C, 0xDF, 0x2D, 0x10, 0xE8, 0x05, 0xA3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
    0xFF,
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x0C, 0x00, 0x0F, 0x08, 0x00,
};
#endif // 0

/* Frame counter */
static unsigned frame_no = 0;
/* Video Hardware Memory */
static volatile unsigned char  *txtbuf = (unsigned char *)VGA_VIDEO_BUFFER3;
/* Video Hardware Memory Cursor Position */
static point_t cursor_position = {0,0};

union txtdata {
    char data[2];
    uint16_t data16;
};

/******************************************************************************
 *				PRIVATE FUNCTIONS			      *
 ******************************************************************************/

/*
 * Update the cursor position
 */
static void update_cursor_position (void)
{
    uint16_t cursor_pos = ((frame_no%VRAM_FRAME_NUM)*VRAM_FRAME_SIZE>>1);
    cursor_pos += X_MAX*cursor_position.y+cursor_position.x;

    /* Update cursor position */
    out_byte(VGA_CRTC_INDEX, 0x0E);
    out_byte(VGA_CRTC_DATA, cursor_pos >> 8);
    out_byte(VGA_CRTC_INDEX, 0x0F);
    out_byte(VGA_CRTC_DATA, cursor_pos & 0xFF);
}
		 
/*
 * Update hw buffer for reading and writing
 */
static void update_video_buffer (void)
{
    ++frame_no;
    if (frame_no%VRAM_FRAME_NUM) {
        txtbuf += VRAM_FRAME_SIZE;
    } else {
        txtbuf = (unsigned char *)VGA_VIDEO_BUFFER3;
    }
}

/*
 * Switch to a new hw buffer
 */
static void update_vram_start(void)
{
    volatile uint8_t regval;

    /*
     * Wait for vertical retrace start
     */
    do 
    {
        regval = in_byte(VGA_INSTAT_READ);
    } 
    while (regval & 8);
    /*
     * Wait for vertical retrace end
     */
    do 
    {
        regval = in_byte(VGA_INSTAT_READ);
    } 
    while (~regval & 8);

    /*
     * Update start address of regenerative buffer,
     * i.e. point to a different position in VRAM.
     * Text modes use 2 planes
     * so pointing to the next 4KByte buffer requires
     * writing a combined high + low address as multiple
     * of 0x800 and not 0x1000 (or 2048 and not 4096 in decimals )...
     */
    out_byte(VGA_CRTC_INDEX, CRT_START_ADDR_HI_IDX);
    regval = (frame_no % VRAM_FRAME_NUM) * 8;
    out_byte(VGA_CRTC_DATA, regval);
    out_byte(VGA_CRTC_INDEX, CRT_START_ADDR_LO_IDX);
    out_byte(VGA_CRTC_DATA, 0);
}

static void write_regs (unsigned char *regs)
{
    unsigned i;

    /* write MISCELLANEOUS reg */
    out_byte(VGA_MISC_WRITE, *regs);
    regs++;

    /* write SEQUENCER regs */
    for(i = 0; i < VGA_NUM_SEQ_REGS; i++) {
        out_byte(VGA_SEQ_INDEX, i);
        out_byte(VGA_SEQ_DATA, *regs);
        regs++;
    }

    /* unlock CRTC registers */
    out_byte(VGA_CRTC_INDEX, 0x03);
    out_byte(VGA_CRTC_DATA, in_byte(VGA_CRTC_DATA) | 0x80);
    out_byte(VGA_CRTC_INDEX, 0x11);
    out_byte(VGA_CRTC_DATA, in_byte(VGA_CRTC_DATA) & ~0x80);

    /* make sure they remain unlocked */
    regs[0x03] |= 0x80;
    regs[0x11] &= ~0x80;

    /* write CRTC regs */
    for(i = 0; i < VGA_NUM_CRTC_REGS; i++) {
        out_byte(VGA_CRTC_INDEX, i);
        out_byte(VGA_CRTC_DATA, *regs);
        regs++;
    }

    /* write GRAPHICS CONTROLLER regs */
    for(i = 0; i < VGA_NUM_GC_REGS; i++) {
        out_byte(VGA_GC_INDEX, i);
        out_byte(VGA_GC_DATA, *regs);
        regs++;
    }

    (void)in_byte(VGA_INSTAT_READ);
    /* write ATTRIBUTE CONTROLLER regs */
    for(i = 0; i < VGA_NUM_AC_REGS; i++) {
        out_byte(VGA_AC_INDEX, i);
        out_byte(VGA_AC_WRITE, *regs);
        regs++;
    }

    /* lock 16-color palette and unblank display */
    (void)in_byte(VGA_INSTAT_READ);
    out_byte(VGA_AC_INDEX, 0x20);
}

static void write_font(const unsigned char *buf)
{
    unsigned char seq2, seq4, gc4, gc5, gc6;
    unsigned i,k;
    char *dest = (char *)(VGA_VIDEO_BUFFER1);

    /* save registers MAP MASK and MEMORY MODE */
    out_byte(VGA_SEQ_INDEX, 2);
    seq2 = in_byte(VGA_SEQ_DATA);
    out_byte(VGA_SEQ_INDEX, 4);
    seq4 = in_byte(VGA_SEQ_DATA);
    /* save registers MAP MASK, GRAPHICS MODE and MISC */
    out_byte(VGA_GC_INDEX, 4);
    gc4 = in_byte(VGA_GC_DATA);
    out_byte(VGA_GC_INDEX, 5);
    gc5 = in_byte(VGA_GC_DATA);
    out_byte(VGA_GC_INDEX, 6);
    gc6 = in_byte(VGA_GC_DATA);
    /* set write plane 2, i.e. memory BANK 2 */
    out_byte(VGA_SEQ_INDEX, 2);
    out_byte(VGA_SEQ_DATA, 0x4);
    /*
     * turn off even-odd addressing (set flat addressing)
     * and enable extended memory
     */
    out_byte(VGA_SEQ_INDEX, 4);
    out_byte(VGA_SEQ_DATA, 0x06);
    /* set read plae 2, i.e. memory BANK 2 */
    out_byte(VGA_GC_INDEX, 4);
    out_byte(VGA_GC_DATA, 0x02);
    /* turn off even-odd addressing (twice!) */
    out_byte(VGA_GC_INDEX, 5);
    out_byte(VGA_GC_DATA, gc5 & ~0x10);
    /* turn-off even-odd addressing and select MAPPING 1 */
    out_byte(VGA_GC_INDEX, 6);
    out_byte(VGA_GC_DATA, 0x04);
    /* write font 0 */
    k = i = 0;
    /* VGA Fonts are arrays of bytes with a fixed size, 32 bytes per font.
     * Most common resolutions are VGA 640x480 with font sizes of 8x16 pixels or 8x8 pixels, giving 80x25 or 80x50 text resolutions.
     * Right now only 80x25 is supported.
     */
    while (i < 32*256)
    {
	dest[i++] = buf[k++];
	if (0 == (i%16))
	{
	    dest += 16;
	}
    }
    /* restore registers */
    out_byte(VGA_SEQ_INDEX, 2);
    out_byte(VGA_SEQ_DATA, seq2);
    out_byte(VGA_SEQ_INDEX, 4);
    out_byte(VGA_SEQ_DATA, seq4);
    out_byte(VGA_GC_INDEX, 4);
    out_byte(VGA_GC_DATA, gc4);
    out_byte(VGA_GC_INDEX, 5);
    out_byte(VGA_GC_DATA, gc5);
    out_byte(VGA_GC_INDEX, 6);
    out_byte(VGA_GC_DATA, gc6);
}

/*
 * DRIVER FUNCTIONS
 */

/* Forward declaration, I fucking hate this ... */
static void set_palette_all (const palette_t *pal);

static int vga_init (unsigned unitno)
{
    uint16_t *hwmem = (uint16_t *)VGA_VIDEO_BUFFER3;
    unsigned i;

    if (unitno)
    {
	return ENXIO;
    }

    write_regs(mode_80x25_text);
    write_font(vga_get_font(VGA_FONTS_DEFAULT));
    set_palette_all(vga_get_palette(VGA_PAL_DEFAULT));
    for (i = 0; i < 0x10000; i++) *hwmem++ = 0;

    return (EOK);
}

static void put_char (point_t pos, char ch, uint8_t color)
{
    unsigned mempos = pos.y*80 + pos.x;
    uint16_t *temp = (uint16_t *)txtbuf;
    union txtdata pix;

    pix.data[0] = ch;
    pix.data[1] = color;
    temp[mempos] = pix.data16;
}

static void write_str (point_t a, point_t b, const char *ch, uint8_t color)
{
    uint16_t *temp = (uint16_t *)txtbuf;    
    unsigned c;
    unsigned toty = b.y - a.y + 1;
    unsigned totx = b.x - a.x + 1;
    union txtdata pix;

    pix.data[1] = color;
    temp += a.y*80+a.x;
    while (*ch && toty--)
    {
        c = totx;
        while (*ch && c--)
	{			
            pix.data[0] =*ch++;
            *temp++ = pix.data16;
	}
        temp += (80 - totx);        
    }
}

static void rect (point_t a, point_t b, unsigned width, char ch, uint8_t color)
{ 
	
}

static void fillrect (point_t a, point_t b, char ch, uint8_t color)
{    
    uint16_t *temp = (uint16_t *)txtbuf;
    uint16_t val = ch+color*256;
    unsigned c;
    unsigned toty = b.y - a.y + 1;
    unsigned totx = b.x - a.x + 1;
    
    temp += a.y*80+a.x;
    while (toty--)
    {
        c = totx;
        while (c--) *temp++ = val;
        temp += 80 - totx;        
    }
}

static void bitblt (point_t a, point_t b, void *dbuffer)
{    
    uint16_t *temp = (uint16_t *)txtbuf;
    uint16_t *dest = (uint16_t *)dbuffer;
    unsigned c, tot = b.y - a.y + 1;
    
    temp += a.y*80+a.x;
    while (tot--)
    {
        c = b.x - a.x + 1;
        while (c--) *dest++ = *temp++;
        temp += 80 - (b.x - a.x + 1);        
    }    
}

static void bitblt2 (point_t a, point_t b, const void *sbuffer)
{
    uint16_t *temp = (uint16_t *)txtbuf;
    uint16_t *src = (uint16_t *)sbuffer;
    unsigned c, tot = b.y - a.y + 1;
    
    temp += a.y*80+a.x;
    while (tot--)
    {
        c = b.x - a.x + 1;
        while (c--) *temp++ = *src++;
        temp += 80 - (b.x - a.x + 1);        
    }
}

static void set_cursor_position (point_t pos)
{
    uint16_t cursor_pos = ((frame_no%VRAM_FRAME_NUM)*VRAM_FRAME_SIZE>>1);
    cursor_pos += X_MAX*pos.y+pos.x;

    /* Update cursor position */
    out_byte(VGA_CRTC_INDEX, 0x0E);
    out_byte(VGA_CRTC_DATA, cursor_pos >> 8);
    out_byte(VGA_CRTC_INDEX, 0x0F);
    out_byte(VGA_CRTC_DATA, cursor_pos & 0xFF);
    cursor_position = pos;
}

static void cursor_visible (int on)
{
    uint8_t reg;
    
    out_byte(VGA_CRTC_INDEX, 0x0A);
    reg = in_byte(VGA_CRTC_DATA);
    
    if (on)
    {
 	reg &= ~0x20; 	
    }
    else
    {
 	reg |= 0x20;
    }

    out_byte(VGA_CRTC_DATA, reg);
}

static void cursor_size (unsigned startline, unsigned endline)
{
	uint8_t reg;
	
	out_byte(VGA_CRTC_INDEX, 0x0A);
	reg = in_byte(VGA_CRTC_DATA);
	reg &= 0xE0;
	reg |= startline & 0x1F;
	out_byte(VGA_CRTC_DATA, reg);

	/* 
	 * We assume cursor skew will never be used, unless someone in the world
	 * is still sticking to EGA
	 */
	out_byte(VGA_CRTC_INDEX, 0x0B);
	reg = endline & 0x1F;
	out_byte(VGA_CRTC_DATA, reg);
}

static void set_fonts (const unsigned char *fonts)
{
        write_font(fonts);
}

static void set_palette (unsigned index, palette_t pal)
{
    uint8_t r,g,b;
    
    /*
     * VGA can use 6 bits per channel
     */ 
    r = pal.red >> 2;
    g = pal.green >> 2;
    b = pal.blue >> 2;
    
    /*
     * Rounding
     */
    if ((pal.red & 3) > 1 ) r++; 
    if ((pal.green & 3) > 1 ) g++;
    if ((pal.blue & 3) > 1 ) b++;
    
    out_byte(VGA_DAC_WRITE_INDEX, index);
    out_byte(VGA_DAC_DATA, r);
    out_byte(VGA_DAC_DATA, g);
    out_byte(VGA_DAC_DATA, b);
}

static void set_palette_all (const palette_t *pal)
{ 
    unsigned index;
        
    for (index = 0; index < VGA_NUM_DAC_PALS; index++)
    {
        set_palette(index, pal[index]);    
    }   
}

static void show_video (void)
{
    update_vram_start();
	update_cursor_position();
    update_video_buffer();
}

static unsigned vga_status (unsigned unitno)
{
    return (DRV_STATUS_RUN | DRV_IS_TXT_UI);
}

/*
 * This has been placed at the end so we do not need to
 * forward static declarations etc. etc.
 * Moreover, it is an initialization, no more than this...
 */
text_driver_t vgatxt_drv = {
    .cmn = {
        .name = "vgtxt",
        .init_fn = vga_init,
        .start_fn = driver_def_ok,
        .stop_fn = driver_def_ok,
        .done_fn = driver_def_ok,
        .status_fn = vga_status,
        .ioctrl_fn = NULL,
        .poll_fn = NULL
    },
    .put_char_fn = put_char,
    .write_char_fn = write_str,
    .line_fn = 0,
    .rectangle_fn = rect,
    .filled_rectangle_fn = fillrect,    
    .bitblt_fn = bitblt,
    .bitblt2_fn = bitblt2,
    .cursor_fn = set_cursor_position,
    .cursor_on_fn = cursor_visible,
    .cursor_size_fn = cursor_size,
    .set_fonts_fn = set_fonts,
    .set_palette_fn = set_palette,
    .set_palette_fn2 = set_palette_all,
    .show_fn = show_video
};

