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

#include "fat_internals.h"
#include "fat_print.h"

void FAT_print_PB(struct ParameterBlock *PB)
{
    printf("=========== Parameter Block ============\n");
    printf("+---------------------------------------\n");
    printf("| OEM ................... %8.8s\n", PB->OEMName);
    printf("| Reserved Sectors ...... %u\n", PB->ResvdSecCnt);
    printf("| Bytes per Sector ...... %u\n", PB->BytsPerSec);
    printf("| Sectors per Cluster ... %u\n", PB->SecPerClus);
    printf("| FAT number ............ %u\n", PB->NumFATs);
    printf("| Total sectors ......... %u\n", PB->TotSec);
    printf("| FAT size in sectors ... %u\n", PB->FATSz16);
    printf("| Volume ID ............. %8X\n", PB->VolID);
    printf("| Volume label .......... %16.16s\n", PB->VolLab);
    printf("| File System Type ...... %8.8s\n", PB->FilSysType);
    printf("+---------------------------------------\n");
}

void FAT_print_Volume(struct FATVolume *vol)
{
    printf("============ FAT16.1 Volume ============\n");
    printf("+---------------------------------------\n");
    printf("| Number of Clusters ...... %u\n", vol->CountOfClusters);
    printf("| Number of Sectors ....... %u\n", vol->DataSectors);
    printf("| Bytes per Cluster ....... %u\n", vol->BytesPerCluster);
    printf("| Data bytes .............. %uMB\n", vol->BytesPerCluster*vol->CountOfClusters/(1024*1024));
    printf("| Flags.................... ");
    if (vol->flags & FLAG_ACTIVE)
        printf("ACTIVE ");
    if (vol->flags & FLAG_MOUNTED)
        printf("MOUNTED ");
    putchar('\n');
    printf("+---------------------------------------\n");
}

void FAT_print_FAT(struct FATVolume *vol)
{
    uint16_t i = 0;
    printf("======================== FAT =========================\n");
    printf("------------------------------------------------------\n");
    while (i < vol->FATSize)
    {
        printf("0x%4.4X", FAT_get_cluster_from_table(vol, i));
        if ((++i % 16) == 0) putchar('\n');
        else putchar('|');
    }
    if ((i % 16)) putchar('\n');
    else putchar('|');
    printf("------------------------------------------------------\n");
}

void FAT_print_entry(struct FAT *entry)
{
    struct FAT_date_stamp *date;
    struct FAT_time_stamp *time;

    if (!entry ||
        (entry->DIR_Name[0] == DIR_ENTRY_LAST_FREE) ||
        (entry->DIR_Name[0] == (char)DIR_ENTRY_FREE))
        return;

    date = &entry->DIR_CrtDate;
    time = &entry->DIR_CrtTime;

    printf(" %02d/%02d/%4d, %02d:%02d:%02d",
           date->dayOfMonth,
           date->monthOfYear,
           date->years + 2000,
           time->hours,
           time->mins,
           time->secs);

    if (entry->DIR_Attr & ATTR_DIRECTORY)
        printf("    <DIR>        ");
    else
    {
        printf("    ");
        if (entry->DIR_Attr & ATTR_LINK)
            putchar('L');
        else
            putchar('.');
        if (entry->DIR_Attr & ATTR_READ_ONLY)
            putchar('R');
        else
            putchar('.');
        if (entry->DIR_Attr & ATTR_SYSTEM)
            putchar('S');
        else
            putchar('.');
        if (entry->DIR_Attr & ATTR_HIDDEN)
            putchar('H');
        else
            putchar('.');
        if (entry->DIR_Attr & ATTR_VOLUME_ID)
            putchar('V');
        else
            putchar('.');
        printf(" %10u ", entry->DIR_FileSize);
    }

    printf("%39.39s\n", entry->DIR_Name);
}

int FAT_print_loop(struct FATVolume *vol, struct FAT *buffer)
{
    unsigned i = 0;

    if (!vol || !buffer)
        return -1;

    for (i = 0; i < vol->FATEntriesPerSec*vol->PB.SecPerClus; i++, buffer++)
    {
        if (buffer->DIR_Name[0] == 0x00)
        {
            return -1;
        }
        FAT_print_entry(buffer);
    }

    return 0;
}
