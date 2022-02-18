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
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include <ia32/ports.h>
#include <pc/int86.h>
#include <diegos/devices.h>
#include <errno.h>
#include <endian.h>

#include "vesa.h"
#include "vesa_private.h"
#include "vga_fonts.h"

/* Frame buffer pointers */
static volatile uint8_t *frame_buffer = NULL;
static volatile uint32_t *frame_buffer32 = NULL;
/* Font pointer */
//static const unsigned char *g_font = NULL;
static struct VBEInfoBlock vesa_info;
static struct ModeInfoBlock vesa_gmode;
static uint16_t *vesa_mode_list = NULL;
static const uint8_t *g_font = NULL;
static unsigned W, H, max_offset;

static inline void swap(int *a, int *b)
{
	int c = *a;
	*a = *b;
	*b = c;
}

static inline unsigned compute_offset(unsigned x, unsigned y)
{
	return y * vesa_gmode.LinBytesPerScanLine + x;
}

static inline unsigned compute_len(int a, int b)
{
	if (a > b)
		return (unsigned)(a - b + 1);
	else
		return (b - a + 1);
}

static int vesa_init(unsigned unitno)
{
	regs16_t regs;
	uint16_t *ptr;
	unsigned modecount = 0;

	if (unitno) {
		return ENXIO;
	}

	strncpy(vesa_info.signature, "VBE2", sizeof(vesa_info.signature));
	regs.ax = 0x4F00;
	prot_to_seg_ofs(&vesa_info, &regs.es, &regs.di);
	int86(0x10, &regs);

	if (0x004F != regs.ax) {
		return ENXIO;
	}

	kdrvprintf("VESA Version %d.%d Video memory %u MBytes\n",
		   vesa_info.version >> 8, vesa_info.version & 0xFF, vesa_info.videomem64k >> 4);
	kdrvprintf("OEM: %s - Vendor: %s - Product: %s\n",
		   real_to_prot(vesa_info.OEMnameptr),
		   real_to_prot(vesa_info.vendornameptr), real_to_prot(vesa_info.productnameptr));
	kdrvprintf("Capabilities: %x\n", vesa_info.capabilities);

	if ((vesa_info.version >> 8) < 3)
		return ENOTSUP;
	ptr = real_to_prot(vesa_info.VESAmodesptr);
	while (*ptr++ != 0xFFFFU) {
		modecount++;
	}

	if (modecount) {
		modecount++;
		vesa_mode_list = malloc(modecount * sizeof(uint16_t));
		if (!vesa_mode_list)
			return ENXIO;
		ptr = real_to_prot(vesa_info.VESAmodesptr);
		memcpy(vesa_mode_list, ptr, modecount * sizeof(uint16_t));
	}
	return (EOK);
}

static void put_pixel(point_t pix, unsigned c)
{
	unsigned off = compute_offset(pix.x, pix.y);

	if (off < max_offset)
		frame_buffer[off] = (uint8_t) c;
}

static unsigned horiz_line(unsigned offset, unsigned len, unsigned c)
{
	c &= 0xFF;
	c |= c << 8;
	c |= c << 16;

	while ((offset & 3) && (len > 0)) {
		frame_buffer[offset++] = (uint8_t) c;
		--len;
	}
	offset >>= 2;
	while (len > 4) {
		frame_buffer32[offset] = c;
		len -= 4;
		offset++;
	}
	offset <<= 2;
	while (len--) {
		frame_buffer[offset++] = (uint8_t) c;
	}

	return offset;
}

static unsigned vert_line(unsigned offset, unsigned len, unsigned c)
{
	while (len--) {
		frame_buffer[offset] = (uint8_t) c;
		offset += vesa_gmode.LinBytesPerScanLine;
	}

	return offset;
}

static unsigned diag_line(unsigned offset, unsigned len, unsigned c, int adv)
{
	unsigned sum = vesa_gmode.LinBytesPerScanLine;

	if (adv > 0) {
		sum++;
	} else {
		sum--;
	}

	while (len--) {
		frame_buffer[offset] = (uint8_t) c;
		offset += sum;
	}

	return offset;
}

