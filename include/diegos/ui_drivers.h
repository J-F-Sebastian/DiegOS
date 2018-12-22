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

#ifndef UI_DRIVERS_H_INCLUDED
#define UI_DRIVERS_H_INCLUDED

/*
 * Do not include this file directly - include drivers.h
 */

#include <stdint.h>

typedef struct point {
    int x;
    int y;
} point_t;

typedef struct palette {
    uint8_t red, green, blue;
} palette_t;

typedef struct text_ui_driver {
	/*
	 * Common functionalities
	 */
	driver_header_t cmn;    
    /*
     *  Set a single character on screen
     */	
    void (*put_char_fn)(point_t pos, char ch, uint8_t color);
	/*
     *  Write characters on screen
     */	
    void (*write_char_fn)(point_t a, point_t b, const char *ch, uint8_t color);
	/*
     * Draws a line from a to b.
	 * Width grows simmetrically.
     */
    void (*line_fn)(point_t a, point_t b, unsigned width, char ch, uint8_t color);
	/*
	 * Draws a rectangle from a to b.
	 * Width grows inward.
	 */
    void (*rectangle_fn)(point_t a, point_t b, unsigned width, char ch, uint8_t color);
	/*
	 * Draws a solid (filled) rectangle.
     */
    void (*filled_rectangle_fn)(point_t a, point_t b, char ch, uint8_t color);
	/*
	 * Blitters from the video buffer (not visible, readable, writable)
	 * to dbuffer. The copied area has a topmost left limit a and a bottom right
	 * limit b.
	 * To dump the whole buffer use a(0,0) and b(79,24).
	 */
    void (*bitblt_fn)(point_t a, point_t b, void *dbuffer);
	/*
	 * Blitters from sbuffer to the video buffer (not visible, readable, writable).
	 * The copied area has a topmost left limit a and a bottom right
	 * limit b.
	 * To dump the whole buffer use a(0,0) and b(79,24).
	 */
    void (*bitblt2_fn)(point_t a, point_t b, const void *sbuffer);
	/*
	 * Set the cursor position on screen
	 */
    void (*cursor_fn)(point_t pos);
	/*
	 * Set the cursor on (on != 0) or off (on == 0).
	 */
	void (*cursor_on_fn)(int on);
	/*
	 * Set the cursor size
	 */
	void (*cursor_size_fn)(unsigned startline, unsigned endline);
	/*
	 * Use a different font 
	 */
    void (*set_fonts_fn)(const unsigned char *fonts);
	/*
	 * Set 1 out of 16 palette color
	 */
    void (*set_palette_fn)(unsigned index, palette_t pal);
	/*
	 * Set all palette colors
	 */
    void (*set_palette_fn2)(const palette_t *pal);
	/*
	 * Make the video buffer visible, select a new video buffer
	 */
    void (*show_fn)(void);
    
} text_driver_t;

typedef struct grafics_ui_driver {
	/*
	 * Common functionalities
	 */
	driver_header_t cmn;    
    /*
     * Draw a pixel at coordinates a with color <color>
     */
    void (*put_pixel_fn)(point_t a, unsigned color);
    /*
     * Draw a line from coordinates a to coordinates b with color <color>,
     * line thickness is <width> in pixels
     */
    void (*line_fn)(point_t a, point_t b, unsigned width, unsigned color);
    /*
     * Draw a rectangle from coordinates a to coordinates b with color <color>,
     * line thickness is <width> in pixels
     */
    void (*rectangle_fn)(point_t a, point_t b, unsigned width, unsigned color);
    /*
     * Draw a filled rectangle from coordinates a to coordinates b
     * with color <color>, line thickness is <width> in pixels
     */
    void (*filled_rectangle_fn)(point_t a, point_t b, unsigned color);
    /*
     * Write text at coordinates a with color <color>, text must be a
     * null-terminated string
     */
    void (*write_text_fn)(point_t a, unsigned color, const char *text);
    /*
     * Write text at coordinates a with color <color> and background color
     * <bgcolor>, text must be a null-terminated string
     */
    void (*write_text_bg_fn)(point_t a,
                             unsigned bgcolor,
                             unsigned color,
                             const char *text);
    /*
     * Load a palette color at index <index>, rgb array is indexed as
     * RED, GREEN, BLUE
     */
    void (*load_palette_fn)(unsigned index,
                            uint8_t rgb[]);
    /*
     * Load all palette colors, rgb array is indexed as RED, GREEN, BLUE and
     * must carry a sequence as long as the palette size
     */
    void (*load_all_palette_fn)(uint8_t rgb[]);

    /*
     * Store a palette color at index <index>, rgb array is indexed as
     * RED, GREEN, BLUE
     */
    void (*store_palette_fn)(unsigned index,
                            uint8_t rgb[]);
    /*
     * Store all palette colors, rgb array is indexed as RED, GREEN, BLUE and
     * must carry a sequence as long as the palette size
     */
    void (*store_all_palette_fn)(uint8_t rgb[]);

    /*
     * Draw a sprite; the source data is a bitmap whose size is WxH, W must be
     * a multiple of 8. Only 1s are written to video.
     */
    void (*draw_sprite_fn)(point_t a,
                           uint8_t sprite[],
                           unsigned W,
                           unsigned H,
                           unsigned color);
    /*
     * Show the drawing buffer, it might imply switching buffers, copying
     * from system memory to video memory,...
     */
    void (*show_fn)(void);
    /*
     * Set a new array for fonts bitmaps.
     * The caller must specify the font pixel width and height, it is implicitely
     * assumed the number of characters is 256 (from 0 to 0xFF)
     */
    void (*set_font_fn)(uint8_t fonts[], unsigned W, unsigned H);
    /*
     * Set the desired resolution and color depth.
     * In case loose is TRUE, the closest resolution available is selected
     */
    int (*set_res_fn)(unsigned W, unsigned H, unsigned depth, int loose);  
	
} grafics_driver_t;

#endif // UI_DRIVERS_H_INCLUDED
