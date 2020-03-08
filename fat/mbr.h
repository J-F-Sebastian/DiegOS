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

#ifndef MBR_H
#define MBR_H

#include <stdint.h>

/* FAT16.1 File System */
#define PART_FAT16_1 0x0D
/* Alternative OS Development Partition Standard */
#define PART_AOPDS   0x7F

#pragma pack(1)
struct MBR_partition_entry
{
    char status;
    /*
     * Bits
     * 0-7 Heads
     * 0-5 Sectors, 6-7 Cylinders (High 2 bits)
     * 0-7 Cylinders
     *
     * Ranges
     * Heads 0 - 255
     * Sectors 1 - 64
     * Cylinders 0 -1023
     */
    char CHS_first_sector[3];
    char type;
    char CHS_last_sector[3];
    uint32_t LBA_first_Sector;
    uint32_t num_of_sectors;
};

struct MBR
{
    char bootcode[446];
    struct MBR_partition_entry partitions[4];
    /*
     * 0x55 - 0xAA
     */
    char boot_signature[2];
};
#pragma pack(0)

/**
 * @brief MBR_read access the Master Boot Record found on disk pointed by ctx.
 * ctx is a disk access context as returned by the disk access API.
 * On failure the contents of mbr are unchanged.
 *
 * @param ctx disk access API context
 * @param mbr pointer to a Master Boot Record data structure
 *
 * @return 0 on success
 * @return -1 on error
 */
int MBR_read(void *ctx, struct MBR *mbr);

/**
 * @brief MBR_write access adn write the Master Boot Record found on disk pointed by ctx.
 * ctx is a disk access context as returned by the disk access API.
 * On failure the contents of mbr MIGHT BE PERMANENTLY CHANGED !!!
 * DANGER: CORRUPTING OR CHANGING THE MBR OF A DISK WILL RESULT IN A POSSIBLE DATA LOSS !!!
 *
 * @param ctx disk access API context
 * @param mbr pointer to a Master Boot Record data structure
 *
 * @return 0 on success
 * @return -1 on error
 */
int MBR_write(void *ctx, struct MBR *mbr);

/**
 * @brief MBR_get_active_partition returns the index of the partition in the MBR with the Active
 * Partition flags set. If no partition is flagged as Active, the function returns -1.
 *
 * @param mbr pointer to a Master Boot Record data structure
 *
 * @return a partition index with range [0, 3] on success
 * @return -1 on error
 */
int MBR_get_active_partition(struct MBR *mbr);

/**
 * @brief MBR_get_partition_entry copy the partition with index partum into entry.
 * On failure the contents of entry are unchanged.
 *
 * @param mbr pointer to a Master Boot Record data structure
 * @param partnum partition index [0..3]
 * @param entry pointer to a Master Boot Record partition entry data structure
 * @return
 */
int MBR_get_partition_entry(struct MBR *mbr, int partnum, struct MBR_partition_entry *entry);

/**
 * @brief MBR_list prints the Master Boot Record partition table found in mbr.
 * Partition marked with a star '*' is active.
 *
 * @param mbr pointer to a Master Boot Record data structure
 */
void MBR_list(struct MBR *mbr);

#endif // MBR_H
