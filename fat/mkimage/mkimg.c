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
#include "../disk_access.h"
#include "../fat_format.h"

int mkimg(const char *image,
          const char *bootsec,
          const char *diegos,
          const struct MBR_partition_entry *table)
{
    FILE *mbr, *os;
    struct MBR buffer;
    char *osbuffer;
    void *ctx;
    struct disk_geometry geom;
    size_t retcode;
    int iretcode;
    unsigned i;
    struct FATPartition fat;

    if (!image || !bootsec || !diegos || !table)
        return -1;

    if (disk_init(image, &ctx))
        return -1;

    mbr = fopen(bootsec, "rb");
    if (!mbr)
    {
        disk_done(ctx);
        return -1;
    }

    retcode = fread(&buffer, sizeof(buffer), 1, mbr);
    fclose(mbr);

    if (retcode != 1)
    {
        disk_done(ctx);
        return -1;
    }

    memcpy(buffer.partitions, table, sizeof(buffer.partitions));

    if (disk_write(ctx, 0, 1, (char *)&buffer))
    {
        disk_done(ctx);
        return -1;
    }

    iretcode = MBR_get_active_partition(&buffer);
    if (iretcode < 0)
    {
        disk_done(ctx);
        return -1;
    }

    if (buffer.partitions[iretcode].type != PART_AOPDS)
    {
        disk_done(ctx);
        return -1;
    }

    os = fopen(diegos, "rb");
    if (!os)
    {
        disk_done(ctx);
        return -1;
    }

    disk_get_geometry(ctx, &geom);
    osbuffer = malloc(geom.bytes_per_sector);
    if (!osbuffer)
    {
        fclose(os);
        disk_done(ctx);
        return -1;
    }

    for (iretcode = 0; iretcode < 4; iretcode++)
    {
        if (buffer.partitions[iretcode].type == PART_AOPDS)
        {
            for (i = 0; i < buffer.partitions[iretcode].num_of_sectors; i++)
            {
                fread(osbuffer, geom.bytes_per_sector, 1, os);
                disk_write(ctx, buffer.partitions[iretcode].LBA_first_Sector + i, 1, osbuffer);
            }
            fseek(os, 0, SEEK_SET);
        }
        else if (buffer.partitions[iretcode].type == PART_FAT16_1)
        {
            fat.secpercluster = 0;
            fat.startsector = buffer.partitions[iretcode].LBA_first_Sector;
            fat.endsector = fat.startsector + buffer.partitions[iretcode].num_of_sectors - 1;
            memset(fat.label, ' ', sizeof(fat.label));
            if (FAT_format(ctx, &fat))
            {
                free(osbuffer);
                fclose(os);
                disk_done(ctx);
                return -1;
            }
        }
    }

    free(osbuffer);
    fclose(os);
    disk_done(ctx);

    return 0;
}
