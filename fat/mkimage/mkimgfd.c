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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mkimgfd.h"
#include "../disk_access.h"
#include "../fat_format.h"

int mkimgfd(const char *image, const char *bootsec, const char *diegos)
{
	FILE *bootsector, *os;
	char *osbuffer;
	void *ctx;
	struct disk_geometry geom;
	size_t retcode;
	unsigned i;
	unsigned short temp;
	struct BPB buffer = {
		{0xEB, 0x3C, 0x90},
		{"DiegOS  "},
		{0x00, 0x02},
		1,
		{0, 0},
		2,
		{0x00, 0x01},
		{0x40, 0x0B},
		0xF0,
		{12, 0},
		{18, 0},
		{2, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		0,
		0,
		0x29,
		{0xDE, 0xAD, 0xBE, 0xEF},
		"NO NAME    ",
		"FAT     ",
		{0},
		{0x55, 0xAA}
	};

	if (!image || !bootsec || !diegos)
		return -1;

	if (disk_init(image, &ctx))
		return -1;

	bootsector = fopen(bootsec, "rb");
	if (!bootsector) {
		disk_done(ctx);
		return -1;
	}

	fseek(bootsector, 62, SEEK_SET);
	retcode = fread(buffer.bootcode, sizeof(buffer.bootcode) + 2, 1, bootsector);
	fclose(bootsector);

	if (retcode != 1) {
		disk_done(ctx);
		return -1;
	}

	os = fopen(diegos, "rb");
	if (!os) {
		disk_done(ctx);
		return -1;
	}

	/*
	 * 1 + os image size in sectors
	 */
	fseek(os, 0, SEEK_END);
	temp = 1 + (ftell(os) + 511) / 512;
	*(short *)(buffer.BPB_RsvdSecCnt) = temp;

	if (disk_write(ctx, 0, 1, (char *)&buffer)) {
		disk_done(ctx);
		return -1;
	}

	disk_get_geometry(ctx, &geom);
	osbuffer = malloc(geom.bytes_per_sector);
	if (!osbuffer) {
		fclose(os);
		disk_done(ctx);
		return -1;
	}

	fseek(os, 0, SEEK_SET);

	for (i = 1; i < temp; i++) {
		fread(osbuffer, geom.bytes_per_sector, 1, os);
		disk_write(ctx, i, 1, osbuffer);
	}

	memset(osbuffer, 0xDF, geom.bytes_per_sector);

	for (i = temp; i < 2880; i++)
		disk_write(ctx, i, 1, osbuffer);

	free(osbuffer);
	fclose(os);
	disk_done(ctx);

	return 0;
}
