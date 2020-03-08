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

#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "fat_data.h"
#include "fat_format.h"
#include "fat_names.h"
#include "fat_internals.h"

#define MAX_CLUSTERS 65524U
/* default to 1 reserved sector */
#define RES_SECTORS  1U
/* default to 2 FATs */
#define NUM_FATS     2U
#define MAX_CLUS_SIZE 32768U

static uint8_t compute_secperclus(uint16_t bytes_per_sector, uint32_t sectors)
{
    uint8_t retval = 1;
    unsigned clusters = MAX_CLUSTERS;
    sectors -= RES_SECTORS;

    /*
     * Do not take into account the FAT area now.
     * Loop until the clusters can address all sectors
     */
    while (retval && ((clusters * retval) < sectors))
    {
        retval += retval;
        if ((retval * bytes_per_sector) > MAX_CLUS_SIZE)
        {
            retval = 0;
            break;
        }
    }

    /*
     * if retval is nul, sectors are too much to be addressed, or the cluster size exceeds 32k
     */
    return retval;
}

int FAT_format(void *ctx, struct FATPartition *part)
{
    struct FATVolume newvolume;
    struct FAT *volumeid;
    struct disk_geometry geom;
    uint32_t sectors = part->endsector - part->startsector + 1;
    uint8_t check;
    uint32_t TmpVal1, TmpVal2, FATSz;
    char *buffer = 0;
    unsigned i,j,k;

    if (!ctx || !part)
        return -1;

    for (i = 0; i < sizeof(part->label); i++)
        if (FAT_check_character(part->label[i]))
            return -1;

    if (disk_get_geometry(ctx, &geom))
        return -1;

    if (!part->secpercluster)
    {
        part->secpercluster = compute_secperclus(geom.bytes_per_sector, sectors);
        if (!part->secpercluster)
            return -1;
    }
    else
    {
        for (check = 1; check; check += check)
            if (part->secpercluster == check)
                break;

        if (!check)
            return -1;

        if (part->secpercluster * geom.bytes_per_sector > MAX_CLUS_SIZE)
            return -1;
    }

    strncpy(newvolume.PB.OEMName, "DiegOS  ", sizeof(newvolume.PB.OEMName));
    newvolume.PB.BytsPerSec = geom.bytes_per_sector;
    newvolume.PB.SecPerClus = part->secpercluster;
    newvolume.PB.ResvdSecCnt = RES_SECTORS;
    newvolume.PB.NumFATs = NUM_FATS;
    newvolume.PB.TotSec = sectors;

    TmpVal1 = sectors - newvolume.PB.ResvdSecCnt;
    TmpVal2 = (256 * newvolume.PB.SecPerClus) + newvolume.PB.NumFATs;
    FATSz = (TmpVal1 + (TmpVal2 - 1)) / TmpVal2;
    newvolume.PB.FATSz16 = FATSz & UINT16_MAX;

    newvolume.PB.HiddSec = 0;
    newvolume.PB.VolID = (uint32_t)time(0);
    memset(newvolume.PB.VolLab, ' ', sizeof(newvolume.PB.VolLab));
    strncpy(newvolume.PB.VolLab, part->label, sizeof(newvolume.PB.VolLab));
    memset(newvolume.PB.FilSysType, 0, sizeof(newvolume.PB.FilSysType));
    strncpy(newvolume.PB.FilSysType, "FAT16.1 ", sizeof(newvolume.PB.FilSysType));
    newvolume.PB.RootClus = 2;

    buffer = malloc(geom.bytes_per_sector);
    if (!buffer)
        return -1;

    /*
     * A FAT file system volume is composed of three basic regions,
     * which are laid out in this order on the volume:
     *
     * 0 – Reserved Region
     * 1 – FAT Region
     * 2 – File and Directory Data Region
     */

    /* Reserved Region */
    memset(buffer, 0, geom.bytes_per_sector);
    memcpy(buffer, &newvolume.PB, sizeof(newvolume.PB));
    if (disk_write(ctx, part->startsector, 1, buffer))
        goto terminate;

    memset(buffer, 0, geom.bytes_per_sector);
    for (i = 1; i < newvolume.PB.ResvdSecCnt; i++)
        if (disk_write(ctx, part->startsector + i, 1, buffer))
            goto terminate;

    /* FAT Region */
    for (j = 0; j < newvolume.PB.NumFATs; j++)
    {
        k = part->startsector + newvolume.PB.ResvdSecCnt;
        k += j*newvolume.PB.FATSz16;
        memset(buffer, 0, geom.bytes_per_sector);
        buffer[0] = (char)0x00;
        buffer[1] = (char)0xFF;
        buffer[2] = (char)0xFF;
        buffer[3] = (char)0xFF;
        buffer[newvolume.PB.RootClus*2] = (char)0xFF;
        buffer[newvolume.PB.RootClus*2 + 1] = (char)0xFF;
        if (disk_write(ctx, k++, 1, buffer))
            goto terminate;

        memset(buffer, 0, geom.bytes_per_sector);
        for (i = 1; i < newvolume.PB.FATSz16; i++)
            if (disk_write(ctx, k++, 1, buffer))
                goto terminate;
    }

    /* Data Region */

    /*
     * Create VOLUME ID file in the root directory
     */
    memset(buffer, 0, geom.bytes_per_sector);
    volumeid = (struct FAT *)buffer;
    volumeid->DIR_Attr = ATTR_VOLUME_ID;
    memset(volumeid->DIR_Name, ' ', sizeof(volumeid->DIR_Name));
    memcpy(volumeid->DIR_Name, newvolume.PB.VolLab, sizeof(newvolume.PB.VolLab));
    FAT_update_timestamps(volumeid, FAT_TS_CREATE);
    k = part->startsector + newvolume.PB.ResvdSecCnt;
    k += newvolume.PB.NumFATs*newvolume.PB.FATSz16;

    if (disk_write(ctx, k++, 1, buffer))
        goto terminate;

    /*
     * blank data area
     */
    memset(buffer, 0, geom.bytes_per_sector);
    while (k < part->endsector + 1)
        if (disk_write(ctx, k++, 1, buffer))
            goto terminate;

    free(buffer);
    return 0;

  terminate:
    if (buffer)
        free(buffer);

    return -1;
}
