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

#ifndef BPB_H
#define BPB_H

#include <stdint.h>

/*
 * BIOS Parameter Block is in the boot record found on FAT12/FAT16 volumes
 * or floppy disks sector 0 (boot sector).
 * This is used for booting ONLY.
 * The structure is packed as it must be exactly 512 bytes long, and the
 * structure must be consistent across different platforms.
 */
#pragma pack(1)
struct BPB {
	/*
	 * Usually JMP SHORT 0x3C or 0xEB 0x?? 0x90
	 */
	char BS_jmpBoot[3];
	/*
	 * OEM Name is merely an ASCII string, padded with spaces,
	 * not NULL terminated
	 */
	char BS_OEMName[8];
	/*
	 * Bytes per sector
	 */
	char BPB_BytsPerSec[2];
	/*
	 * Sectors per cluster
	 */
	char BPB_SecPerClus;
	/*
	 * Reserved sectors
	 */
	char BPB_RsvdSecCnt[2];
	/*
	 * Number of FATs in this volume
	 */
	char BPB_NumFATs;
	/*
	 * Root directory entries
	 */
	char BPB_RootEntCnt[2];
	/*
	 * Total sectors (16-bit)
	 */
	char BPB_TotSec16[2];
	/*
	 * Media type
	 */
	char BPB_Media;
	/*
	 * Size of each FAT (16-bit) is sectors
	 */
	char BPB_FATSz16[2];
	/*
	 * Sectors per track
	 */
	char BPB_SecPerTrk[2];
	/*
	 * Number of heads
	 */
	char BPB_NumHeads[2];
	/*
	 * Hidden sectors
	 */
	char BPB_HiddSec[4];
	/*
	 * Total sectors (32-bit)
	 */
	char BPB_TotSec32[4];
	/*
	 * Drive number
	 */
	char BS_DrvNum;
	/*
	 * Reserved
	 */
	char Reserved1;
	/*
	 * Extended boot signature (0x29 if the next three fields are valid)
	 */
	char BS_BootSig;
	/*
	 * Volume ID (serial number)
	 */
	char BS_VolID[4];
	/*
	 * Volume label, padded with spaces, not NULL terminated
	 */
	char BS_VolLab[11];
	/*
	 * File system type, padded with spaces, not NULL terminated
	 */
	char BS_FilSysType[8];
	/*
	 * Boot code, not used by the FAT file system, but can be used by a boot loader
	 */
	char bootcode[448];
	/*
	 * 0x55 - 0xAA
	 */
	char boot_signature[2];
};
#pragma pack(0)

/**
 * @brief BPB_read access the Boot Sector found on disk pointed by ctx.
 * ctx is a disk access context as returned by the disk access API.
 * On failure the contents of bpb are unchanged.
 * The boot sector of floppies is set to store BPB values, BIOS Parameter Block.
 *
 * PARAMETERS IN
 * @param ctx disk access API context
 *
 * PARAMETERS OUT
 * @param bpb pointer to a Boot Sector BPB data structure
 *
 * @return 0 on success
 * @return -1 on error
 */
int BPB_read(void *ctx, struct BPB *bpb);

/**
 * @brief BPB_write access and write the Boot Sector found on disk pointed by ctx.
 * ctx is a disk access context as returned by the disk access API.
 * On failure the contents of the bpb MIGHT BE PERMANENTLY CHANGED !!!
 * DANGER: CORRUPTING OR CHANGING THE BPB OF A DISK WILL RESULT IN A POSSIBLE DATA LOSS !!!
 *
 * PARAMETERS IN
 * @param ctx disk access API context
 * @param bpb pointer to a Boot Sector BPB data structure
 *
 * @return 0 on success
 * @return -1 on error
 */
int BPB_write(void *ctx, const struct BPB *bpb);

#endif
