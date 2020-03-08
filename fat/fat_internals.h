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

#ifndef FAT_INTERNALS_H
#define FAT_INTERNALS_H

#include "fat_data.h"

#define FAT_TS_CREATE 1
#define FAT_TS_WRITE  2
#define FAT_TS_READ   4

struct FAT_direntry_search_result
{
    /* Copy of the FAT entry */
    struct FAT entry;
    /* disk sector relative to the volume start */
    uint32_t sector;
    /* Offset of the entry in the sector */
    unsigned offset;
    /* First cluster of the directory containing this entry. */
    uint16_t container;
};

struct FAT_statistics
{
    uint32_t totbytes;
    uint32_t totfiles;
    uint32_t totlinks;
    uint32_t totdirs;
    uint32_t totavail;
};

/**
 * @brief FAT_FSoC computes the first sector on disk where the cluster N starts.
 * It converts clusters to starting sectors.
 *
 * @param vol volume structure
 * @param N cluster number
 *
 * @return sector number
 */
static inline uint32_t FAT_FSoC(struct FATVolume *vol, uint32_t N)
{
    return ((N - 2) * vol->PB.SecPerClus) + vol->FirstDataSector;
}

/**
 * @brief FAT_get_cluster_from_table returns the cluster values stored in the FAT table at position N
 *
 * @param vol volume structure
 * @param N FAT table index
 *
 * @return cluster number
 */
uint16_t FAT_get_cluster_from_table(struct FATVolume *vol, uint16_t N);

/**
 * @brief FAT_set_cluster_into_table write the value val into FAT table at cluster index N.
 *
 * @param vol volume structure
 * @param N FAT table index
 *
 * @return 0 on success
 * @return -1 on error
 */
int FAT_set_cluster_into_table(struct FATVolume *vol, uint16_t N, uint16_t val);

/**
 * @brief FAT_set_EOC_into_table writes the EOC value into FAT table at cluster index N.
 *
 * @param vol volume structure
 * @param N FAT table index
 *
 * @return 0 on success
 * @return -1 on error
 */
int FAT_set_EOC_into_table(struct FATVolume *vol, uint16_t N);

/**
 * @brief FAT_check_DIR_entry checks if a FAT entry is valid, void, or invalid.
 *
 * @param entry pointer to a FAT directory structure
 *
 * @return 1 if the FAT entry is a valid entry
 * @return 0 if the FAT entry is a void entry
 * @return -1 if the FAT entry is an invalid entry
 */
int FAT_check_DIR_entry(struct FAT *entry);

/**
 * @brief FAT_check_attrib checks if attributes are coherent and valid.
 *
 * @param attributes a bitmap of FAT directory entry attributes
 *
 * @return 0 if the FAT attributes are valid
 * @return -1 if the FAT attributes are not valid
 */
int FAT_check_attrib(uint8_t attributes);
/**
 * @brief FAT_get_free_cluster_from_table returns a free cluster number, or 0 if there are
 * no free clusters available.
 * NOTE: available clusters starts at index 2.
 *
 * @param vol volume structure
 *
 * @return cluster number on success, range is [2, FATSize]
 * @return 0 on failure
 */
uint16_t FAT_get_free_cluster_from_table(struct FATVolume *vol);

/**
 * @brief FAT_update_timestamps update the date and time stamps of a FAT directory structure.
 *
 * @param entry pointer to a FAT directory structure
 * @param flags a combination of FAT_TS_ flags to update the corresponding timestamp
 *
 * @return 0 on success
 * @return -1 on error
 */
int FAT_update_timestamps(struct FAT *entry, int flags);

/**
 * @brief FAT_update_statistics updates counters in stats parsing the contents of a cluster
 * pointed by buffer.
 * The buffer is seen as an array of FAT directory entries and must be exactly one cluster long,
 * the size of a cluster is defined at mount time.
 * Counters are updated only, i.e. new values are added to the values found in stats, to gather
 * statistics in a meaningful manner the FAT_statistics structure should be initialized to all 0s.
 *
 * @param vol volume structure
 * @param buffer memory buffer storing one cluster
 * @param stats pointer to statistics
 *
 * @return 0 on success
 * @return -1 on error
 */
int FAT_update_statistics(struct FATVolume *vol, struct FAT *buffer, struct FAT_statistics *stats);

#endif // FAT_INTERNALS_H
