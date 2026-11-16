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
#include <fs/fs.h>
#include <unistd.h>

#include "fs_core.h"

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
		printf("Scanning block device %s for partitions...\n", dev->header.name);
		fs_search_for_partitions(dev, 0);
		fs_search_for_partitions(dev, 1);
		fs_search_for_partitions(dev, 2);
		fs_search_for_partitions(dev, 3);
		dev = device_get_next(&search);
	}

	//test();

	while (TRUE) {
		// thread_suspend();
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
