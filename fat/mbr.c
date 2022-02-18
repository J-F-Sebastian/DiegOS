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

#include "disk_access.h"
#include "mbr.h"

int MBR_read(void *ctx, struct MBR *mbr)
{
	struct disk_geometry geom;
	char *buf;

	if (disk_get_geometry(ctx, &geom))
		return -1;

	buf = malloc(geom.bytes_per_sector);

	if (disk_read(ctx, 0, 1, buf)) {
		free(buf);
		return -1;
	}

	memcpy(mbr, buf, sizeof(*mbr));
	free(buf);

	return 0;
}

void MBR_list(struct MBR *mbr)
{
	unsigned i;
	struct MBR_partition_entry *cursor;

	if (!mbr)
		return;

	printf("+-----------------------------------------"
	       " MBR Partition entries " "-----------------------------------------+\n");
	printf("|                                         "
	       "                       " "                                         |\n");
	for (i = 0, cursor = mbr->partitions; i < 4; i++, cursor++) {
		printf("| %c%u) Type 0x%.2X Status 0x%.2X  CHS(%4u/%3u/%2u) -> CHS(%4u/%3u/%2u)"
		       " first LBA %10u sectors %10u |\n",
		       (cursor->status & 0x80) ? '*' : ' ',
		       i,
		       cursor->type,
		       (uint8_t) cursor->status,
		       ((int)(cursor->CHS_first_sector[1] & 0xc0) << 2) +
		       cursor->CHS_first_sector[2],
		       cursor->CHS_first_sector[0],
		       cursor->CHS_first_sector[1] & 0x3f,
		       ((int)(cursor->CHS_last_sector[1] & 0xc0) << 2) +
		       cursor->CHS_last_sector[2],
		       cursor->CHS_last_sector[0],
		       cursor->CHS_last_sector[1] & 0x3f,
		       cursor->LBA_first_Sector, cursor->num_of_sectors);
	}
	printf("|                                         "
	       "                       " "                                         |\n");
	printf("+-----------------------------------------"
	       "-----------------------" "-----------------------------------------+\n");
}

int MBR_get_active_partition(struct MBR *mbr)
{
	int i;
	struct MBR_partition_entry *cursor;

	if (!mbr)
		return -1;

	for (i = 0, cursor = mbr->partitions; i < 4; i++, cursor++) {
		/* Active partition status is 0x80 */
		if (cursor->status == (char)0x80)
			break;
	}

	return (i < 4) ? i : -1;
}

int MBR_get_partition_entry(struct MBR *mbr, int partnum, struct MBR_partition_entry *entry)
{
	if (!mbr || (partnum < 0) || (partnum > 3) || !entry)
		return -1;

	*entry = mbr->partitions[partnum];

	return 0;
}
