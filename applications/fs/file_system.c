/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2026 Diego Gallizioli
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
#include <libs/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <diegos/kernel.h>
#include <diegos/devices.h>
#include <unistd.h>

#include "fs_core.h"

//FIXME move to some header file
// partition name is 
// controller name up to 5 characters including the unit number
// 'd' followed by the drive number 0 to 9
// 'p' followed by the partition number 0 to 9 (optional)
//
// example: "ide0d0p0" is the first partition of the first drive of the first IDE controller
// example: "sata1d2p3" is the fourth partition of the third drive of the second SATA controller
// example: "fdc0d0p0" is the first partition of the first drive of the first floppy controller  

#define PART_NAME_LEN (5 + 1 + 1 + 1 + 1)	// controller name (5) + 'd' (1) + drive number (1) + 'p' (1) + partition number (1)

typedef struct partitions {
	list_node header;
	device_t *dev;
	char name[PART_NAME_LEN + 1];
	uint8_t drive_no;
	uint8_t partition_no;
} partition_t;

enum {
	VOLUME_IS_MOUNTED = 0x01,
	VOLUME_IS_READONLY = 0x02,
	VOLUME_IS_DIRTY = 0x04,
	VOLUME_IS_VALID = 0x08
};

typedef struct volumes {
	list_node header;
	partition_t *part;
	int flags;
	char mount_letter;
} volume_t;

/*
 * A = (c x N_heads + h) x N_sectors + (s-1)
 *
 * Where A is the LBA logical address, c is the cylinder number,
 * h is the head number, s is the sector number, N_heads is the number of heads per cylinder,
 * and N_sectors is the number of sectors per track (i.e. on 1 platter only).
 * A cylinder is a set of tracks that are at the same distance from the spindle on all platters.
 */
static inline uint32_t lba_from_chs(uint16_t c, uint8_t h, uint8_t s, uint8_t heads_per_cylinder,
				    uint16_t sectors_per_track)
{
	return (uint32_t) ((c * heads_per_cylinder + h) * sectors_per_track + (s - 1));
}

//static LIST_STATIC_INIT(partitions_list);
//static LIST_STATIC_INIT(volumes_list);
//static char buffer[512*4] = { 0x34, 0x43, 0xFF, 0x99};

#if 0
static void test(device_t *dev)
{
	struct block_request br;
	int status = EOK;

//      char buffer[512] = { 0x34, 0x43, 0xFF, 0x99};
	br.start_block = 0;
	br.buffer = buffer;
	br.buffer_size = sizeof(buffer);
	br.bytes = 512 * 2;
	br.disk = 1;
	br.cmd = 0;		// READ
	br.errors = 0;
	while (status == EOK) {
		status = dev->bdrv->read_fn(&br, dev->header.unitno);
		printf("R %s ", errno2str(status));
		br.start_block += 2;
		printf("buffer %#X\n", *(uint32_t *) buffer);
		if (br.start_block > 2879)
			break;
	}

	sleep(2);

	memset(buffer, 0x00, sizeof(buffer));
	br.start_block = 0;
	br.buffer = buffer;
	br.buffer_size = sizeof(buffer);
	br.bytes = 512 * 2;
	br.disk = 1;
	br.cmd = 1;		// WRITE
	br.errors = 0;
	while (status == EOK) {
		status = dev->bdrv->write_fn(&br, dev->header.unitno);
		printf("W %s ", errno2str(status));
		br.start_block += 2;
		printf("buffer %#X\n", *(uint32_t *) buffer);
		memset(buffer, br.start_block, sizeof(buffer));
		if (br.start_block > 2879)
			break;
	}

	sleep(2);

	memset(buffer, 0x00, sizeof(buffer));
	br.start_block = 0;
	br.buffer = buffer;
	br.buffer_size = sizeof(buffer);
	br.bytes = 512 * 2;
	br.disk = 1;
	br.cmd = 0;		// READ
	br.errors = 0;
	while (status == EOK) {
		status = dev->bdrv->read_fn(&br, dev->header.unitno);
		printf("V %s ", errno2str(status));
		br.start_block += 2;
		printf("buffer %#X\n", *(uint32_t *) buffer);
		if (br.start_block > 2879)
			break;
	}
}
#endif

static void fs_core_main_entry(void)
{
	device_t *dev;
	uint64_t search;

	sleep(5);

	dev = device_get_first_of_type(&search, DEV_TYPE_BLOCK);

	/*
	 * Now loop and scan block devices for partitions.
	 * Keep only running devices.
	 */
	while (dev) {
		if (dev->cmn->status_fn(dev->header.unitno) & DRV_STATUS_RUN) {
			printf("Scanning block device %s for partitions...\n", dev->header.name);
			//test(dev);
		} else {
			printf("Skipping block device %s, not running.\n", dev->header.name);
		}
		dev = device_get_next(&search);
	}

	while (TRUE) {
		//thread_suspend();
		sleep(1);
	}
}

void filesystem_run()
{
	uint8_t pid;
	BOOL retcode;

	retcode = thread_create("fs", THREAD_PRIO_NORMAL, fs_core_main_entry, 0, 16 * 1024, &pid);

	assert(TRUE == retcode);
}
