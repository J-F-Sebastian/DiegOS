/*
 * FAT File System
 *
 * Copyright (C) 2019 - 2026 Diego Gallizioli
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

static void print_help(void)
{
	printf("Usage:\n\n\t-b <boot sector>\n\t-s <DiegOS>\n\t-d <image>\n\n");
}

int main(int argc, char *argv[])
{
	unsigned i = 1;
	int retval;
	char *bs, *os, *img;

	printf("DiegOS mkimage for floppy v2.1\n");
	if (argc < 7) {
		print_help();
		return 0;
	}

	if (argc != 7) {
		print_help();
		return 0;
	}

	if ((argv[i][0] != '-') || (argv[i + 2][0] != '-') || (argv[i + 4][0] != '-')) {
		print_help();
		return 0;
	}

	if ((argv[i][1] != 'b') || (argv[i + 2][1] != 's') || (argv[i + 4][1] != 'd')) {
		print_help();
		return 0;
	}

	i++;

	bs = calloc(1, strlen(&argv[i][0]) + 1);
	os = calloc(1, strlen(&argv[i + 2][0]) + 1);
	img = calloc(1, strlen(&argv[i + 4][0]) + 1);

	if (!bs || !os || !img) {
		printf("ERROR: CANNOT ALLOCATE MEMORY\n");
		if (bs)
			free(bs);
		if (os)
			free(os);
		if (img)
			free(img);

		return -1;
	}

	strncpy(bs, &argv[i][0], strlen(&argv[i][0]));
	strncpy(os, &argv[i + 2][0], strlen(&argv[i + 2][0]));
	strncpy(img, &argv[i + 4][0], strlen(&argv[i + 4][0]));

	printf("BOOT SECTOR ... %s\n" "OS IMAGE ...... %s\n" "DISK IMAGE .... %s\n", bs, os, img);

	printf("FLOPPY IMAGE\n\n");
	retval = mkimgfd(img, bs, os);
	if (retval) {
		printf("mkimage ERROR %d\n", retval);
	}

	free(bs);
	free(os);
	free(img);

	return 0;
}
