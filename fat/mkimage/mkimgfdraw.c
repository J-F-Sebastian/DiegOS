/*
 * FAT File System
 *
 * Copyright (C) 2019 Diego Gallizioli
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

 /*
  * The layout of a floppy disk image is
  *
  * 0 - Boot Sector (BPB)
  * 1 - FAT16.1 Parameter Block (PB)
  * 2 - Reserved sectors (RES) storing DiegOS.bin
  * 3 - FAT16.1 File System (FAT)
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../disk_access.h"

#define NELEMENTS(x) (sizeof(x)/sizeof(x[0]))

static uint16_t buffer[512 / 2];

static void fill_buffer(uint16_t value)
{
	unsigned i;

	for (i = 0; i < NELEMENTS(buffer); i++)
		buffer[i] = value;
}

static int mkimgfdraw(const char *image)
{
	void *ctx;
	struct disk_geometry geom;
	size_t retcode;
	unsigned i;

	if (!image)
		return -EINVAL;

	if (disk_init(image, &ctx))
		return -EINVAL;

	disk_get_geometry(ctx, &geom);

	/*
	 * Write the OS image to disk in the reserved sectors
	 * skipping the Parameter Block (PB) which is
	 * the first sector of the reserved area
	 */
	for (i = 0; i < geom.num_of_sectors; i++) {
		fill_buffer(i);
		disk_write(ctx, i, 1, (char *)buffer);
	}

	disk_done(ctx);

	return 0;
}

static void print_help(void)
{
	printf("Usage:\n-d <image>\n\n");
}

int main(int argc, char *argv[])
{
	unsigned i = 1;
	int retval;
	char *bs, *os, *img;

	printf("DiegOS mkimage for floppy v2.1 - raw image\n");
	if (argc < 3) {
		print_help();
		return 0;
	}

	if (argc != 3) {
		print_help();
		return 0;
	}

	if (argv[i][0] != '-' && argv[i][1] != 'd') {
		print_help();
		return 0;
	}

	i++;

	img = calloc(1, strlen(&argv[i][0]) + 1);

	if (!img) {
		printf("ERROR: CANNOT ALLOCATE MEMORY\n");
		if (img)
			free(img);

		return 1;
	}

	strncpy(img, &argv[i][0], strlen(&argv[i][0]));

	printf("FLOPPY IMAGE .... %s\n", img);

	retval = mkimgfdraw(img);
	if (retval) {
		printf("mkimagefdraw ERROR %d\n", retval);
	}

	free(img);

	return 0;
}
