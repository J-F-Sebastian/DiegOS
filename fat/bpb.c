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
#include "bpb.h"

int BPB_read(void *ctx, struct BPB *bpb)
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

	memcpy(bpb, buf, sizeof(*bpb));
	free(buf);

	return 0;
}

int BPB_write(void *ctx, struct BPB *bpb)
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

	memcpy(buf, bpb, sizeof(*bpb));

	if (disk_write(ctx, 0, 1, buf)) {
		free(buf);
		return -1;
	}

	free(buf);

	return 0;
}
