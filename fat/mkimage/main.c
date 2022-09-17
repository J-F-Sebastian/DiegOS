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

#include "mkimg.h"
#include "mkimgfd.h"

static void print_help(void)
{
	printf("Usage:\n\n\t<-f> -b <boot sector>\n\t-s <DiegOS>\n\t-d <image>\n\n");
	printf("Optional -f build a boot image for floppies.\n");
}

int main(int argc, char *argv[])
{
	unsigned i = 1;
	struct MBR_partition_entry table[4];
	uint32_t first, last;
	char *bs, *os, *img;
	FILE *temp;
	int floppy = 0;

	printf("DiegOS mkimage v2.0\n");
	if (argc < 7) {
		print_help();
		return 0;
	}

	if ((argc != 7) && (argc != 8)) {
		print_help();
		return 0;
	}

	if (argc == 8) {
		if ((argv[1][0] != '-') && (argv[1][1] != 'f')) {
			print_help();
			return 0;
		}
		floppy = 1;
		i++;
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
	strncpy(bs, &argv[i][0], strlen(&argv[i][0]));
	strncpy(os, &argv[i + 2][0], strlen(&argv[i + 2][0]));
	strncpy(img, &argv[i + 4][0], strlen(&argv[i + 4][0]));

	printf("BOOT SECTOR ... %s\n" "OS IMAGE ...... %s\n" "DISK IMAGE .... %s\n", bs, os, img);

	if (floppy) {
		printf("FLOPPY IMAGE\n\n");
		if (mkimgfd(img, bs, os)) {
			printf("mkimage ERROR\n");
		}

		free(bs);
		free(os);
		free(img);

		return 0;
	}

	temp = fopen(os, "rb");
	if (!temp) {
		printf("Ï/O ERROR: %s not found\n", os);
		free(bs);
		free(os);
		free(img);
		return -1;
	}

	memset(table, 0, sizeof(table));
	fseek(temp, 0, SEEK_END);
	table[0].type = PART_AOPDS;
	table[0].status = 0x80;
	table[0].LBA_first_Sector = 1;
	table[0].num_of_sectors = (ftell(temp) + 511) / 512;
	fclose(temp);

	first = table[0].LBA_first_Sector;
	last = first + table[0].num_of_sectors - 1;
	/* Head */
	table[0].CHS_first_sector[0] = (first / 18) % 2;
	/* Sector */
	table[0].CHS_first_sector[1] = (first % 18) + 1;
	/* Cylinder */
	table[0].CHS_first_sector[2] = first / (2 * 18);
	/* Head */
	table[0].CHS_last_sector[0] = (last / 18) % 2;
	/* Sector */
	table[0].CHS_last_sector[1] = (last % 18) + 1;
	/* Cylinder */
	table[0].CHS_last_sector[2] = last / (2 * 18);

	table[1].type = PART_FAT16_1;
	table[1].status = 0x00;
	table[1].LBA_first_Sector = last + 1;
	table[1].num_of_sectors = 2880 - table[1].LBA_first_Sector;
	first = last + 1;
	last = 2879;
	/* Head */
	table[1].CHS_first_sector[0] = (first / 18) % 2;
	/* Sector */
	table[1].CHS_first_sector[1] = (first % 18) + 1;
	/* Cylinder */
	table[1].CHS_first_sector[2] = first / (2 * 18);
	/* Head */
	table[1].CHS_last_sector[0] = (last / 18) % 2;
	/* Sector */
	table[1].CHS_last_sector[1] = (last % 18) + 1;
	/* Cylinder */
	table[1].CHS_last_sector[2] = last / (2 * 18);

	printf("\nPartition table entries:\n\n");

	/*
	 * C = LBA ÷ (HPC × SPT)
	 * H = (LBA ÷ SPT) mod HPC
	 * S = (LBA mod SPT) + 1
	 */

	for (i = 0; i < 4; i++) {
		printf("%u Type 0x%.2X Active %c CHS(%4u/%3u/%2u) -> CHS(%4u/%3u/%2u)"
		       " LBA %10u sectors %10u\n",
		       i,
		       table[i].type,
		       (table[i].status & 0x80) ? 'Y' : 'N',
		       ((int)(table[i].CHS_first_sector[1] & 0xc0) << 2) +
		       table[i].CHS_first_sector[2],
		       table[i].CHS_first_sector[0],
		       table[i].CHS_first_sector[1] & 0x3f,
		       ((int)(table[i].CHS_last_sector[1] & 0xc0) << 2) +
		       table[i].CHS_last_sector[2],
		       table[i].CHS_last_sector[0],
		       table[i].CHS_last_sector[1] & 0x3f,
		       table[i].LBA_first_Sector, table[i].num_of_sectors);
	}

	if (mkimg(img, bs, os, table)) {
		printf("mkimage ERROR\n");
	}

	free(bs);
	free(os);
	free(img);

	return 0;
}
