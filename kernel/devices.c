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
		retval = strncmp(nn->dv.header.name, name, sizeof(nn->dv.header.name));

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

static STATUS insert_device(device_int_t * dev)
{
	device_int_t *prev = list_head(&device_list);
	int retval;

	while (prev) {
		retval = strncmp(prev->dv.header.name,
				 dev->dv.header.name, sizeof(prev->dv.header.name));

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
	int ui = FALSE;
	unsigned counter = 0;

	/*
	 * Now loop and start drivers.
	 */
	while (ptr) {
		switch (ptr->dv.header.type) {
		case DEV_TYPE_CHAR:
			if (ptr->dv.cmn->start_fn(ptr->dv.header.unitno)) {
				kerrprintf("Failed starting %s\n", ptr->dv.header.name);
			} else {
				counter++;
			}
			break;

		case DEV_TYPE_BLOCK:
			kerrprintf("BLOCK devices not supported yet.\n");
			kerrprintf("Skipped starting %s\n", ptr->dv.header.name);
			break;

		case DEV_TYPE_TXT_UI:
			/* FALLTHRU */
		case DEV_TYPE_GFX_UI:
			if (ui) {
				kerrprintf("Multiple UI not supported for %s\n",
					   ptr->dv.header.name);
				kerrprintf("Skipped starting %s\n", ptr->dv.header.name);
			} else {
				if (ptr->dv.cmn->start_fn(ptr->dv.header.unitno)) {
					kerrprintf("Failed starting %s\n", ptr->dv.header.name);
				} else {
					ui = TRUE;
					counter++;
				}
			}
			break;

		default:
			kerrprintf("Unknown device type for %s\n", ptr->dv.header.name);
			break;
		}
		ptr = (device_int_t *) ptr->header.next;
	}

	kmsgprintf("%u devices started\n", counter);
}

/*
 * Public section
 */
device_t *device_create(const void *inst, unsigned unitno)
{
	device_int_t *tmp;
	driver_header_t *cmn = (driver_header_t *) inst;
	char temp[DEV_NAME_LEN + 1];
	unsigned type;
	unsigned drvtype;
	int retcode;

	if (!inst) {
		return (NULL);
	}

	retcode = cmn->init_fn(unitno);
	if (EOK != retcode) {
		if (ENXIO != retcode) {
			kerrprintf("Driver %s[%u] failed to init with error %d\n", cmn->name,
				   unitno, retcode);
		} else {
			kerrprintf("Driver %s does not support unit %u.\n", cmn->name, unitno);
		}
		return (NULL);
	}

	drvtype = cmn->status_fn(unitno);
	drvtype &= (DRV_IS_CHAR | DRV_IS_BLOCK | DRV_IS_NET | DRV_IS_TXT_UI | DRV_IS_GFX_UI);

	switch (drvtype) {
	case DRV_IS_NET:
		/* FALLTRHU */
	case DRV_IS_CHAR:
		type = DEV_TYPE_CHAR;
		break;

	case DRV_IS_BLOCK:
		type = DEV_TYPE_BLOCK;
		break;

	case DRV_IS_TXT_UI:
		type = DEV_TYPE_TXT_UI;
		break;

	case DRV_IS_GFX_UI:
		type = DEV_TYPE_GFX_UI;
		break;

	default:
		kerrprintf("Unknown driver type for %s or bad type format %#x\n",
			   cmn->name, drvtype);
		return (NULL);
	}

	snprintf(temp, sizeof(temp), "%.5s%1u", cmn->name, unitno);
	tmp = lookup_name(temp);

	if (tmp) {
		kerrprintf("Device %s already in list, skipping ...\n", temp);
		return (NULL);
	}

	tmp = calloc(sizeof(*tmp), 1);

	if (!tmp) {
		kerrprintf("Out of memory for device %s\n", temp);
		return (NULL);
	}

	memcpy(tmp->dv.header.name, temp, sizeof(tmp->dv.header.name));
	tmp->dv.header.unitno = unitno;
	tmp->dv.header.type = type;
	tmp->dv.drv = (void *)inst;

	if (EOK != insert_device(tmp)) {
		kerrprintf("Device %s failed list insertion\n", temp);
		free(tmp);
		return (NULL);
	}

	kmsgprintf("Created device %s\n", temp);

	return (&tmp->dv);
}

int device_set_access(device_t * dev, device_write_fn wfn, device_read_fn rfn)
{
	if (!dev) {
		return (EINVAL);
	}

	if (wfn && dev->header.write_fn) {
		return (EINVAL);
	}

	if (rfn && dev->header.read_fn) {
		return (EINVAL);
	}

	dev->header.write_fn = wfn;
	dev->header.read_fn = rfn;

	return (EOK);
}

device_t *device_lookup(const char *drvname, unsigned unitno)
{
	char temp[DEV_NAME_LEN + 1];
	device_int_t *retval;

	if (!drvname) {
		errno = EINVAL;
		return (NULL);
	}

	snprintf(temp, sizeof(temp), "%.5s%1u", drvname, unitno);

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

int device_io_tx(device_t * dev, const char *buf, size_t bytes)
{
	int retcode = bytes;
	size_t cbytes = 0;

	if (!dev || !dev->drv || !buf) {
		return (EINVAL);
	}

	if (dev->header.type != DEV_TYPE_CHAR) {
		return (EPERM);
	}

	/*
	 * The two loops are identical except for the function call
	 * servicing the write operation.
	 * They are split - though redundant - for performance reasons.
	 */
	if (dev->header.write_fn) {
		retcode = dev->header.write_fn(buf, bytes, dev->cdrv, dev->header.unitno);
		if (retcode > 0) {
			return (retcode);
		} else {
			return (EIO);
		}
	} else {
		retcode = dev->cdrv->write_fn(buf, bytes, dev->header.unitno);
		if (retcode > 0) {
			return (retcode);
		} else {
			return (EIO);
		}
	}

	return (cbytes);
}

int device_io_rx(device_t * dev, char *buf, size_t bytes)
{
	int retcode;

	if (!dev || !dev->drv || !buf) {
		return (EINVAL);
	}

	if (dev->header.type != DEV_TYPE_CHAR) {
		return (EPERM);
	}

	if (dev->header.read_fn) {
		retcode = dev->header.read_fn(buf, bytes, dev->cdrv, dev->header.unitno);
	} else {
		retcode = dev->cdrv->read_fn(buf, bytes, dev->header.unitno);
	}

	return ((retcode >= 0) ? (retcode) : (EIO));
}

int device_poll(device_t *dev, poll_table_t *table, short *events)
{
	if (!dev || !dev->drv || !events) {
		return (EINVAL);
	}

	if (dev->cmn->poll_fn) {
		*events = dev->cmn->poll_fn(dev->header.unitno, table);
	} else {
		*events = 0;
	}

	return (EOK);
}
