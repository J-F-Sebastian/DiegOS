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
#include <string.h>
#include <assert.h>
#include <ia32/ports.h>
#include <diegos/devices.h>
#include <errno.h>

#include "vga_tty.h"

/*
 * Implementation in vga_tty_bcopy.s, no header file for one private function
 */
extern void vga_tty_bcopy(const uint16_t * src, uint16_t * dst, unsigned words);

/*
 * Implementation in vga_tty_bset.s, no header file for one private function
 */
extern void vga_tty_bset(uint16_t * dst, uint16_t val, unsigned words);

#define X_MAX (80)
#define Y_MAX (25)

/*
 * VGA Video memory is word sized (16 bit wide)
 */
#define VRAM_SIZEW (X_MAX * Y_MAX)
#define VRAM_SIZEB (X_MAX * Y_MAX * 2)

/*
 * Some ports
 */
#define AC_REG_W            (0x3C0)
#define AC_REG_R            (0x3C1)
#define MISC_OUT_REG_R      (0x3CC)
#define MISC_OUT_REG_W      (0x3C2)
#define INPUT_STAT_REG_R    (0x3DA)
#define CRT_CTRL_REG_IDX    (0x3D4)
#define CRT_CTRL_REG_RW     (0x3D5)

#define CRT_START_ADDR_HI_IDX (0xC)
#define CRT_START_ADDR_LO_IDX (0xD)
#define CRT_CURSOR_LOC_HI_IDX (0xE)
#define CRT_CURSOR_LOC_LO_IDX (0xF)

/*
 * VGA Video buffer is 32kbyte wide; it is split into
 * 8 segments, 4kbytes wide, each of which can address
 * 4000 bytes (that is, 80*25*2 bytes).
 * For computational and H/W requirements only the most
 * significant byte of the buffer address is explicitely
 * used.
 */
#define VRAM_BEGIN 	(0xB8000UL)
#define VRAM_FRAME_SIZE (0x1000UL)
#define VRAM_FRAME_NUM  (8)

/*
 * frames are 0 based.
 */
#define VRAM_ADDR(frame) (VRAM_BEGIN + (frame%VRAM_FRAME_NUM)*VRAM_FRAME_SIZE)

/*
 * Default attribute
 */
#define DEFAULT_COLOR (0x0F)

static const char first_char_visible = 0x20;
static const char fill_char = ' ';

/*
 * NOTE: every character on video is defined by a short
 *
 * 0.1.2.3.4.5.6.7.8.9.A.B.C.D.E.F
 *                |
 *    ASCII CODE  |    COLOR CODE
 * ===============================
 */

/* Frame counter */
static unsigned frame_no = 0;
/* Current character position (in words) in buffer */
static unsigned cur_pos = 0;
/* In-memory buffer */
static uint16_t *buffer;
/* Scrolling indicator */
static BOOL scrolling = FALSE;

static unsigned status = DRV_IS_CHAR;

/*
 * Update the cursor position
 */
static void update_cursor_position(void)
{
	uint16_t cursor_pos = ((frame_no % VRAM_FRAME_NUM) * VRAM_FRAME_SIZE >> 1);

	if (scrolling) {
		cursor_pos += X_MAX * (Y_MAX - 1) + cur_pos % X_MAX;
	} else {
		cursor_pos += cur_pos;
	}

	/* Update cursor position */
	out_byte(CRT_CTRL_REG_IDX, CRT_CURSOR_LOC_HI_IDX);
	out_byte(CRT_CTRL_REG_RW, cursor_pos >> 8);
	out_byte(CRT_CTRL_REG_IDX, CRT_CURSOR_LOC_LO_IDX);
	out_byte(CRT_CTRL_REG_RW, cursor_pos & 0xFF);
}

/*
 * Initialize buffers content
 */
static void init_memory_buffers(void)
{
	const uint32_t clearchar = ((0x07 << 8) | first_char_visible);
	uint16_t *vram = 0;
	unsigned i = 0;

	/*
	 * Will clear with spaces both memory buffer and
	 * video RAM. Attribute is 0 as foreground does not matter
	 * (blank character !!!) and background is black.
	 */
	vga_tty_bset(buffer, clearchar, VRAM_SIZEW);

	while (i < VRAM_FRAME_NUM) {
		vram = (uint16_t *) VRAM_ADDR(i);
		vga_tty_bset(vram, clearchar, VRAM_SIZEW);
		i++;
	}
}

/*
 * Updates frame number, used to switch hw memory buffer
 */
static inline void update_frame(void)
{
	frame_no++;
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
	do {
		regval = in_byte(INPUT_STAT_REG_R);
	} while (regval & 8);
	/*
	 * Wait for vertical retrace end
	 */
	do {
		regval = in_byte(INPUT_STAT_REG_R);
	} while (~regval & 8);

	/*
	 * Update start address of regenerative buffer,
	 * i.e. point to a different position in VRAM.
	 * Text modes use 2 planes
	 * so pointing to the next 4KByte buffer requires
	 * writing a combined high + low address as multiple
	 * of 0x800 and not 0x1000 (or 2048 and not 4096 in decimals )...
	 */
	out_byte(CRT_CTRL_REG_IDX, CRT_START_ADDR_HI_IDX);
	regval = (frame_no % VRAM_FRAME_NUM) * 8;
	out_byte(CRT_CTRL_REG_RW, regval);
	out_byte(CRT_CTRL_REG_IDX, CRT_START_ADDR_LO_IDX);
	out_byte(CRT_CTRL_REG_RW, 0);
}

