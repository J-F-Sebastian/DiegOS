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

#ifndef FAT_FORMAT_H
#define FAT_FORMAT_H

#include <stdint.h>

#include "disk_access.h"

/*
 * Structure used as input to FAT_format and FAT_mkfs to
 * describe a FAT partition (volume).
 */
struct FATPartition {
	/* zero based LBA addresses */
	uint32_t startsector;
	uint32_t endsector;
	uint8_t secpercluster;
	/* Volume Label */
	char label[16];
};

/**
 * @brief FAT_format formats a FAT file system on the partition described by part.
 * The storage media is accessed through disk_access API calls.
 * The opaque context ctx is provided from the disk_access API.
 * On success the FAT file system is created and written on disk,
 * and the BPB is written on the first sector of the partition.
 * It is a responsibility of the caller to get a context from the disk_access API
 * before calling this function.
 * A partition is then a continuous area of the storage media,
 * delimited by startsector and endsector, where the FAT file system is created.
 *
 * PARAMETERS IN
 * @param ctx opaque I/O context provided by disk_access API
 * @param part pointer to the FATPartition structure describing the partition to be formatted
 *
 * RETURNS
 * @return 0 on success
 * @return -1 on failure
 */
int FAT_format(void *ctx, struct FATPartition *part);

/**
 * @brief FAT_mkfs is an alias for FAT_format, it is provided for compatibility with the standard mkfs API.
 */
inline int FAT_mkfs(void *ctx, struct FATPartition *part)
{
	return FAT_format(ctx, part);
}

#endif
