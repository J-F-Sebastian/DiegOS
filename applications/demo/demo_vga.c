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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <diegos/devices.h>
#include <diegos/kernel.h>
#include <errno.h>
#include <unistd.h>
// DEMO TEST !!!!
#include "../../drivers/VGA/vga.h"

unsigned W = 8, H = 16;

void gv_zoom (point_t a, point_t b, int zoom)
{
    a.x += zoom;
    a.y += zoom;
    b.x -= zoom;
    b.y -= zoom;
}


void gv_desktop (uint8_t color, const uint8_t *pattern)
{
    point_t a = {0,0};
    point_t b = {639,479};

    if (!pattern) {
        vga_drv.filled_rectangle_fn(a,b,color);
    } else {

    }
}

/*
 * colors
 *      title fg
 *      title bg
 *      frame
 *      background
 */
void gv_frame (point_t a, point_t b, const char *title, uint8_t colors[])
{
    size_t titlelen;
    char buffer[81];

    if (!title || (a.x > b.x) || (a.y > b.y)) {
        return;
    }

    titlelen = strlen(title);
    if (titlelen > 78) {
        titlelen = 78;
    }
    if ((b.x - a.x) < titlelen*8) {
        titlelen = (b.x - a.x)/8;
    }

    strncpy(buffer + 1, title, titlelen);
    buffer[0] = ' ';
    buffer[titlelen + 1] = ' ';
    buffer[titlelen + 2] = '\0';
    vga_drv.write_text_bg_fn(a, colors[1], colors[0], buffer);
    a.y += H;
    vga_drv.rectangle_fn(a, b, 2, colors[2]);
    a.x += 2;
    a.y += 2;
    b.x -= 2;
    b.y -= 2;
    vga_drv.filled_rectangle_fn(a, b, colors[3]);
}

/*
 * colors
 *      text fg
 *      text bg
 */
void gv_textbox (point_t a, point_t b, const char *text, uint8_t colors[],
                 int alignment, unsigned linesp)
{
    size_t textlen;
    size_t linelen;
    size_t linemax;
    unsigned kerning;
    char buffer[81];
    int backg = (colors[0] == colors[1]) ? 0 : 1;

    if (!text || (a.x > b.x) || (a.y > b.y)) {
        return;
    }

    /*
     * A line of text goes from a.x to b.y with some spacing (kerning)
     */
    linesp += H;
    textlen = strlen(text);
    linemax = (b.x - a.x)/8;
    kerning = ((b.x - a.x) - linemax*8)/2;
    if (0 == kerning) {
        linemax--;
        kerning = 4;
    }
    a.x += kerning;

    while ((textlen > linemax) && (a.y + linesp < b.y)) {
        /*
         * Set the maximum line length to be copied for this run,
         * and break the text at space characters. It should be at
         * break characters including punctuations etc.
         */
        linelen = linemax;
        while (linelen && !isspace(text[linelen]) && !ispunct(text[linelen])) {
            linelen--;
        }
        /*
         * No break in the line !!
         */
        if (0 == linelen) {
            linelen = linemax;
        }

        strncpy(buffer, text, linelen+1);
        if (isspace(buffer[linelen])) {
            buffer[linelen] = 0;
        } else {
            buffer[linelen+1] = 0;
        }
        if (alignment == 1) {
            kerning = ((b.x - a.x) - (linelen+1)*8)/2;
        } else if (alignment == 2) {
            if (buffer[linelen]) {
                kerning = ((b.x - a.x) - (linelen+1)*8) -1;
            } else {
                kerning = ((b.x - a.x) - (linelen)*8) - 1;
            }
        }
        a.x += kerning;
        if (backg) {
            vga_drv.write_text_bg_fn(a, colors[1], colors[0], buffer);
        } else {
            vga_drv.write_text_fn(a, colors[0], buffer);
        }
        text += linelen;
        textlen -= linelen;
        a.y += linesp;
        a.x -= kerning;
        text++;
        textlen--;
    }

    if (textlen && (a.y + linesp < b.y)) {
        if (alignment == 1) {
            a.x += ((b.x - a.x) - textlen*8)/2;
        } else if (alignment == 2) {
            a.x += ((b.x - a.x) - textlen*8) - 1;
        }
        if (backg) {
            vga_drv.write_text_bg_fn(a, colors[1], colors[0], text);
        } else {
            vga_drv.write_text_fn(a, colors[0], text);
        }
    }
}

static const char txt[] = "Testo di prova per spaziatura,"
                          "variabile con salto di riga senza uscire dal "
                          "box frame.";

static void demo_thread_entry(void)
{
    point_t a,b;
    uint8_t colors[] = {0,8,8,15};
    uint8_t colors2[] = {0,13,13,10};
    uint8_t colors3[] = {0,14,14,9};
    uint8_t color4[] = {8, 8};

    vga_drv.init_fn();
    W= 8;
    H = 16;

    vga_drv.set_font_fn(NULL, W, H);
    gv_desktop(12, NULL);
#if 0
    a.x = 10;
    a.y = 13;
    b.x = 603;
    b.y = 457;
    gv_frame(a, b, "DiegOS", colors);
    thread_delay(1000);
    gv_frame(a, b, "Sono tutti tests", colors);
    thread_delay(1000);
    a.x = 123;
    a.y = 34;
    b.x = 400;
    b.y = 399;
    colors[0] = 13;
    colors[1] = 12;
    colors[2] = 11;
    colors[3] = 10;
    gv_frame(a, b, "DiegOS", colors);
    a.y += (H + 2);
    a.x += 2;
    colors2[1] = 6;
    gv_textbox(a, b, txt, colors2, 0, 0);
    colors2[1] = 12;
    a.y += 55;
    W= 8;
    H = 8;
    vga_drv.set_font_fn(NULL, W, H);
    gv_textbox(a, b, txt, colors2, 1, 2);
    colors2[1] = 13;
    a.y += 59;
    gv_textbox(a, b, txt, colors2, 2, 3);
#else
    a.x = 4;
    a.y = 4;
    b.x = 635;
    b.y = H*6;
    gv_frame(a, b, "DiegOS", colors);
    a.y += H;
    gv_zoom(a, b, 2);
    gv_textbox(a, b, "Welcome to DiegOS Operating System Release 1.0",color4,0,0);
    gv_zoom(a, b, -2);
    a.y -= H;
    a.y += H*7;
    b.y += H*7;
    gv_frame(a, b, "DiegOS", colors2);
    a.y += H;
    gv_zoom(a, b, 2);
    gv_textbox(a, b, "Welcome to DiegOS Operating System Release 1.0",color4,0,0);
    gv_zoom(a, b, -2);
    a.y -= H;
    a.y += H*7;
    b.y += H*7;
    gv_frame(a, b, "DiegOS", colors3);
    a.y += H;
    gv_zoom(a, b, 2);
    gv_textbox(a, b, "Welcome to DiegOS Operating System Release 1.0",color4,0,0);
    gv_zoom(a, b, -2);
    a.y -= H;
#endif
    while (TRUE) thread_suspend();
}

void platform_run(void)
{
    uint8_t pid;

    thread_create("VGADemo",
                  THREAD_PRIO_NORMAL,
                  demo_thread_entry,
                  0,
                  8192,
                  &pid);
}