static void line(point_t a, point_t b, unsigned width, unsigned color)
{
	unsigned offset;
	int advancex, deltax, deltay;
	int wholestep, adjup, adjdown;
	int errorterm, inipixcount, finpixcount;
	int runlen;
	unsigned cnt;

	if (a.y > b.y) {
		swap(&a.y, &b.y);
		swap(&a.x, &b.x);
	}

	/* first pixel in memory */
	offset = compute_offset(a.x, a.y);

	deltax = b.x - a.x;

	if (deltax < 0) {
		advancex = -1;
		deltax = -deltax;
	} else {
		advancex = 1;
	}

	deltay = b.y - a.y;

	if (!deltax) {
		/* vertical line */
		vert_line(offset, deltay + 1, color);
		return;
	}

	if (!deltay) {
		/* horizontal line */
		horiz_line(offset, deltax + 1, color);
		return;
	}

	if (deltax == deltay) {
		/* diagonal line */
		diag_line(offset, deltax + 1, color, advancex);
	} else if (deltax < deltay) {
		/* Y major line */

		/* minimum # of pixels in a run */
		wholestep = deltay / deltax;

		adjup = (deltay % deltax) * 2;
		adjdown = deltax * 2;
		errorterm = (deltay % deltax) - adjdown;

		inipixcount = (wholestep / 2) + 1;
		finpixcount = inipixcount;

		if (!adjup && !(wholestep & 1)) {
			inipixcount--;
		}

		if (wholestep & 1) {
			errorterm += deltax;
		}

		/* draw the first partial run */
		offset = vert_line(offset, inipixcount, color);
		offset += advancex;

		/* draw all full runs */
		for (cnt = 0; cnt < (unsigned)(deltax - 1); cnt++) {
			runlen = wholestep;
			errorterm += adjup;

			if (errorterm > 0) {
				runlen++;
				errorterm -= adjdown;
			}

			offset = vert_line(offset, runlen, color);
			offset += advancex;
		}

		/* draw the final run of pixels */
		vert_line(offset, finpixcount, color);
		return;

	} else {
		/* X major */

		/* minimum # of pixels in a run */
		wholestep = deltax / deltay;

		adjup = (deltax % deltay) * 2;
		adjdown = deltay * 2;
		errorterm = (deltax % deltay) - adjdown;

		inipixcount = (wholestep / 2) + 1;
		finpixcount = inipixcount;

		if (!adjup && !(wholestep & 1)) {
			inipixcount--;
		}

		if (wholestep & 1) {
			errorterm += deltay;
		}

		/* draw the first partial run */
		if (advancex < 0) {
			offset -= inipixcount - 1;
			horiz_line(offset, inipixcount, color);
		} else {
			offset = horiz_line(offset, inipixcount, color);
		}
		offset += vesa_gmode.LinBytesPerScanLine + advancex;

		/* draw all full runs */
		for (cnt = 0; cnt < (uint32_t) (deltay - 1); cnt++) {
			runlen = wholestep;
			errorterm += adjup;

			if (errorterm > 0) {
				runlen++;
				errorterm -= adjdown;
			}

			if (advancex < 0) {
				offset -= runlen - 1;
				horiz_line(offset, runlen, color);
			} else {
				offset = horiz_line(offset, runlen, color);
			}
			offset += vesa_gmode.LinBytesPerScanLine + advancex;
		}

		/* draw the final run of pixels */
		if (advancex < 0)
			offset -= finpixcount - 1;
		horiz_line(offset, finpixcount, color);
		return;
	}
}

static void rect(point_t a, point_t b, unsigned wide, unsigned color)
{
	unsigned width = compute_len(a.x, b.x);
	unsigned height = compute_len(a.y, b.y);
	unsigned offseta = compute_offset(a.x, a.y);
	unsigned offsetd = compute_offset(a.x, b.y);
	unsigned offsetc = compute_offset(b.x, a.y);

	if (width < 2 * wide)
		return;
	if (height < 2 * wide)
		return;
	if (!wide)
		return;

	/*
	 * a --------------- c
	 * |                             | 
	 * |                 |
	 * d --------------- b
	 */

	while (wide--) {
		horiz_line(offseta, width, color);
		horiz_line(offsetd, width, color);
		vert_line(offseta, height, color);
		vert_line(offsetc, height, color);
		width -= 2;
		height -= 2;
		offseta += vesa_gmode.LinBytesPerScanLine + 1;
		offsetd -= vesa_gmode.LinBytesPerScanLine - 1;
		offsetc += vesa_gmode.LinBytesPerScanLine - 1;
	}
}

