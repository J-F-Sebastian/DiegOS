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

struct FATPartition {
    /* zero based LBA addresses */
    uint32_t startsector;
    uint32_t endsector;
    uint8_t  secpercluster;
    char label[16];
};

int FAT_format(void *ctx, struct FATPartition *part);

inline int FAT_mkfs(void *ctx, struct FATPartition *part)
{
    return FAT_format(ctx, part);
}

#endif // FAT_FORMAT_H
