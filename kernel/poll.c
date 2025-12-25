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

#include <diegos/interrupts.h>
#include <diegos/poll.h>
#include <diegos/devices.h>
#include <diegos/net_interfaces.h>
#include <errno.h>
#include <libs/list.h>
#include <libs/chunks.h>
#include <libs/bitmaps.h>
#include <string.h>

#include "scheduler.h"
#include "threads.h"
#include "io_waits_private.h"
#include "poll_private.h"
#include "./libs/unistd/fdescr_private.h"
#include "kprintf.h"
#include "platform_include.h"

struct poll_item {
	queue_node header;
	int fd;
	wait_queue_t *wq;
	struct wait_queue_item *item;
};

/*
 * a poll_table oject is created for every call to poll.
 */
typedef struct poll_table {
	/*
	 * All poll_tables are doubly linked
	 */
	list_node header;
	/*
	 * All poll_tables have a poll_item table
	 */
	queue_inst table;
	/*
	 * if any wq is signalled, and the wq is in the poll_table table, set
	 * signalled to 1.
	 */
	int signalled;
	/*
	 * Thread to be woken up
	 */
	uint8_t tid;
} poll_table_t;

static chunks_pool_t *poll_items = NULL;
static chunks_pool_t *poll_tables = NULL;
static list_inst poll_table_list;
static long thread_ids[BITMAPLEN(DIEGOS_MAX_THREADS)];

static void cleanup(poll_table_t * table)
{
	struct poll_item *cursor;

	while (queue_count(&table->table)) {
		if (EOK == queue_dequeue(&table->table, (queue_node **) & cursor)) {
			io_wait_remove(cursor->item, cursor->wq);
			io_wait_put_item(cursor->item);
			chunks_pool_free(poll_items, cursor);
		}
	}

	list_remove(&poll_table_list, &table->header);
	chunks_pool_free(poll_tables, table);
}

int poll(struct pollfd ufds[], unsigned nfds, int timeout)
{
	poll_table_t *newtable;
	unsigned i;
	fd_data_t *cursor;
	thread_t *prev, *next;
	short events;

	if (!ufds || !nfds || (nfds > OPEN_MAX)) {
		return EINVAL;
	}

	newtable = chunks_pool_malloc(poll_tables);

	if (!newtable) {
		return EAGAIN;
	}

	if ((EOK != list_prepend(&poll_table_list, &newtable->header)) ||
	    (EOK != queue_init(&newtable->table))) {
		chunks_pool_free(poll_tables, newtable);
		return EAGAIN;
	}

	newtable->signalled = 0;
	newtable->tid = scheduler_running_tid();

	for (i = 0; i < nfds; i++) {
		cursor = fdget(ufds[i].fd);
		if (cursor) {
			if (EOK != device_poll(cursor->rawdev, newtable, &events)) {
				kerrprintf("Device %s failed polling 1\n", cursor->rawdev->header.name);
				cleanup(newtable);
				return ENOSYS;
			}
			ufds[i].events = events;
			ufds[i].events &= ufds[i].revents;
			if (ufds[i].events) {
				newtable->signalled = 1;
			}
		}
	}

	if (!newtable->signalled) {
		prev = scheduler_running_thread();

		if (!scheduler_wait_thread(THREAD_FLAG_WAIT_COMPLETION, 0)) {
			kerrprintf("TID %u Cannot wait for poll\n", prev);
			cleanup(newtable);
			return EPERM;
		}

		schedule_thread();
		next = scheduler_running_thread();
		switch_context(&prev->context, next->context);
	}

	for (i = 0; i < nfds; i++) {
		cursor = fdget(ufds[i].fd);
		if (cursor) {
			if (EOK != device_poll(cursor->rawdev, NULL, &events)) {
				kerrprintf("Device %s failed polling 2\n", cursor->rawdev->header.name);
				cleanup(newtable);
				return ENOSYS;
			}
			ufds[i].events = events;
			ufds[i].events &= ufds[i].revents;
		}
	}

	cleanup(newtable);

	return EOK;
}

