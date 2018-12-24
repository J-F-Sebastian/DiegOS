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
#include <endian.h>

#include "vga_private.h"
#include "vga_palettes.h"
#include "vga_fonts.h"
#include "vga.h"

#define VGA_NUM_SEQ_REGS    (5)
#define VGA_NUM_CRTC_REGS   (25)
#define VGA_NUM_GC_REGS     (9)
#define VGA_NUM_AC_REGS     (21)
#define VGA_NUM_DAC_PALS    (16)
#define VGA_NUM_REGS        (1 + VGA_NUM_SEQ_REGS + \
                                 VGA_NUM_CRTC_REGS + \
                                 VGA_NUM_GC_REGS + \
                                 VGA_NUM_AC_REGS)

static unsigned char mode640x480x16[] = {
    /* MISC*/
    0xC3,
    /* SEQ */
    0x03, 0x03, 0x0F, 0x00, 0x06,
    /* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
    0x00, 0x40, 0x00, 0x00, 0x00, 0x00 ,0x00, 0x00,
    0xEA, 0x0C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x03, 0x02, 0x05, 0x00,
    0xFF,
    /* AC */
    /*0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,*/
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x01, 0x00, 0x0F, 0x00, 0x00
};

static volatile unsigned char  *vgabuf = (unsigned char *)VGA_VIDEO_BUFFER1;
static const unsigned char *g_font = NULL;
static unsigned W,H;

static void write_regs (unsigned char *regs)
{
    unsigned i;
    const palette_t *pal = vga_get_palette(VGA_PAL_VGA);
    uint8_t r,g,b;

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

    for (i = 0; i < VGA_NUM_DAC_PALS; i++) {
		/*
		 * VGA can use 6 bits per channel
		 */ 
		r = pal[i].red >> 2;
		g = pal[i].green >> 2;
		b = pal[i].blue >> 2;

		/*
		 * Rounding
		 */
		if ((pal[i].red & 3) > 1 ) r++; 
		if ((pal[i].green & 3) > 1 ) g++;
		if ((pal[i].blue & 3) > 1 ) b++;
		
        out_byte(VGA_DAC_WRITE_INDEX, i);
        out_byte(VGA_DAC_DATA,r);
        out_byte(VGA_DAC_DATA,g);
        out_byte(VGA_DAC_DATA,b);
    }

    /* lock 16-color palette and unblank display */
    (void)in_byte(VGA_INSTAT_READ);
    out_byte(VGA_AC_INDEX, 0x20);
}

static inline unsigned compute_offset (unsigned x, unsigned y)
{
    return ((y*80) + x/8);
}

static int vga_init (unsigned unitno)
{
    unsigned i = 0;

    if (unitno)
    {
        return ENXIO;
    }

    write_regs(mode640x480x16);

    /* Set write function select to NOT MODIFY */
    out_byte(VGA_GC_INDEX, 3);
    out_byte(VGA_GC_DATA, 0x00);
    /* Set bitmask for display memory */
    out_byte(VGA_GC_INDEX, 8);
    out_byte(VGA_GC_DATA, 0xFF);
    /*
     * Do not use memcpy as the mapped device has a byte-array buffer
     */
    while (i < 640*480/8) {
        vgabuf[i++] = 0x00;
    }

    g_font = vga_get_font(VGA_FONTS_CP437_8x16);
    W = 8;
    H = 16;

    return (EOK);
}

static void vga_put_pixel_wm2 (point_t pix, unsigned c)
{
    unsigned off, mask;

    off = compute_offset(pix.x, pix.y);
    pix.x %= 8;
    mask = 0x80 >> pix.x;

    /* Set write function select to NOT MODIFY */
    out_byte(VGA_GC_INDEX, 3);
    out_byte(VGA_GC_DATA, 0x00);
    /* Set bitmask for display memory */
    out_byte(VGA_GC_INDEX, 8);
    out_byte(VGA_GC_DATA, mask);
    mask = vgabuf[off];
    vgabuf[off] = (uint8_t)c;
}

