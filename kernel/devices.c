/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2015 Diego Gallizioli
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
#include <list.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <diegos/kernel.h>
#include <diegos/devices.h>

#include "kprintf.h"
#include "devices_private.h"

typedef struct device_int {
	list_node header;
	device_t dv;
} device_int_t;

static LIST_STATIC_INIT(device_list);

static device_int_t *lookup_name(const char *name)
{
	device_int_t *nn = list_head(&device_list);
	int retval;

	while (nn) {
		retval = strncmp(nn->dv.name, name, sizeof(nn->dv.name));

		/*
		 * Device list is sorted.
		 * If retval is negative, name is greater than the nn device name.
		 * If retval is positive, we went beyond the search limit, return NULL.
		 * If retval is 0, we found the correct device
		 */
		if (retval < 0) {
			nn = (device_int_t *) nn->header.next;
		} else if (0 == retval) {
			return (nn);
		} else {
			return (NULL);
		}
	}

	return (NULL);
}

static STATUS insert_device(device_int_t *dev)
{
	device_int_t *prev = list_head(&device_list);
	int retval;

	while (prev) {
		retval = strncmp(prev->dv.name, dev->dv.name, sizeof(prev->dv.name));

		if (retval < 0) {
			prev = (device_int_t *) prev->header.next;
		} else {
			return (list_add(&device_list, prev->header.prev, &dev->header));
		}
	}

	/*
	 * If the loop exited, we need to add the device at the end
	 */
	return (list_add(&device_list, list_tail(&device_list), &dev->header));
}

BOOL init_devices_lib()
{
	return (TRUE);
}

void start_devices_lib()
{
	device_int_t *ptr = list_head(&device_list);

	/*
	 * Now loop and start drivers.
	 */
	while (ptr) {
		if (ptr->dv.flags & DEV_FLAG_CHAR) {
			if (ptr->dv.drv->start_fn(ptr->dv.unit)) {
				kerrprintf("Failed starting %s\n", ptr->dv.name);
			}
		}
		ptr = (device_int_t *) ptr->header.next;
	}

	kmsgprintf("All devices started\n");
}
/*
 * Public section
 */
device_t *device_create(unsigned unit, char_driver_t *inst)
{
	device_int_t *tmp;
	char temp[DEV_NAME_LEN + 1];

	if (!inst) {
		return (NULL);
	}

	snprintf(temp, sizeof(temp), "%.5s%1u", inst->name, unit);
	tmp = lookup_name(temp);

	if (tmp) {
		return (NULL);
	}

	tmp = malloc(sizeof(*tmp));

	if (!tmp) {
		return (NULL);
	}

	memset(tmp->dv.name, 0, sizeof(tmp->dv.name));
	memcpy(tmp->dv.name, temp, sizeof(tmp->dv.name));
	tmp->dv.unit = unit;
	tmp->dv.flags = DEV_FLAG_CHAR;
	tmp->dv.drv = inst;

	if (EOK != insert_device(tmp)) {
		free(tmp);
		return (NULL);
	}

	kmsgprintf("Created device %s\n", temp);

	return (&tmp->dv);
}

device_t *device_lookup(const char *drvname, unsigned unit)
{
	char temp[DEV_NAME_LEN + 1];
	device_int_t *retval;

	if (!drvname) {
		errno = EINVAL;
		return (NULL);
	}

	snprintf(temp, sizeof(temp), "%.5s%1u", drvname, unit);

	retval = lookup_name(temp);

	return ((retval) ? (&retval->dv) : (NULL));
}

device_t *device_lookup_name(const char *devname)
{
	device_int_t *retval;

	if (!devname) {
		errno = EINVAL;
		return (NULL);
	}

	retval = lookup_name(devname);

	return ((retval) ? (&retval->dv) : (NULL));
}

int device_io_tx(device_t *dev, const char *buf, size_t bytes)
{
	int retcode = bytes;
	size_t cbytes = 0;

	if (!dev || !dev->drv || !buf) {
		return (EINVAL);
	}

	while (bytes > 0) {
		retcode = dev->drv->write_fn(buf, bytes, dev->unit);
		if (retcode > 0) {
			buf += retcode;
			bytes -= retcode;
			cbytes += retcode;
		} else {
			return (EIO);
		}
	}

	return (cbytes);
}

int device_io_rx(device_t *dev, char *buf, size_t bytes)
{
	int retcode;

	if (!dev || !dev->drv || !buf) {
		return (EINVAL);
	}

	retcode = dev->drv->read_fn(buf, bytes, dev->unit);

	return ((retcode >= 0) ? (retcode) : (EIO));
}