int poll_network(struct pollfd ufds[], unsigned nfds, int timeout)
{
#if 0
	poll_table_t *newtable;
	unsigned i;
	net_interface_t *cursor;
	thread_t *prev, *next;
	short events;

	if (!ufds || !nfds || (nfds > OPEN_MAX)) {
		return EINVAL;
	}

	newtable = chunks_pool_malloc(poll_tables);

	if (!newtable) {
		return EAGAIN;
	}

	if ((EOK != list_prepend(&poll_table_list, &newtable->header)) ||
	    (EOK != queue_init(&newtable->table))) {
		chunks_pool_free(poll_tables, newtable);
		return EAGAIN;
	}

	newtable->signalled = 0;
	newtable->tid = scheduler_running_tid();

	for (i = 0; i < nfds; i++) {
		cursor = net_interface_lookup_index(ufds[i].fd);
		if (cursor) {
			if (EOK != device_poll(cursor->drv, newtable, &events)) {
				kerrprintf("Interface %s failed polling 1\n", cursor->drv->ifname);
				cleanup(newtable);
				return ENOSYS;
			}
			ufds[i].events = events;
			ufds[i].events &= ufds[i].revents;
			if (ufds[i].events) {
				newtable->signalled = 1;
			}
		}
	}

	if (!newtable->signalled) {
		prev = scheduler_running_thread();

		if (!scheduler_wait_thread(THREAD_FLAG_WAIT_COMPLETION)) {
			kerrprintf("TID %u Cannot wait for poll\n", prev);
			cleanup(newtable);
			return EPERM;
		}

		schedule_thread();
		next = scheduler_running_thread();
		switch_context(&prev->context, next->context);
	}

	for (i = 0; i < nfds; i++) {
		cursor = net_interface_lookup_index(ufds[i].fd);
		if (cursor) {
			if (EOK != device_poll(cursor->drv, newtable, &events)) {
				kerrprintf("Interface %s failed polling 2\n", cursor->drv->ifname);
				cleanup(newtable);
				return ENOSYS;
			}
			ufds[i].events = events;
			ufds[i].events &= ufds[i].revents;
		}
	}

	cleanup(newtable);
#endif
	return EOK;
}

int poll_wait(wait_queue_t * wq, poll_table_t * table)
{
	struct wait_queue_item *temp;
	struct poll_item *cursor;

	if (!wq) {
		return EINVAL;
	}

	if (!table) {
		return EOK;
	}

	temp = io_wait_get_item();
	cursor = chunks_pool_malloc(poll_items);

	if (!temp || !cursor) {
		if (temp) {
			io_wait_put_item(temp);
		}
		if (cursor) {
			chunks_pool_free(poll_items, cursor);
		}
		return EPERM;
	}

	if (EOK != io_wait_add(temp, wq)) {
		chunks_pool_free(poll_items, cursor);
		io_wait_put_item(temp);
		return EPERM;
	}

	temp->flags = IO_WAIT_POLL;
	temp->pt = (void *)table;
	cursor->item = temp;
	cursor->wq = wq;

	queue_enqueue(&table->table, &cursor->header);

	return EOK;
}

/*
 * Private section
 */

BOOL init_poll_lib()
{
	if (EOK != list_init(&poll_table_list)) {
		return (FALSE);
	}

	poll_items = chunks_pool_create("poll items",
					0, sizeof(struct poll_item), DRV_UNIT_MAX, DRV_UNIT_MAX);

	if (!poll_items) {
		return (FALSE);
	}

	poll_tables = chunks_pool_create("poll tables",
					 0, sizeof(poll_table_t), DRV_UNIT_MAX, DRV_UNIT_MAX);

	if (!poll_tables) {
		chunks_pool_done(poll_items);
		return (FALSE);
	}

	memset(thread_ids, 0, sizeof(thread_ids));

	return (TRUE);
}

int poll_wakeup(struct wait_queue_item *wqi)
{
	poll_table_t *pt;

	if (!wqi || !wqi->pt || (IO_WAIT_POLL != wqi->flags)) {
		return EINVAL;
	}

	lock();
	pt = (poll_table_t *) wqi->pt;
	if (!pt->signalled) {
		pt->signalled = 1;
		bitmap_set(thread_ids, pt->tid);
	}
	unlock();

	return EOK;
}

static void resumecb(long *bitmap, unsigned pos, void *param)
{
	if (!scheduler_resume_thread(THREAD_FLAG_WAIT_COMPLETION, (uint8_t) pos)) {
		kerrprintf("TID %u cannot be resumed from poll\n", pos);
	}
	bitmap_clear(bitmap, pos);
}

void resume_on_poll()
{
	lock();
	bitmap_for_each_set(thread_ids, BITMAPLEN(DIEGOS_MAX_THREADS), resumecb, NULL);
	unlock();
}