static void horiz_pixel_wm2(point_t a, unsigned len, unsigned c)
{
    unsigned offset[2];
    unsigned char valb, valb1;

    if (!len) return;

    offset[0] = compute_offset(a.x, a.y);
    offset[1] = compute_offset(a.x+len-1, a.y);
    /* Set write function select to NOT MODIFY */
    out_byte(VGA_GC_INDEX, 3);
    out_byte(VGA_GC_DATA, 0x00);

    if (offset[0] == offset[1]) {
        valb = ~(0xFF >> len);
        valb >>= a.x%8;
        /* Set bitmask for display memory */
        out_byte(VGA_GC_INDEX, 8);
        out_byte(VGA_GC_DATA, valb);
        valb1 = vgabuf[offset[0]];
        vgabuf[offset[0]] = (uint8_t)c;
    } else if (offset[1] - offset[0] == 1) {
        valb = (0xFF >> a.x%8);
        /* Set bitmask for display memory */
        out_byte(VGA_GC_INDEX, 8);
        out_byte(VGA_GC_DATA, valb);
        valb = vgabuf[offset[0]];
        vgabuf[offset[0]] = (uint8_t)c;
        valb1 = ~(0x7F >> (a.x + len -1)%8);
        out_byte(VGA_GC_DATA, valb1);
        valb1 = vgabuf[offset[1]];
        vgabuf[offset[1]] = (uint8_t)c;
    } else {
        /*
         * Starting and ending bytes
         */
        valb = (0xFF >> a.x%8);
        /* Set bitmask for display memory */
        out_byte(VGA_GC_INDEX, 8);
        out_byte(VGA_GC_DATA, valb);
        valb = vgabuf[offset[0]];
        vgabuf[offset[0]] = (uint8_t)c;
        valb1 = ~(0x7F >> (a.x + len -1)%8);
        out_byte(VGA_GC_DATA, valb1);
        valb1 = vgabuf[offset[1]];
        vgabuf[offset[1]] = (uint8_t)c;
        offset[0]++;
        out_byte(VGA_GC_DATA, 0xFF);
        while (offset[0] < offset[1]) {
            vgabuf[offset[0]++] = (uint8_t)c;
        }
    }
}

static void vert_pixel_wm2(point_t a, unsigned len, unsigned c)
{
    unsigned loop;
    unsigned offset[2];
    unsigned char valb;

    if (!len) return;

    offset[0] = compute_offset(a.x, a.y);
    offset[1] = offset[0]+len*80;
    valb = (0x80 >> a.x%8);

    /* Set write function select to NOT MODIFY */
    out_byte(VGA_GC_INDEX, 3);
    out_byte(VGA_GC_DATA, 0x00);
    /* Set bitmask for display memory */
    out_byte(VGA_GC_INDEX, 8);
    out_byte(VGA_GC_DATA, valb);
    for (loop = offset[0]; loop < offset[1]; loop += 80) {
        valb = vgabuf[loop];
        vgabuf[loop] = (uint8_t)c;
    }
}

static void vga_line_wm2 (point_t a, point_t b, unsigned width, unsigned color)
{
    if (a.y == b.y) {
        horiz_pixel_wm2(a, b.x-a.x+1, color);
    } else if (a.x == b.x) {
        vert_pixel_wm2(a, b.y-a.y+1, color);
    }
}

static void vga_rect_wm2 (point_t a, point_t b, unsigned wide, unsigned color)
{
    unsigned loop;
    point_t c,d;
    unsigned width = b.x-a.x+1;

    if (b.x - a.x < (int)(2*wide)) return;
    if (b.y - a.y < (int)(2*wide)) return;
    if (!wide) return;

    c.y = a.y;
    c.x = b.x;
    d.y = b.y;
    d.x = a.x;

    if (1 == wide) {
        horiz_pixel_wm2(a, width, color);
        horiz_pixel_wm2(d, width, color);
        vert_pixel_wm2(a, b.y-a.y+1, color);
        vert_pixel_wm2(c, b.y-a.y+1, color);
    } else {
        loop = wide;
        while (loop--) {
            horiz_pixel_wm2(a, width, color);
            horiz_pixel_wm2(d, width, color);
            a.y++;
            d.y--;
        }
        loop = d.y - a.y +1;
        c.x -= wide -1;
        c.y += wide;
        while (loop--) {
            horiz_pixel_wm2(a,wide,color);
            horiz_pixel_wm2(c,wide,color);
            a.y++;
            c.y++;
        }
    }
}

static void vga_fillrect_wm2 (point_t a, point_t b, unsigned c)
{
    unsigned width = b.x-a.x+1;

    while (a.y <= b.y) {
        horiz_pixel_wm2(a,width,c);
        a.y++;
    }
}

