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

#ifndef _VGA_PALETTES_H_
#define _VGA_PALETTES_H_

#include <diegos/drivers.h>

/*
 * Preconfigured palettes
 */
enum VGA_PALETTE {
	VGA_PAL_VGA,
	VGA_PAL_WIN,
	VGA_PAL_MAC,
	VGA_PAL_RISCOS,
	VGA_PAL_HTML,
	VGA_PAL_DEFAULT = VGA_PAL_VGA
};

const palette_t *vga_get_palette (enum VGA_PALETTE pal);

#endif
