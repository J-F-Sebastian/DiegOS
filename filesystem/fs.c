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
#include <diegos/kernel.h>
#include <fs/fs.h>
#include <fs/bpb_data.h>
#include <fs/mbr_data.h>

/* FAT16.1 File System */
#define PART_FAT16_1 0x0D
/* Alternative OS Development Partition Standard */
#define PART_AOPDS 0x7F

// FIXME move to some header file
//  partition name is
//  controller name up to 5 characters including the unit number
//  'd' followed by the drive number 0 to 9
//  'p' followed by the partition number 0 to 9 (optional)
//
//  example: "ide0d0p0" is the first partition of the first drive of the first IDE controller
//  example: "sata1d2p3" is the fourth partition of the third drive of the second SATA controller
//  example: "fdc0d0p0" is the first partition of the first drive of the first floppy controller

#define PART_NAME_LEN (DRV_NAME_LEN + 1 + 1 + 1 + 1)	// controller name (11) + 'd' (1) + drive number (1) + 'p' (1) + partition number (1)

typedef struct partitions {
	list_node header;
	device_t *dev;
	char name[PART_NAME_LEN + 1];
	uint8_t drive_no;
	uint8_t partition_no;
	uint64_t start_sector;
	uint64_t end_sector;
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

static LIST_STATIC_INIT(partitions_list);
// static LIST_STATIC_INIT(volumes_list);

#if 0
static void test(void)
{
	int status = EOK;
	int retcode;
	partition_t *part = list_head(&partitions_list);
	device_t *dev = part->dev;
	static char buffer[512], compare[512];
	unsigned i = 0;
	unsigned j;

	while (i < 2880) {
		status = device_io_rq_lock(dev);
		if (status != EOK) {
			printf("%s ERROR 1 sector %d\n", __FUNCTION__, i);
			return;
		}

		for (j = 0; j < 8; j++, i++) {
		memset(buffer, (0x21 + i) & 0xFF, sizeof(buffer));
		status = device_io_rq_tx(dev, buffer, sizeof(buffer), i, 1);
		if (status != EOK) {
			device_io_rq_unlock(dev);
			printf("%s ERROR 2 sector %d\n", __FUNCTION__, i);
			return;
		}
		status = device_io_rq_rx(dev, compare, sizeof(compare), i, 1);
		if (status != EOK) {
			device_io_rq_unlock(dev);
			printf("%s ERROR 3 sector %d\n", __FUNCTION__, i);
			return;
		}
	}

		status = device_io_rq_commit(dev, &retcode);

		if (status != EOK) {
			printf("%s ERROR 4 sector %d\n", __FUNCTION__, i);
		}

		if (retcode != EOK) {
			printf("%s ERROR 5 sector %d\n", __FUNCTION__, i);
		}

		status = device_io_rq_unlock(dev);

		if (status != EOK) {
			printf("%s ERROR 6 sector %d\n", __FUNCTION__, i);
		}

		if (memcmp(buffer, compare, sizeof(compare))) {
			printf("%s ERROR 7 sector %d\n", __FUNCTION__, i);
		}
	}
}
#endif

int fs_search_for_partitions(device_t *dev, unsigned dsk)
{
	int status = EOK;
	int retcode;
	partition_t *part;
	unsigned part_counter = 0, i;
	char buffer[512];
	struct BPB *pbpb;
	struct MBR *pmbr;

	status = device_io_rq_lock(dev);
	if (status != EOK) {
		return (status);
	}

	status = device_io_rq_rx(dev, buffer, sizeof(buffer), 0, dsk);
	if (status != EOK) {
		device_io_rq_unlock(dev);
		return (status);
	}

	status = device_io_rq_commit(dev, &retcode);
	device_io_rq_unlock(dev);

	if (status != EOK || retcode != EOK) {
		return (status == EOK) ? (retcode) : (status);
	}

	/*
	 * Look for MBR or a boot floppy
	 */
	pbpb = (struct BPB *)buffer;
	if ((pbpb->boot_signature[0] == (char)0x55) &&
	    (pbpb->boot_signature[1] == (char)0xAA) &&
	    (strncmp(pbpb->BS_FilSysType, "FAT16.1 ", sizeof(pbpb->BS_FilSysType)) == 0)) {
		/* Floppy disk image with FAT16.1 file system */
		part = malloc(sizeof(*part));
		if (!part) {
			return (ENOMEM);
		}

		part->dev = dev;
		part->drive_no = dsk;
		part->partition_no = part_counter++;
		part->start_sector = 0;
		part->end_sector = dev->bdrv->block_count - 1;
		snprintf(part->name, sizeof(part->name), "%sd%dp%d", dev->header.name,
			 part->drive_no, part->partition_no);

		if (EOK != list_append(&partitions_list, &part->header)) {
			free(part);
			return (EPERM);
		}
		printf("Add %s file system is %8.8s OS is %8.8s\n", part->name,
		       pbpb->BS_FilSysType, pbpb->BS_OEMName);
	} else {
		/*
		 * parse MBR
		 */
		pmbr = (struct MBR *)buffer;
		if ((pmbr->boot_signature[0] == (char)0x55) &&
		    (pmbr->boot_signature[1] == (char)0xAA)) {
			for (i = 0; i < NELEMENTS(pmbr->partitions); i++) {

				part = malloc(sizeof(*part));
				if (!part) {
					return (ENOMEM);
				}

				if (pmbr->partitions[i].type == PART_FAT16_1) {
					/* FAT16.1 partition */
					part->dev = dev;
					part->drive_no = dsk;
					part->partition_no = part_counter++;
					part->start_sector = pmbr->partitions[i].LBA_first_sector;
					part->end_sector =
					    part->start_sector +
					    pmbr->partitions[i].num_of_sectors - 1;
					snprintf(part->name, sizeof(part->name), "%sd%dp%d",
						 dev->header.name, part->drive_no,
						 part->partition_no);

					if (EOK != list_append(&partitions_list, &part->header)) {
						free(part);
						return (EPERM);
					}

					printf("Add %s file system is %8.8s", part->name,
					       "FAT16.1");
					if (pmbr->partitions[i].status & 0x80) {
						printf(" and ACTIVE\n");
					} else
						putchar('\n');
				} else if (pmbr->partitions[i].type == PART_AOPDS) {
					/* DiegOS boot partition */
					part->dev = dev;
					part->drive_no = dsk;
					part->partition_no = part_counter++;
					part->start_sector = pmbr->partitions[i].LBA_first_sector;
					part->end_sector =
					    part->start_sector +
					    pmbr->partitions[i].num_of_sectors - 1;
					snprintf(part->name, sizeof(part->name), "%sd%dp%d",
						 dev->header.name, part->drive_no,
						 part->partition_no);

					if (EOK != list_append(&partitions_list, &part->header)) {
						free(part);
						return (EPERM);
					}

					printf("Add %s DiegOS", part->name);
					if (pmbr->partitions[i].status & 0x80) {
						printf(" boot partition\n");
					} else
						putchar('\n');
				}
			}
		}
	}

	return 0;
}