static void vga_write_text_wm2 (point_t a, unsigned c, const char *str)
{
    unsigned loop, i = 0;
    unsigned offset;
    unsigned char valb1, valb2;
    const unsigned char *fontptr;

    if (!str || !str[0]) return;

    /* Set write function select to NOT MODIFY */
    out_byte(VGA_GC_INDEX, 3);
    out_byte(VGA_GC_DATA, 0x00);
    offset = compute_offset(a.x, a.y);

    out_byte(VGA_GC_INDEX, 8);
    if (a.x%8) {
        while (str[i]) {
            fontptr = &g_font[((unsigned char)str[i])*H];
            for (loop = 0; loop < H; loop++) {
                valb1 = *fontptr;
                valb2 = *fontptr;
                valb1 >>= (a.x%8);
                valb2 <<= (8-(a.x%8));
                /* Set bitmask for display memory */
                out_byte(VGA_GC_DATA, valb1);
                valb1 = vgabuf[offset];
                vgabuf[offset] = (uint8_t)c;
                /* Set bitmask for display memory */
                out_byte(VGA_GC_DATA, valb2);
                valb2 = vgabuf[offset+1];
                vgabuf[offset+1] = (uint8_t)c;
                offset += 80;
                fontptr++;
            }
            offset -= (80*H - 1);
            i++;
        }
    } else {
        while (str[i]) {
            fontptr = &g_font[((unsigned char)str[i])*H];
            for (loop = 0; loop < H; loop++) {
                valb1 = *fontptr++;
                /* Set bitmask for display memory */
                out_byte(VGA_GC_DATA, valb1);
                valb1 = vgabuf[offset];
                vgabuf[offset] = (uint8_t)c;
                offset += 80;
            }
            offset -= 80*H - 1;
            i++;
        }
    }
}

static void vga_write_bgtext_wm2 (point_t a,
                                  unsigned bg,
                                  unsigned c,
                                  const char *str)
{
    unsigned loop, i = 0;
    unsigned offset;
    unsigned char valb1, valb2, mask1, mask2;
    const unsigned char *fontptr;

    if (!str || !str[0]) return;

    /* Set write function select to NOT MODIFY */
    out_byte(VGA_GC_INDEX, 3);
    out_byte(VGA_GC_DATA, 0x00);
    offset = compute_offset(a.x, a.y);

    out_byte(VGA_GC_INDEX, 8);
    if (a.x%8) {
        mask1 = 0xFF >> (a.x%8);
        mask2 = ~mask1;
        while (str[i]) {
            fontptr = &g_font[((unsigned char)str[i])*H];
            for (loop = 0; loop < H; loop++) {
                valb1 = *fontptr;
                valb1 >>= (a.x%8);
                /* Set bitmask for display memory */
                out_byte(VGA_GC_DATA, valb1 & mask1);
                valb2 = vgabuf[offset];
                vgabuf[offset] = (uint8_t)c;
                out_byte(VGA_GC_DATA, (~valb1) & mask1);
                valb2 = vgabuf[offset];
                vgabuf[offset] = (uint8_t)bg;
                offset += 80;
                fontptr++;
            }
            offset -= (80*H - 1);
            fontptr -= (H);
            for (loop = 0; loop < H; loop++) {
                /* Set bitmask for display memory */
                valb2 = *fontptr;
                valb2 <<= (8-(a.x%8));
                out_byte(VGA_GC_DATA, valb2 & mask2);
                valb1 = vgabuf[offset];
                vgabuf[offset] = (uint8_t)c;
                out_byte(VGA_GC_DATA, (~valb2) & mask2);
                valb1 = vgabuf[offset];
                vgabuf[offset] = (uint8_t)bg;
                offset += 80;
                fontptr++;
            }
            offset -= (80*H);
            i++;
        }
    } else {
        /* Set bitmask to load latched display memory with background color*/
        out_byte(VGA_GC_DATA, 0xFF);
        vgabuf[offset] = (uint8_t)bg;
        valb1 = vgabuf[offset];

        while (str[i]) {
            fontptr = &g_font[((unsigned char)str[i])*H];
            for (loop = 0; loop < H; loop++) {
                valb1 = *fontptr++;
                /* Set bitmask for display memory */
                out_byte(VGA_GC_DATA, valb1);
                vgabuf[offset] = (uint8_t)c;
                offset += 80;
            }
            offset -= 80*H - 1;
            i++;
        }
    }
}

static void vga_load_palette (unsigned index, uint8_t rgb[])
{
    if (index < VGA_NUM_DAC_PALS) {
        out_byte(VGA_DAC_WRITE_INDEX, index+8);
        out_byte(VGA_DAC_DATA,rgb[0]);
        out_byte(VGA_DAC_DATA,rgb[1]);
        out_byte(VGA_DAC_DATA,rgb[2]);
    }
}

