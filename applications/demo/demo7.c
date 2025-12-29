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
#include <diegos/devices.h>
#include <diegos/kernel.h>
#include <errno.h>
#include <unistd.h>
// DEMO TEST !!!!
//#include <diegos/ui_drivers.h>
#include "../../drivers/VGA/vga.h"
//#include "../../drivers/VGA/vga_txt.h"

static void demo_thread_entry(void)
{
#if 0
	point_t a = { 0, 0 };
	point_t b = { 639, 479 };
	unsigned i;

	vga_drv.cmn.init_fn(0);

	for (i = 0; i < 16; i++) {
		vga_drv.rectangle_fn(a, b, 5, i);
		a.x += 5;
		a.y += 5;
		b.x -= 5;
		b.y -= 5;
	}

	a.x += 16;
	a.y += 16;
	b.x -= 16;
	b.y -= 16;
	vga_drv.write_text_fn(a, 15, "DiegOS DiegOS DiegOS");
#else
	point_t a, b, c, d;
	//unsigned i;

	vga_drv.cmn.init_fn(0);
	a.x = a.y = 0;
	b.x = 639;
	b.y = 479;
	vga_drv.filled_rectangle_fn(a, b, 7);
	a.x = 40;
	a.y = 30;
	b.x = 600;
	b.y = 30;
	c.x = 600;
	c.y = 450;
	d.x = 40;
	d.y = 450;
	vga_drv.line_fn(a, b, 1, 0);
	vga_drv.line_fn(a, d, 1, 2);
	vga_drv.line_fn(d, c, 1, 6);
	vga_drv.line_fn(b, c, 1, 5);
	a.x++;
	a.y++;
	b.x--;
	b.y++;
	c.x--;
	c.y--;
	d.x++;
	d.y--;
	vga_drv.line_fn(a, b, 1, 0);
	vga_drv.line_fn(a, d, 1, 2);
	vga_drv.line_fn(d, c, 1, 6);
	vga_drv.line_fn(b, c, 1, 5);
	a.x++;
	a.y++;
	b.x--;
	b.y++;
	c.x--;
	c.y--;
	d.x++;
	d.y--;
	vga_drv.filled_rectangle_fn(a, c, 2);

#endif
	while (TRUE)
		thread_suspend();
}

void platform_run(void)
{
	uint8_t pid;

	thread_create("Demo", THREAD_PRIO_NORMAL, demo_thread_entry, 0, 4096, &pid);
}
