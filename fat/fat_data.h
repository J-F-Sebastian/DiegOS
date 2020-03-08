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

#ifndef FAT_DATA_H
#define FAT_DATA_H

#include <stdint.h>

/*
 * A FAT16.1 file system volume is composed of 3 basic regions,
 * which are laid out in this order on the volume:
 *
 * 0 – Reserved Region
 * 1 – FAT Region
 * 2 – File and Directory Data Region
 */

/*
 * +----------------------------------------------------------------------------+
 * |                                                                            |
 * |                                RESERVED                                    |
 * |                                                                            |
 * +----------------------------------------------------------------------------+
 * |                                                                            |
 * |                                FAT 0                                       |
 * |                                                                            |
 * +----------------------------------------------------------------------------+
 * |                                                                            |
 * |                                FAT 1                                       |
 * |                                                                            |
 * +----------------------------------------------------------------------------+
 *                                  ....
 *                                  ....
 * +----------------------------------------------------------------------------+
 * |                                                                            |
 * |                                FAT N                                       |
 * |                                                                            |
 * +----------------------------------------------------------------------------+
 * |                                                                            |
 * |                                DATA                                        |
 * |                                                                            |
 * +----------------------------------------------------------------------------+
 *
 */

#pragma pack(1)
struct ParameterBlock
{
    char     OEMName[8];
    uint16_t BytsPerSec;
    uint8_t  SecPerClus;
    uint8_t  NumFATs;
    uint16_t ResvdSecCnt;      
    uint16_t FATSz16;
    /* 16 bytes */
    uint32_t HiddSec;
    uint32_t TotSec;
    uint32_t VolID;
    uint32_t Flags;
    /* 32 bytes */
    char     VolLab[16];
    /* 48 bytes */
    char     FilSysType[8];
    /* 56 bytes */
    uint16_t RootClus;
    /* 58 bytes */
};

/*
 * Date and time are relative to Midnight 1/1/2000
 */
struct FAT_date_stamp
{
    uint16_t dayOfMonth:5;
    uint16_t monthOfYear:4;
    /* number of years since 2000 */
    uint16_t years:7;
};

struct FAT_time_stamp
{
    uint32_t msecs:10;
    uint32_t secs:6;
    uint32_t mins:6;
    uint32_t hours:5;
};

#define DIR_ENTRY_FREE      (0xE5)
#define DIR_ENTRY_LAST_FREE (0x00)

#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_LINK       0x20
/* Update this define if you change the definition of struct FAT !!! */
/* Update the same define in fat_names.c if you change the definition of struct FAT !!! */
#define DIR_NAMELEN     39
struct FAT
{
    char                    DIR_Name[39];
    uint8_t                 DIR_Attr;
    /* 40 Bytes */
    uint16_t                DIR_FstClus;
    struct FAT_date_stamp   DIR_AccDate;
    struct FAT_date_stamp   DIR_CrtDate;
    struct FAT_date_stamp   DIR_WrtDate;
    /* 48 bytes */
    struct FAT_time_stamp   DIR_AccTime;
    struct FAT_time_stamp   DIR_CrtTime;
    struct FAT_time_stamp   DIR_WrtTime;
    /* 60 Bytes */
    uint32_t                DIR_FileSize;
    /* 64 bytes */
};
#pragma pack()

#define FLAG_MOUNTED    0x1
#define FLAG_ACTIVE     0x2

struct FATVolume
{
    struct ParameterBlock   PB;
    /* in-memory copy of the FAT */
    uint16_t               *FAT;
    /* in-memory buffer to read or write clusters - and sectors */
    char                   *buffer;
    /*
     * FirstSector is the LBA address of the sector storing the PB, which is considered
     * the first sector of the volume.
     */
    uint32_t                FirstSector;
    /*
     * FirstDataSector is relative to the sector storing the PB, which is considered
     * as sector 0.
     */
    uint32_t                FirstDataSector;
    /* Number of Clusters in this volume */
    uint16_t                CountOfClusters;
    /* Number of sectors in the Data Region */
    uint32_t                DataSectors;
    /* Bytes in a Cluster */
    uint16_t                BytesPerCluster;
    /* Number of entries in the FAT table */
    uint16_t                FATSize;
    /* Number of FAT directory entries per sector */
    unsigned                FATEntriesPerSec;
    /* Volume flags, MOUNTED ACTIVE */
    int                     flags;
    /* Opaque I/O context provided by disk_access API */
    void                   *ctx;
    /* Last free FAT entry to speed up free clusters search */
    uint16_t                lastFreeFATEntry;
};

#endif // FAT_DATA_H
