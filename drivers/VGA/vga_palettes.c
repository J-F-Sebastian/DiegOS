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

#include "vga_palettes.h"

static const palette_t CGA_16_palette[] = {
	/* RGB, 16 palettes */
	/* black */
	{0x00, 0x00, 0x00},
	/* blue */
	{0x00, 0x00, 0xAA},
	/* green */
	{0x00, 0xAA, 0x00},
	/*cyan */
	{0x00, 0xAA, 0xAA},
	/*red */
	{0xAA, 0x00, 0x00},
	/* magenta */
	{0xAA, 0x00, 0xAA},
	/* brown */
	{0xAA, 0x55, 0x00},
	/* light gray */
	{0xAA, 0xAA, 0xAA},
	/* gray */
	{0x55, 0x55, 0x55},
	/* light blue */
	{0x55, 0x55, 0xFF},
	/* light green */
	{0x55, 0xFF, 0x55},
	/* light cyan */
	{0x55, 0xFF, 0xFF},
	/* light red */
	{0xFF, 0x55, 0x55},
	/* light magenta */
	{0xFF, 0x55, 0xFF},
	/* yellow */
	{0xFF, 0xFF, 0x55},
	/* white */
	{0xFF, 0xFF, 0xFF}
};

static const palette_t Windows_16_palette[] = {
	/* RGB, 16 palettes */
	/* black */
	{0x00, 0x00, 0x00},
	/* maroon */
	{0x80, 0x00, 0x00},
	/* green */
	{0x00, 0x80, 0x00},
	/* olive */
	{0x80, 0x80, 0x00},
	/* navy */
	{0x00, 0x00, 0x80},
	/* purple */
	{0x80, 0x00, 0x80},
	/* teal */
	{0x00, 0x80, 0x80},
	/* silver */
	{0xC0, 0xC0, 0xC0},
	/* gray */
	{0x80, 0x80, 0x80},
	/* red */
	{0xFF, 0x00, 0x00},
	/* lime */
	{0x00, 0xFF, 0x00},
	/* yellow */
	{0xFF, 0xFF, 0x00},
	/* blue */
	{0x00, 0x00, 0xFF},
	/* fuchsia */
	{0xFF, 0x00, 0xFF},
	/* aqua */
	{0x00, 0xFF, 0xFF},
	/* white */
	{0xFF, 0xFF, 0xFF}
};

static const palette_t Mac_16_palette[] = {
	/* RGB, 16 palettes */
	/* white */
	{0xFF, 0xFF, 0xFF},
	/* yellow */
	{0xFB, 0xF3, 0x05},
	/* orange */
	{0xFF, 0x64, 0x03},
	/* red */
	{0xDD, 0x09, 0x07},
	/* magenta */
	{0xF2, 0x08, 0x84},
	/* purple */
	{0x47, 0x00, 0xA5},
	/* blue */
	{0x00, 0x00, 0xD3},
	/* cyan */
	{0x02, 0xAB, 0xEA},
	/* green */
	{0x1F, 0xB7, 0x14},
	/* dark green */
	{0x00, 0x64, 0x12},
	/* brown */
	{0x56, 0x2C, 0x05},
	/* tan */
	{0x90, 0x71, 0x3A},
	/* light grey */
	{0xC0, 0xC0, 0xC0},
	/* medium grey */
	{0x80, 0x80, 0x80},
	/* dark grey */
	{0x40, 0x40, 0x40},
	/* black */
	{0x00, 0x00, 0x00}
};

static const palette_t RISC_OS_palette[] = {
    /* RGB, 16 palettes */
    /* White */
	{0xFF, 0xFF, 0xFF},
    /* Grey 1/7 */
	{0xDD, 0xDD, 0xDD},
    /* Grey 2/7 */
	{0xBB, 0xBB, 0xBB},
    /* Grey 3/7 */
	{0x99, 0x99, 0x99},
    /* Grey 4/7 */
	{0x77, 0x77, 0x77},
    /* Grey 5/7 */
	{0x55, 0x55, 0x55},
    /* Grey 6/7 */
	{0x33, 0x33, 0x33},
    /* Black */
	{0x00, 0x00, 0x00},
    /* Dark Blue */
	{0x00, 0x44, 0x99},
    /* Yellow */
	{0xEE, 0xEE, 0x00},
    /* Green */
	{0x00, 0xCC, 0x00},
    /* Red */
	{0xDD, 0x00, 0x00},
    /* Beige */
	{0xEE, 0xEE, 0xBB},
    /* Dark Green */
	{0x55, 0x88, 0x00},
    /* Orange */
	{0xFF, 0xBB, 0x00},
    /* Light Blue */
	{0x00, 0xBB, 0xFF}
};

static const palette_t HTML_palette[] = {
    /* RGB, 16 palettes */
    /* White */
	{0xFF, 0xFF, 0xFF},
    /* Silver */
	{0xC0, 0xC0, 0xC0},
    /* Gray */
	{0x80, 0x80, 0x80},
    /* Black */
	{0x00, 0x00, 0x00},
    /* Red */
	{0xFF, 0x00, 0x00},
    /* Maroon */
	{0x80, 0x00, 0x00},
    /* Yellow */
	{0xFF, 0xFF, 0x00},
    /* Olive */
	{0x80, 0x80, 0x00},
    /* Lime */
	{0x00, 0xFF, 0x00},
    /* Green */
	{0x00, 0x80, 0x00},
    /* Aqua */
	{0x00, 0xFF, 0xFF},
    /* Teal */
	{0x00, 0x80, 0x80},
    /* Blue */
	{0x00, 0x00, 0xFF},
    /* Navy */
	{0x00, 0x00, 0x80},
    /* Fuchsia */
	{0xFF, 0x00, 0xFF},
    /* Purple */
	{0x80, 0x00, 0x80}
};

const palette_t *vga_get_palette (enum VGA_PALETTE pal)
{
	switch (pal)
	{
		case VGA_PAL_VGA:
			return CGA_16_palette;
			
		case VGA_PAL_WIN:
			return Windows_16_palette;
			
		case VGA_PAL_MAC:
			return Mac_16_palette;
			
		case VGA_PAL_RISCOS:
			return RISC_OS_palette;

		case VGA_PAL_HTML:
			return HTML_palette;
			
		default:
			return CGA_16_palette; 
	}
}
