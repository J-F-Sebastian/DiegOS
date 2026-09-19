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
#include <libs/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <diegos/kernel.h>
#include <diegos/devices.h>
#include <diegos/mutexes.h>

#include "kprintf.h"
#include "devices_private.h"

#define BLKRQMAXSIZE 16

struct blk_rq {
	struct block_request *rq[BLKRQMAXSIZE];
	unsigned rq_size;
	mutex_t *rq_mutex;
};

typedef struct device_int {
	list_node header;
	device_t dv;
	/*
	 * Alloc for block devices only, used to call R/W and R/W multi
	 */
	struct blk_rq *blkrq;
} device_int_t;

struct device_search {
	device_int_t *current;
	unsigned type;
};

static LIST_STATIC_INIT(device_list);

static unsigned devices_count[DEV_TYPE_MAX] = { 0 };

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

static STATUS insert_device(device_int_t *dev)
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
	return (list_append(&device_list, &dev->header));
}

BOOL init_devices_lib()
{
	return (TRUE);
}

void start_devices_lib()
{
	device_int_t *ptr = list_head(&device_list);
	static int ui = FALSE;
	unsigned counter = 0;

	/*
	 * Now loop and start drivers.
	 */
	while (ptr) {
		switch (ptr->dv.header.type) {
		case DEV_TYPE_CHAR:
			/* FALLTHRU */
		case DEV_TYPE_BLOCK:
			if (ptr->dv.cmn->start_fn(ptr->dv.header.unitno)) {
				kerrprintf("Failed starting %s\n", ptr->dv.header.name);
			} else {
				counter++;
			}
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

	tmp = calloc(1, sizeof(*tmp));

	if (!tmp) {
		kerrprintf("Out of memory for device %s\n", temp);
		return (NULL);
	}

	memcpy(tmp->dv.header.name, temp, sizeof(tmp->dv.header.name));
	tmp->dv.header.unitno = unitno;
	tmp->dv.header.type = type;
	tmp->dv.drv = (void *)inst;

	if (type == DEV_TYPE_BLOCK) {
		tmp->blkrq = calloc(1, sizeof(*tmp->blkrq));
		if (!tmp->blkrq) {
			kerrprintf("Out of memory for device %s\n", temp);
			free(tmp);
			return (NULL);
		}

		tmp->blkrq->rq_mutex = thread_create_mutex(NULL);
		if (!tmp->blkrq->rq_mutex) {
			kerrprintf("Failed to initialize mutex for device %s\n", temp);
			free(tmp->blkrq);
			free(tmp);
			return (NULL);
		}
	}

	if (EOK != insert_device(tmp)) {
		kerrprintf("Device %s failed list insertion\n", temp);
		free(tmp);
		return (NULL);
	}

	kmsgprintf("Created device %s\n", temp);

	devices_count[type]++;

	return (&tmp->dv);
}

int device_set_access(device_t *dev, device_write_fn wfn, device_read_fn rfn)
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

device_t *device_get_first(uint64_t *search)
{
	device_int_t *retval;
	struct device_search *search_ptr = (struct device_search *)search;

	if (!search) {
		errno = EINVAL;
		return (NULL);
	}

	retval = list_head(&device_list);

	search_ptr->current = retval;
	search_ptr->type = DEV_TYPE_MAX;

	return ((retval) ? (&retval->dv) : (NULL));
}

device_t *device_get_first_of_type(uint64_t *search, unsigned type)
{
	device_int_t *retval;
	struct device_search *search_ptr = (struct device_search *)search;

	if (!search) {
		errno = EINVAL;
		return (NULL);
	}

	if (type >= DEV_TYPE_MAX) {
		errno = EINVAL;
		return (NULL);
	}

	retval = list_head(&device_list);

	if (type == DEV_TYPE_MAX) {
		search_ptr->current = retval;
		search_ptr->type = type;
		return ((retval) ? (&retval->dv) : (NULL));
	}

	while (retval) {
		if (retval->dv.header.type == type) {
			search_ptr->current = retval;
			search_ptr->type = type;
			return (&retval->dv);
		}
		retval = (device_int_t *) retval->header.next;
	}

	search_ptr->current = NULL;
	search_ptr->type = type;

	return (NULL);
}

device_t *device_get_next(uint64_t *search)
{
	device_int_t *retval;
	struct device_search *search_ptr = (struct device_search *)search;

	if (!search) {
		errno = EINVAL;
		return (NULL);
	}

	if (search_ptr->current == NULL) {
		return (NULL);
	}

	if (search_ptr->type != DEV_TYPE_MAX) {
		while (search_ptr->current) {
			retval = (device_int_t *) search_ptr->current->header.next;
			if (!retval) {
				search_ptr->current = NULL;
				return (NULL);
			}
			if (retval->dv.header.type == search_ptr->type) {
				search_ptr->current = retval;
				return (&retval->dv);
			}
			search_ptr->current = retval;
		}
		return (NULL);
	} else {
		retval = (device_int_t *) search_ptr->current->header.next;
		search_ptr->current = retval;
		return ((retval) ? (&retval->dv) : (NULL));
	}
}

unsigned device_get_count(unsigned type)
{
	if (type > DEV_TYPE_MAX) {
		errno = EINVAL;
		return (0);
	}

	return ((type < DEV_TYPE_MAX) ? devices_count[type] : list_count(&device_list));
}

int device_io_tx(device_t *dev, const char *buf, size_t *bytes)
{
	int retcode;

	if (!dev || !dev->drv || !buf || !bytes) {
		return (EINVAL);
	}

	if (dev->header.type != DEV_TYPE_CHAR) {
		return (EPERM);
	}

	if (dev->header.write_fn) {
		retcode = dev->header.write_fn(buf, bytes, dev->cdrv, dev->header.unitno);
	} else {
		retcode = dev->cdrv->write_fn(buf, bytes, dev->header.unitno);
	}

	return (retcode);
}

int device_io_rx(device_t *dev, char *buf, size_t *bytes)
{
	int retcode;

	if (!dev || !dev->drv || !buf || !bytes) {
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

	return (retcode);
}

int device_io_rq_lock(device_t *dev)
{
	device_int_t *cntr;

	if (!dev || !dev->drv) {
		return (EINVAL);
	}

	if (dev->header.type != DEV_TYPE_BLOCK) {
		return (ENOTBLK);
	}

	cntr = container_of(dev, device_int_t, dv);

	return (thread_lock_mutex(cntr->blkrq->rq_mutex));
}

int device_io_rq_unlock(device_t *dev)
{
	device_int_t *cntr;

	if (!dev || !dev->drv) {
		return (EINVAL);
	}

	if (dev->header.type != DEV_TYPE_BLOCK) {
		return (ENOTBLK);
	}

	cntr = container_of(dev, device_int_t, dv);

	return (thread_unlock_mutex(cntr->blkrq->rq_mutex));
}

int device_io_rq_tx(device_t *dev, const char *buf, size_t bytes, uint64_t sblock, unsigned dsk)
{
	device_int_t *cntr;
	struct block_request *req;

	if (!dev || !dev->drv || !buf) {
		return (EINVAL);
	}

	if (dev->header.type != DEV_TYPE_BLOCK) {
		return (ENOTBLK);
	}

	if (sblock >= dev->bdrv->block_count)
		return (EACCES);

	cntr = container_of(dev, device_int_t, dv);

	if (!thread_mutex_is_locked(cntr->blkrq->rq_mutex)) {
		return (EPERM);
	}

	if (cntr->blkrq->rq_size == BLKRQMAXSIZE) {
		return (EAGAIN);
	}

	req = calloc(1, sizeof(*req));
	if (!req) {
		return (ENOMEM);
	}

	req->start_block = sblock;
	req->buffer = (void *)buf;
	req->buffer_size = bytes;
	req->bytes = bytes;
	req->disk = (uint16_t) dsk;
	req->cmd = 1;		/* WRITE */
	req->errors = 0;
	cntr->blkrq->rq[cntr->blkrq->rq_size++] = req;

	return (EOK);
}

int device_io_rq_rx(device_t *dev, char *buf, size_t bytes, uint64_t sblock, unsigned dsk)
{
	device_int_t *cntr;
	struct block_request *req;

	if (!dev || !dev->drv || !buf) {
		return (EINVAL);
	}

	if (dev->header.type != DEV_TYPE_BLOCK) {
		return (ENOTBLK);
	}

	if (sblock >= dev->bdrv->block_count)
		return (EACCES);

	cntr = container_of(dev, device_int_t, dv);

	if (!thread_mutex_is_locked(cntr->blkrq->rq_mutex)) {
		return (EPERM);
	}

	if (cntr->blkrq->rq_size == BLKRQMAXSIZE) {
		return (EAGAIN);
	}

	req = calloc(1, sizeof(*req));
	if (!req) {
		return (ENOMEM);
	}

	req->start_block = sblock;
	req->buffer = (void *)buf;
	req->buffer_size = bytes;
	req->bytes = bytes;
	req->disk = (uint16_t) dsk;
	req->cmd = 0;	/* READ */
	req->errors = 0;
	cntr->blkrq->rq[cntr->blkrq->rq_size++] = req;

	return (EOK);

}

int device_io_rq_commit(device_t *dev, int *retcodes)
{
	device_int_t *cntr;
	int retcode = EOK;
	unsigned cmd;
	unsigned i = 0, j = 0, k = 0;
	int *retptr = retcodes;

	if (!dev || !dev->drv || !retcodes) {
		return (EINVAL);
	}

	if (dev->header.type != DEV_TYPE_BLOCK) {
		return (ENOTBLK);
	}

	cntr = container_of(dev, device_int_t, dv);

	if (!thread_mutex_is_locked(cntr->blkrq->rq_mutex)) {
		return (EPERM);
	}

	if (cntr->blkrq->rq_size == 0) {
		return (EOK);
	}

	cmd = cntr->blkrq->rq[i]->cmd;
	while (cntr->blkrq->rq_size && (retcode == EOK)) {
		while ((cmd == cntr->blkrq->rq[i + k]->cmd) && cntr->blkrq->rq_size) {
			--cntr->blkrq->rq_size;
			++i;
		}

		switch (cmd) {
		case 0:
			retcode = dev->bdrv->read_multi_fn(cntr->blkrq->rq + k, i, dev->header.unitno);
			break;
		case 1:
			retcode = dev->bdrv->write_multi_fn(cntr->blkrq->rq + k, i, dev->header.unitno);
			break;
		default:
			kerrprintf("Device %16s with unknown operation %d at position %d\n", dev->header.name, cmd, k);
			/* Purge all allocated memory and return failure */
			while (cntr->blkrq->rq_size--) {
				*retptr++ = EIO;
				free(cntr->blkrq->rq[k]);
				cntr->blkrq->rq[k++] = NULL;
			}
			return (EIO);
		}

		k = j + i;
		while (j < k) {
			*retptr++ = (cntr->blkrq->rq[j]->errors) ? (EIO) : (EOK);
			free(cntr->blkrq->rq[j]);
			cntr->blkrq->rq[j++] = NULL;
		}

		cmd = cntr->blkrq->rq[k]->cmd;
		i = 0;
	}

	if (cntr->blkrq->rq_size) {
		kerrprintf("Device %16s request queue not empty after commit\n", dev->header.name);
		/* Purge all allocated memory and return failure */
		while (cntr->blkrq->rq_size--) {
			*retptr++ = EIO;
			free(cntr->blkrq->rq[k]);
			cntr->blkrq->rq[k++] = NULL;
		}
		retcode = EIO;
	}

	return (retcode);
}

int device_io_rq_abort(device_t *dev)
{
	device_int_t *cntr;
	unsigned i = 0;

	if (!dev || !dev->drv) {
		return (EINVAL);
	}

	if (dev->header.type != DEV_TYPE_BLOCK) {
		return (ENOTBLK);
	}

	cntr = container_of(dev, device_int_t, dv);

	if (!thread_mutex_is_locked(cntr->blkrq->rq_mutex)) {
		return (EPERM);
	}

	if (cntr->blkrq->rq_size == 0) {
		return (EOK);
	}

	while (cntr->blkrq->rq_size--) {
		free(cntr->blkrq->rq[i]);
		cntr->blkrq->rq[i++] = NULL;
	}

	return (EOK);
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