static void vga_load_all_palette (uint8_t rgb[])
{
    unsigned i;

    for (i = 0; i < VGA_NUM_DAC_PALS; i++) {
        out_byte(VGA_DAC_WRITE_INDEX, i+8);
        out_byte(VGA_DAC_DATA,rgb[i*3]);
        out_byte(VGA_DAC_DATA,rgb[i*3+1]);
        out_byte(VGA_DAC_DATA,rgb[i*3+2]);
    }
}

static void vga_store_palette (unsigned index, uint8_t rgb[])
{
    if (index < VGA_NUM_DAC_PALS) {
        out_byte(VGA_DAC_READ_INDEX, index+8);
        rgb[0] = in_byte(VGA_DAC_DATA);
        rgb[1] = in_byte(VGA_DAC_DATA);
        rgb[2] = in_byte(VGA_DAC_DATA);
    }
}

static void vga_store_all_palette (uint8_t rgb[])
{
    unsigned i;

    for (i = 0; i < VGA_NUM_DAC_PALS; i++) {
        out_byte(VGA_DAC_READ_INDEX, i+8);
        rgb[i*3] = in_byte(VGA_DAC_DATA);
        rgb[i*3+1] = in_byte(VGA_DAC_DATA);
        rgb[i*3+2] = in_byte(VGA_DAC_DATA);
    }
}

static void vga_draw_sprite(point_t a,
                            uint8_t sprite[],
                            unsigned W,
                            unsigned H,
                            unsigned color)
{
    unsigned off, mask/*, i = 0*/;
    unsigned *memref = (unsigned *)sprite;

    off = compute_offset(a.x, a.y);
    a.x %= 8;
    mask = SWAPL(*memref) >> a.x;

    /* Set write function select to NOT MODIFY */
    out_byte(VGA_GC_INDEX, 3);
    out_byte(VGA_GC_DATA, 0x00);
    /* Set bitmask for display memory */
    out_byte(VGA_GC_INDEX, 8);
    out_byte(VGA_GC_DATA, mask);
    mask = vgabuf[off];
    vgabuf[off] = (uint8_t)color;
}

static void vga_show (void)
{
    /* No op, VGA mode 640x480x16 cannot double buffer in video memory
     * and buffering in system memory is not efficient
     */
}

static void vga_set_font (uint8_t fonts[], unsigned w, unsigned h)
{
    /* Right now we don't use fonts[], next dev round please :-) */
    if ((8 == w) && (8 == h)) {
        g_font = vga_get_font(VGA_FONTS_CP437_8x8);
        W = w;
        H = h;
    }
    else if ((8 == w) && (16 == h)) {
        g_font = vga_get_font(VGA_FONTS_CP437_8x16);
        W = w;
        H = h;
    }
    else {
        g_font = vga_get_font(VGA_FONTS_CP437_8x16);
        W = 8;
        H = 16;
    }
}

static unsigned vga_status (unsigned unitno)
{
    return (DRV_STATUS_RUN | DRV_IS_GFX_UI);
}

static int vga_res (unsigned W, unsigned H, unsigned depth, int loose)
{
	return ENOTSUP;
}

/*
 * This has been placed at the end so we do not need to
 * forward static declarations etc. etc.
 * Moreover, it is an initialization, no more than this...
 */
grafics_driver_t vga_drv = {
    .cmn = {
        .name = "vga",
        .init_fn = vga_init,
        .start_fn = driver_def_ok,
        .stop_fn = driver_def_ok,
        .done_fn = driver_def_ok,
        .status_fn = vga_status,
        .ioctrl_fn = NULL,
        .poll_fn = NULL
    },
    .put_pixel_fn = vga_put_pixel_wm2,
    .line_fn = vga_line_wm2,
    .rectangle_fn = vga_rect_wm2,
    .filled_rectangle_fn = vga_fillrect_wm2,
    .write_text_fn = vga_write_text_wm2,
    .write_text_bg_fn = vga_write_bgtext_wm2,
    .load_palette_fn = vga_load_palette,
    .load_all_palette_fn = vga_load_all_palette,
    .store_palette_fn = vga_store_palette,
    .store_all_palette_fn = vga_store_all_palette,
    .draw_sprite_fn = vga_draw_sprite,
    .show_fn = vga_show,
    .set_font_fn = vga_set_font,
    .set_res_fn = vga_res
};

