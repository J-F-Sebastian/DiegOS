/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2019 - 2026 Diego Gallizioli
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

#ifndef MBR_DATA_H
#define MBR_DATA_H

#include <stdint.h>

/*
 * Master Boot Record occupies the first sector of a hard disk.
 * It ends with a table carrying 4 rows, each row describing a partition
 * on the disk.
 * A partition is a set of blocks (sectors) on disk.
 */
#pragma pack(push, 1)
struct MBR_partition_entry {
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
	unsigned char CHS_first_sector[3];
	char type;
	unsigned char CHS_last_sector[3];
	uint32_t LBA_first_sector;
	uint32_t num_of_sectors;
};

struct MBR {
	char bootcode[446];
	struct MBR_partition_entry partitions[4];
	/*
	 * 0x55 - 0xAA
	 */
	char boot_signature[2];
};
#pragma pack(pop)

#endif