static void fillrect(point_t a, point_t b, unsigned c)
{
	unsigned width = compute_len(a.x, b.x);
	unsigned height = compute_len(a.y, b.y);
	unsigned offset = compute_offset(a.x, a.y);

	while (height--) {
		horiz_line(offset, width, c);
		offset += vesa_gmode.LinBytesPerScanLine;
	}
}

/*
 * It is implicitely assumed that fonts are rendered real-time as well as H,
 * the font width, has a fixed value of 8
 * */
static void vesa_write_text(point_t a, unsigned c, const char *str)
{
	const unsigned char *fontptr;
	volatile uint8_t *fb = frame_buffer;
	unsigned loop, scan, i = 0;
	unsigned char cursor;

	if (!str || !str[0])
		return;

	while (str[i]) {
		fb = frame_buffer + compute_offset(a.x, a.y);
		fontptr = g_font + (unsigned)str[i] * H;
		for (scan = 0; scan < H; scan++, fb += vesa_gmode.XResolution) {
			cursor = 1;
			loop = W;
			while (cursor) {
				loop--;
				if (cursor & fontptr[scan])
					fb[loop] = (uint8_t) c;
				cursor += cursor;
			}
		}
		i++;
		a.x += W;
	}
}

static void vesa_write_bgtext(point_t a, unsigned bg, unsigned c, const char *str)
{
	const unsigned char *fontptr;
	volatile uint8_t *fb = frame_buffer;
	unsigned loop, scan, i = 0;
	unsigned char cursor;

	if (!str || !str[0])
		return;

	while (str[i]) {
		fb = frame_buffer + compute_offset(a.x, a.y);
		fontptr = g_font + (unsigned)str[i] * H;
		for (scan = 0; scan < H; scan++, fb += vesa_gmode.XResolution) {
			cursor = 1;
			loop = W;
			while (cursor) {
				loop--;
				if (cursor & fontptr[scan])
					fb[loop] = (uint8_t) c;
				else
					fb[loop] = (uint8_t) bg;
				cursor += cursor;
			}
		}
		i++;
		a.x += W;
	}
}

static void load_palette(unsigned index, uint8_t rgb[])
{
#if 0
	regs16_t regs;
	uint8_t *ptr = (uint8_t *) real_buffer();

	if (index > 255)
		return;

	regs.ax = 0x4F09;
	regs.bx = 2;
	regs.cx = 1;
	regs.dx = (uint16_t) index;
	ptr[0] = rgb[0];
	ptr[1] = rgb[1];
	ptr[2] = rgb[2];
	ptr[3] = 0;
	prot_to_seg_ofs(ptr, &regs.es, &regs.di);
	int86(0x10, &regs);
#else
	out_byte(0x3c8, index + 8);
	out_byte(0x3c9, rgb[0]);
	out_byte(0x3c9, rgb[1]);
	out_byte(0x3c9, rgb[2]);
#endif
}

static void load_all_palette(uint8_t rgb[])
{
	regs16_t regs;
	uint8_t *ptr = (uint8_t *) real_buffer();

	regs.ax = 0x4F09;
	regs.bx = 0;
	regs.cx = 256;
	regs.dx = 0;
	memcpy(ptr, rgb, 256 * 3);
	prot_to_seg_ofs(ptr, &regs.es, &regs.di);
	int86(0x10, &regs);
}

static void store_palette(unsigned index, uint8_t rgb[])
{
}

static void store_all_palette(uint8_t rgb[])
{
}

