/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2025 Diego Gallizioli
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
#include <errno.h>
#include <types_common.h>

#include "scancodes.h"

/*
 * Keyboard controllers like Intel 8042 send a one byte scancode for a set of keys,
 * a 2-byte scancode (first byte is 0xE0) or multi-word scancode (first byte is 0xE0)
 * for functional keys (SHIFT, ALT...) or multimedia keys.
 * Released keys have the MSB set, i.e. their code is greater than 0x80, or negative.
 * Unfortunately the multi-byte scan codes are not dense, so they would waste a lot
 * of memory space.
 */

static const char CODEPAGE_437[] = {
	/*
	 * UNUSED, ESCAPE, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, -, =, BACKSPACE, TAB
	 */
	0x00, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x2D, 0x3D, 0x08,
	0x09,
	/*
	 * Q, W, E, R, T, Y, U, I, O, P, [, ], ENTER, LEFT CTRL, A, S
	 */
	0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4F, 0x50, 0x5B, 0x5D, 0x0D, 0x00, 0x41,
	0x53,
	/*
	 * D, F, G, H, J, K, L, ;, ', `, LEFT SHIFT, \, Z, X, C, V
	 */
	0x44, 0x46, 0x47, 0x48, 0x4A, 0x4B, 0x4C, 0x3B, 0x27, 0x60, 0x00, 0x5C, 0x5A, 0x58, 0x43,
	0x56,
	/*
	 * B, N, M, ,, ., /, RIGHT SHIFT, KEYPAD *, LEFT ALT, SPACE, CAPSLOCK, F1, F2, F3, F4, F5
	 */
	0x42, 0x4E, 0x4D, 0x2C, 0x2E, 0x2F, 0x00, 0x2A, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	/*
	 * F6, F7, F8, F9, F10, NumLock, ScrollLock, Keypad 7, Kp 8, Kp 9, Kp -, Kp 4, Kp 5,  Kp 6, Kp +
	 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x38, 0x39, 0x2D, 0x34, 0x35, 0x36, 0x2B,
	/*
	 * Kp 1, Kp 2, Kp 3, Kp 0, Kp ., UNUSED, UNUSED, UNUSED, F11, F12
	 */
	0x31, 0x32, 0x33, 0x30, 0x2E, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* MULTI BYTE ARE MISSING */

static const char *codepages[] = {
	CODEPAGE_437
};

enum KBDSTATUS {
	KBD_LSHIFT = (1 << 31),
	KBD_RSHIFT = (1 << 30),
	KBD_LCTRL = (1 << 27),
	KBD_RCTRL = (1 << 26),
	KBD_LALT = (1 << 23),
	KBD_RALT = (1 << 22),
	KBD_CAPSLOCK = (1 << 15),
	KBD_NUMLOCK = (1 << 14),
	KBD_SCRLOCK = (1 << 13)
};

static unsigned codepage = US;
static unsigned status = 0;

int select_keyboard_map(unsigned kbdmap)
{
	if (kbdmap > US) {
		codepage = kbdmap;
		return (EOK);
	}

	return EINVAL;
}

char scancode_to_char(char input)
{
	char retcode = 0;
	BOOL uppercase;

	/*
	 * Control or special key !!!
	 */
	if (codepages[codepage][input & 0x7F] == 0) {
		if (input & 0x80) {
			switch (input & 0x7F) {
			case 0x1D:
				status &= ~KBD_LCTRL;
				break;

			case 0x2A:
				status &= ~KBD_LSHIFT;
				break;

			case 0x36:
				status &= ~KBD_RSHIFT;
				break;

			case 0x38:
				status &= ~KBD_LALT;
				break;

			case 0x3A:
				status &= ~KBD_CAPSLOCK;
				break;

			case 0x45:
				status &= ~KBD_NUMLOCK;
				break;

			case 0x46:
				status &= ~KBD_SCRLOCK;
				break;
			}
		} else {
			switch (input & 0x7F) {
			case 0x1D:
				status |= KBD_LCTRL;
				break;

			case 0x2A:
				status |= KBD_LSHIFT;
				break;

			case 0x36:
				status |= KBD_RSHIFT;
				break;

			case 0x38:
				status |= KBD_LALT;
				break;

			case 0x3A:
				status |= KBD_CAPSLOCK;
				break;

			case 0x45:
				status |= KBD_NUMLOCK;
				break;

			case 0x46:
				status |= KBD_SCRLOCK;
				break;
			}
		}

		return 0;
	}

	if ((input & 0x80) == 0) {
		/*
		 * The compiler regrets input is signed ...
		 */
		retcode = codepages[codepage][(unsigned char)input];
		/*
		 * Codes are all caps. Check if we need to report lower caps.
		 */
		if ((retcode >= 'A') && (retcode <= 'Z')) {
			uppercase = (status & KBD_CAPSLOCK) ? TRUE : FALSE;
			if (status & (KBD_LSHIFT | KBD_RSHIFT)) {
				uppercase = !!uppercase;
			}
			if (uppercase == FALSE) {
				retcode += 0x20;
			}
		}
	}

	return retcode;
}
