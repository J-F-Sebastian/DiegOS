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
 */
#pragma pack(1)
struct BPB {
	/*
	 * Usually JMP SHORT 0x3C
	 */
	char BS_jmpBoot[3];
	char BS_OEMName[8];
	char BPB_BytsPerSec[2];
	char BPB_SecPerClus;
	char BPB_RsvdSecCnt[2];
	char BPB_NumFATs;
	char BPB_RootEntCnt[2];
	char BPB_TotSec16[2];
	char BPB_Media;
	char BPB_FATSz16[2];
	char BPB_SecPerTrk[2];
	char BPB_NumHeads[2];
	char BPB_HiddSec[4];
	char BPB_TotSec32[4];
	char BS_DrvNum;
	char Reserved1;
	char BS_BootSig;
	char BS_VolID[4];
	char BS_VolLab[11];
	char BS_FilSysType[8];
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
 * @param ctx disk access API context
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
 * @param ctx disk access API context
 * @param bpb pointer to a Boot Sector BPB data structure
 *
 * @return 0 on success
 * @return -1 on error
 */
int BPB_write(void *ctx, struct BPB *bpb);

#endif				// BPB_H
