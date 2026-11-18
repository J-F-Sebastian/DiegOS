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
#ifdef _MKIMAGE_
#include "bpb_data.h"
#else
#include <fs/bpb_data.h>
#endif

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