/*
 * Copy the in-memory buffer to hw frame buffer
 */
static void refresh_vram(void)
{
	uint16_t *dest = (uint16_t *) VRAM_ADDR(frame_no);
	unsigned startofs;

	if (scrolling) {
		startofs = (1 + cur_pos / X_MAX) * X_MAX;
		/* We are wrapping the buffer, split the copy */
		vga_tty_bcopy(buffer + startofs, dest, VRAM_SIZEW - startofs);
		vga_tty_bcopy(buffer, dest + VRAM_SIZEW - startofs, startofs);
	} else {
		vga_tty_bcopy(buffer, dest, VRAM_SIZEW);
	}
}

/*
 * Fill a single row with blanks
 */
static void newline(void)
{
	const uint16_t clearchar = ((0x07 << 8) | fill_char);

	if (cur_pos == VRAM_SIZEW) {
		cur_pos = 0;
		scrolling = TRUE;
	}

	vga_tty_bset(buffer + cur_pos, clearchar, X_MAX);

	update_frame();
	refresh_vram();
	update_vram_start();
	update_cursor_position();
}

/*
 * Fill blanks and update video accordingly
 */
static void fill_blanks(unsigned num)
{
	while (num--) {
		buffer[cur_pos++] = (DEFAULT_COLOR << 8) | fill_char;
		if ((cur_pos % X_MAX) == 0)
			newline();
	}
}

static int tty_init(unsigned unitno)
{
	uint8_t regval;

	if (unitno) {
		return (ENXIO);
	}

	/*
	 * No blinking
	 */
#if 0
	regval = in_byte(INPUT_STAT_REG_R);
	regval = 0x10;
	out_byte(AC_REG_W, regval);
	regval = in_byte(AC_REG_R);
	regval &= ~0x18;
	out_byte(AC_REG_W, regval);
#endif
	buffer = calloc(1, VRAM_SIZEB);
	if (!buffer)
		return ENOMEM;
	/*
	 * Enable ? as D in port addressing, set compatibility
	 * with color displays
	 */
	regval = in_byte(MISC_OUT_REG_R);
	if (~regval & 1) {
		regval |= 1;
		regval &= ~0x30;
		out_byte(MISC_OUT_REG_W, regval);
	}

	/* clear video memory */
	init_memory_buffers();

	/* Cursor upper left */
	update_cursor_position();

	return (EOK);
}

static int tty_done(unsigned unitno)
{
	return (EOK);
}

static int tty_write(const void *buf, unsigned bytes, unsigned unitno)
{
	char *localbuf = (char *)buf;
	int retval = bytes;

	if (unitno) {
		return ENXIO;
	}

	while (bytes) {
		/*
		 * All unmanaged ctrl chars are skipped, no output no evidence of
		 * their existance...
		 */
		if (*localbuf < first_char_visible) {
			switch (*localbuf) {
				/* backspace */
			case 0x8:
				buffer[--cur_pos] = (DEFAULT_COLOR << 8) | fill_char;
				break;
				/* horizontal tab, 4 chars per tab */
			case 0x9:
				fill_blanks(4);
				break;
				/* new line */
			case 0xA:
				/*
				 * This is the default behavior on unix
				 */
				fill_blanks(X_MAX - cur_pos % X_MAX);
				break;
				/* vertical tab, 2 lines */
			case 0xB:
				fill_blanks(X_MAX * 2);
				break;
				/* carriage return */
			case 0xC:
				cur_pos -= cur_pos % X_MAX;
				break;
			}
		} else {
			buffer[cur_pos++] = (DEFAULT_COLOR << 8) | (*localbuf);
			if ((cur_pos % X_MAX) == 0)
				newline();
		}
		++localbuf;
		--bytes;
	}

	update_frame();
	refresh_vram();
	update_vram_start();
	update_cursor_position();

	return (retval);
}

static unsigned tty_status(unsigned unitno)
{
	if (unitno) {
		return (0);
	}
	return (status);
}

/*
 * This has been placed at the end so we do not need to
 * forward static declarations etc. etc.
 * Moreover, it is an initialization, no more than this...
 */
char_driver_t vga_tty_drv = {
	.cmn = {
		.name = "vgtty",
		.init_fn = tty_init,
		.start_fn = driver_def_ok,
		.stop_fn = driver_def_ok,
		.done_fn = tty_done,
		.ioctrl_fn = NULL,
		.status_fn = tty_status,
		.poll_fn = NULL}
	,
	.write_fn = tty_write,
	.read_fn = NULL,
	.write_multi_fn = NULL,
	.read_multi_fn = NULL
};
