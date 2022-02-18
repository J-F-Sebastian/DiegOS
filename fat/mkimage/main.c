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

static void print_help(void)
{
	printf("Usage:\n\n\t-b <boot sector>\n\t-s <DiegOS>\n\t-d <image>\n\n");
}

int main(int argc, char *argv[])
{
	unsigned i = 0;
	struct MBR_partition_entry table[4];
	uint32_t first, last;
	char *bs, *os, *img;
	FILE *temp;

	printf("DiegOS mkimage v1.0\n");
	if (argc < 4) {
		print_help();
		return 0;
	}

	if ((argv[1][0] != '-') || (argv[3][0] != '-') || (argv[5][0] != '-')) {
		print_help();
		return 0;
	}

	if ((argv[1][1] != 'b') || (argv[3][1] != 's') || (argv[5][1] != 'd')) {
		print_help();
		return 0;
	}

	bs = calloc(1, strlen(&argv[2][0]) + 1);
	os = calloc(1, strlen(&argv[4][0]) + 1);
	img = calloc(1, strlen(&argv[6][0]) + 1);
	strncpy(bs, &argv[2][0], strlen(&argv[2][0]));
	strncpy(os, &argv[4][0], strlen(&argv[4][0]));
	strncpy(img, &argv[6][0], strlen(&argv[6][0]));

	printf("BOOT SECTOR ... %s\n" "OS IMAGE ...... %s\n" "DISK IMAGE .... %s\n", bs, os, img);
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