static void draw_sprite(point_t a, uint8_t sprite[], unsigned W, unsigned H, unsigned color)
{
	volatile uint8_t *fb = frame_buffer;
	unsigned loop = 0, scan;
	unsigned char cursor;

	if (!W || !H)
		return;

	fb = frame_buffer + compute_offset(a.x, a.y);
	while (H--) {
		for (scan = W, cursor = 1; scan > 8; scan -= 8, loop++, cursor = 1) {
			while (cursor) {
				if (cursor & sprite[loop])
					*fb = (uint8_t) color;
				++fb;
				cursor += cursor;
			}
		}
		if (scan) {
			cursor = 1;
			while (scan--) {
				if (cursor & sprite[loop])
					*fb = (uint8_t) color;
				++fb;
				cursor += cursor;
			}
			loop++;
		}
		fb += vesa_gmode.XResolution - W;
	}
}

static void show(void)
{
	/* 
	 * No op
	 */
}

static void set_font(uint8_t fonts[], unsigned w, unsigned h)
{
	/* Right now we don't use fonts[], next dev round please :-) */
	if ((8 == w) && (8 == h)) {
		g_font = vga_get_font(VGA_FONTS_CP437_8x8);
		W = w;
		H = h;
	} else if ((8 == w) && (16 == h)) {
		g_font = vga_get_font(VGA_FONTS_CP437_8x16);
		W = w;
		H = h;
	} else {
		g_font = vga_get_font(VGA_FONTS_CP437_8x16);
		W = 8;
		H = 16;
	}
}

static unsigned vesa_status(unsigned unitno)
{
	return (DRV_STATUS_RUN | DRV_IS_GFX_UI);
}

static int set_resolution(unsigned W, unsigned H, unsigned depth, int loose)
{
	uint16_t *cursor = vesa_mode_list;
	uint16_t mode;
	regs16_t regs;

	if (depth != 8)
		return ENXIO;

	while (*cursor != 0xFFFF) {
		mode = *cursor++ & 0x1FF;
		mode |= (1 << 14);
		regs.ax = 0x4F01;
		/* Request for linear frame buffer support */
		regs.cx = mode;
		prot_to_seg_ofs(&vesa_gmode, &regs.es, &regs.di);
		int86(0x10, &regs);
		if (regs.ax != 0x004F)
			continue;
		if (0x81 != (vesa_gmode.ModeAttributes & 0x81))
			continue;
		if (vesa_gmode.BitsPerPixel != depth)
			continue;
		if (vesa_gmode.MemoryModel != 0x04)
			continue;
		if ((vesa_gmode.XResolution != W) || (vesa_gmode.YResolution != H))
			continue;
		/* 
		 * OK go for it! 
		 */
		regs.ax = 0x4F02;
		regs.bx = mode;
		int86(0x10, &regs);
		if (regs.ax != 0x004F)
			return ENOTSUP;
		frame_buffer = (uint8_t *) vesa_gmode.PhysAddress;
		frame_buffer32 = (uint32_t *) vesa_gmode.PhysAddress;
		max_offset = vesa_gmode.LinBytesPerScanLine * vesa_gmode.YResolution;
		/*
		 * DAC can be set to 8 bit per pixel
		 */
		//  if (vesa_info.capabilities & 1)
		//  {
		//      regs.ax = 0x4F08;
		//      regs.bx = 0x0800;
		//      int86(0x10, &regs);
		//      assert((regs.bx & 0xFF00) == 0x0800);
		//}
		return EOK;
	}
	return ENOTSUP;
}

/*
 * This has been placed at the end so we do not need to
 * forward static declarations etc. etc.
 * Moreover, it is an initialization, no more than this...
 */
grafics_driver_t vesa_drv = {
	.cmn = {
		.name = "vesa",
		.init_fn = vesa_init,
		.start_fn = driver_def_ok,
		.stop_fn = driver_def_ok,
		.done_fn = driver_def_ok,
		.status_fn = vesa_status,
		.ioctrl_fn = NULL,
		.poll_fn = NULL}
	,
	.put_pixel_fn = put_pixel,
	.line_fn = line,
	.rectangle_fn = rect,
	.filled_rectangle_fn = fillrect,
	.write_text_fn = vesa_write_text,
	.write_text_bg_fn = vesa_write_bgtext,
	.load_palette_fn = load_palette,
	.load_all_palette_fn = load_all_palette,
	.store_palette_fn = store_palette,
	.store_all_palette_fn = store_all_palette,
	.draw_sprite_fn = draw_sprite,
	.show_fn = show,
	.set_font_fn = set_font,
	.set_res_fn = set_resolution
};
