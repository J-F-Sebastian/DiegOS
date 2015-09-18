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

#include <stdint.h>

typedef struct point {
    unsigned x;
    unsigned y;
} point_t;

typedef struct text_ui_driver {
    /*
     * Name of the UI driver
     */
    char name[DRV_NAME_LEN + 1];
    /*
     * Init function, it is to be invoked at startup to discover the supported devices,
     * assign resources for all units, and init the device.
     * Note: init does not mean start, run, enable interrupts, enable DMA.
     *       The kernel will not be able to handle events to or from devices until
     *       the start_fn function is invoked.
     *       The device should be configured to be ready to start working but kept
     *       reset or idle.
     * Return values:
     * EOK in case of success (device unit inited)
     * ENXIO in case the unitno is not supported or inited
     * other errnos for failures
     */
    int (*init_fn)(unsigned unitno);
    /*
     * Start the device's operations. This include interrupt handlers, DMA, I/O activities.
     */
    int (*start_fn)(unsigned unitno);
    /*
     * This function should shut down the device. It will be invoked on shutdown/reboot
     * of the kernel.
     */
    int (*done_fn)(unsigned unitno);
    /*
     *
     */
    void (*put_char_fn)(point_t pos, char ch, uint8_t color);
    void (*line_fn)(point_t a, point_t b, unsigned width, char ch, uint8_t color);
    void (*rectangle_fn)(point_t a, point_t b, unsigned width, char ch, uint8_t color);
    void (*filled_rectangle_fn)(point_t a, point_t b, char ch, uint8_t color);
    void (*write_text_bg_fn)(point_t a, uint8_t bgcolor, uint8_t color, const char *text);
    void (*show_fn)(void);
} text_driver_t;

typedef struct grafics_ui_driver {
    /*
     * Name of the UI driver
     */
    char name[DRV_NAME_LEN + 1];
    /*
     * Init function, it is to be invoked at startup to discover the supported devices,
     * assign resources for all units, and init the device.
     * Note: init does not mean start, run, enable interrupts, enable DMA.
     *       The kernel will not be able to handle events to or from devices until
     *       the start_fn function is invoked.
     *       The device should be configured to be ready to start working but kept
     *       reset or idle.
     * Return values:
     * EOK in case of success (device unit inited)
     * ENXIO in case the unitno is not supported or inited
     * other errnos for failures
     */
    int (*init_fn)(void);
    /*
     * Start the device's operations.
     * This include interrupt handlers, DMA, I/O activities.
     */
    int (*start_fn)(void);
    /*
     * This function should shut down the device.
     * It will be invoked on shutdown/reboot of the kernel.
     */
    int (*done_fn)(void);
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
} grafics_driver_t;


#endif // UI_DRIVERS_H_INCLUDED
