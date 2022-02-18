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

#ifndef FAT_PRINT_H
#define FAT_PRINT_H

#include "fat_data.h"

/**
 * @brief FAT_print_PB print to standard output the contents of the Parameter Block
 * pointed to by PB
 *
 * @param PB pointer to a ParameterBlock structure
 */
void FAT_print_PB(struct ParameterBlock *PB);

/**
 * @brief FAT_print_Volume print to standard output the properties of the FAT Volume
 * pointed to by vol
 *
 * @param vol pointer to a FATVolume structure
 */
void FAT_print_Volume(struct FATVolume *vol);

/**
 * @brief FAT_print_FAT dump to standard output the contents of the FAT table
 *
 * @param vol pointer to a FATVolume structure
 */
void FAT_print_FAT(struct FATVolume *vol);

/**
 * @brief FAT_print_entry prints to standard output the contents of the FAT directory entry
 * pointed by entry.
 *
 * @param entry pointer to a FAT directory structure
 */
void FAT_print_entry(struct FAT *entry);

/**
 * @brief FAT_print_loop prints to standard output the contents of a cluster pointed by buffer.
 * The cluster is seen as an array of FAT directory entries and must be exactly one cluster long,
 * the size of a cluster is defined at mount time.
 * This function is obviously used to list to video the contents of a directory.
 *
 * @param vol pointer to a FATVolume structure
 * @param buffer pointer to a cluster-wide buffer of FAT directory structures
 * @return 0 on success
 * @return -1 if a null entry is encountered before the end of the cluster
 */
int FAT_print_loop(struct FATVolume *vol, struct FAT *buffer);

#endif				// FAT_PRINT_H
