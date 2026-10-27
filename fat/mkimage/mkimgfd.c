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
#include <errno.h>

#include "mkimgfd.h"
#include "../disk_access.h"
#include "../fat_format.h"

/*
 * The layout of a floppy disk image is
 *
 * 0 - Boot Sector (BPB)
 * 1 - FAT16.1 Parameter Block (PB)
 * 2 - Reserved sectors (RES) storing DiegOS.bin
 * 3 - FAT16.1 File System (FAT)
 */

int mkimgfd(const char *image, const char *bootsec, const char *diegos)
{
	FILE *bootsector, *os;
	char *osbuffer;
	void *ctx;
	struct disk_geometry geom;
	size_t retcode;
	unsigned i;
	unsigned short temp;
	struct FATPartition fat;
	struct BPB buffer = {
		{0xEB, 0x3C, 0x90},
		{'D', 'i', 'e', 'g', 'O', 'S', ' ', ' '},
		{0x00, 0x02},
		2,
		{1, 0},
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
		{'N', 'O', ' ', 'N', 'A', 'M', 'E', ' ', ' ', ' ', ' '},
		{'F', 'A', 'T', ' ', ' ', ' ', ' ', ' '},
		{0},
		{0x55, 0xAA}
	};

	if (!image || !bootsec || !diegos)
		return -EINVAL;

	if (disk_init(image, &ctx))
		return -EINVAL;

	bootsector = fopen(bootsec, "rb");
	if (!bootsector) {
		disk_done(ctx);
		return -EINVAL;
	}

	fseek(bootsector, 62, SEEK_SET);
	retcode = fread((char *)&buffer + 62, sizeof(buffer.bootcode) + 2, 1, bootsector);
	fclose(bootsector);

	if (retcode != 1) {
		disk_done(ctx);
		return -EINVAL;
	}

	os = fopen(diegos, "rb");
	if (!os) {
		disk_done(ctx);
		return -EINVAL;
	}

	/*
	 * os image size in sectors
	 */
	fseek(os, 0, SEEK_END);
	temp = (ftell(os) + 511) / 512;

	fat.startsector = 1;
	fat.endsector = 2879;
	fat.secpercluster = 2;
	fat.reservedsectors = temp;
	memset(fat.label, ' ', sizeof(fat.label));

	/*
	 * Format the FAT file system
	 */
	if (FAT_format(ctx, &fat)) {
		fclose(os);
		disk_done(ctx);
		return -EINVAL;
	}

	/*
	 * Write the BPB to disk
	 */
	if (disk_write(ctx, 0, 1, (char *)&buffer)) {
		fclose(os);
		disk_done(ctx);
		return -EINVAL;
	}

	disk_get_geometry(ctx, &geom);
	osbuffer = malloc(geom.bytes_per_sector);
	if (!osbuffer) {
		fclose(os);
		disk_done(ctx);
		return -ENOMEM;
	}

	fseek(os, 0, SEEK_SET);

	/*
	 * Write the OS image to disk in the reserved sectors
	 * skipping the Parameter Block (PB) which is
	 * the first sector of the reserved area
	 */
	for (i = 2; i < temp; i++) {
		fread(osbuffer, geom.bytes_per_sector, 1, os);
		disk_write(ctx, i, 1, osbuffer);
	}

	free(osbuffer);
	fclose(os);
	disk_done(ctx);

	return 0;
}
