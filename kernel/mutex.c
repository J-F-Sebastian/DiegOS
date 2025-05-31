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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <list.h>

#include "threads.h"
#include "mutex_private.h"
#include "kprintf.h"

static list_inst mutexes_list;

BOOL init_mutex_lib()
{
	if (EOK != list_init(&mutexes_list)) {
		kerrprintf("failed initing mutexes lib.\n");
		return (FALSE);
	}

	return (TRUE);
}

struct mutex *init_mutex(const char *name)
{
	struct mutex *tmp;

	tmp = malloc(sizeof(*tmp));

	if (!tmp) {
		return (NULL);
	}

	if (name) {
		snprintf(tmp->name, sizeof(tmp->name), "%s", name);
	} else {
		snprintf(tmp->name, sizeof(tmp->name), "Mutex%x", (intptr_t) tmp);
	}
	tmp->locker_tid = THREAD_TID_INVALID;

	if (EOK != queue_init(&(tmp->wait_queue))) {
		free(tmp);
		return (NULL);
	}

	if (EOK != list_add(&mutexes_list, NULL, &tmp->header)) {
		free(tmp);
		return (NULL);
	}

	return (tmp);
}

BOOL done_mutex(struct mutex * mtx)
{
	if (!mtx) {
		return (FALSE);
	}

	if (THREAD_TID_INVALID != mtx->locker_tid) {
		return (FALSE);
	}

	if (EOK != list_remove(&mutexes_list, &mtx->header)) {
		return (FALSE);
	}

	memset(mtx, 0, sizeof(*mtx));
	free(mtx);

	return (TRUE);
}

BOOL lock_mutex(uint8_t tid, struct mutex * mtx)
{
	thread_t *ptr;

	if (!mtx) {
		return (FALSE);
	}

	ptr = get_thread(tid);

	if (!ptr) {
		return (FALSE);
	}

	if (tid == mtx->locker_tid) {
		return (FALSE);
	}

	if (THREAD_TID_INVALID == mtx->locker_tid) {
		mtx->locker_tid = tid;
		return (TRUE);
	}

	if (EOK != queue_enqueue(&mtx->wait_queue, &ptr->header)) {
		kprintf("Could not set %d in wait queue\n", tid);
		return (FALSE);
	} else {
		return (TRUE);
	}
}

BOOL unlock_mutex(uint8_t tid, struct mutex * mtx)
{
	thread_t *ptr;

	if (!mtx) {
		return (FALSE);
	}

	if ((THREAD_TID_INVALID == mtx->locker_tid) || (tid != mtx->locker_tid)) {
		return (FALSE);
	}

	if (queue_count(&mtx->wait_queue)) {

		if (EOK != queue_dequeue(&mtx->wait_queue, (queue_node **) & ptr)) {
			kprintf("could not get waiting thread\n");
			return (FALSE);
		}

		mtx->locker_tid = ptr->tid;
	} else {
		mtx->locker_tid = THREAD_TID_INVALID;
	}

	return (TRUE);
}

BOOL mutex_is_locked(struct mutex * mtx)
{
	return ((mtx && (THREAD_TID_INVALID != mtx->locker_tid))
		? (TRUE) : (FALSE));
}

static void dump_internal(struct mutex *mtx)
{
	printf("%-15s | %11u | %8s | %s\n",
		mtx->name, mtx->locker_tid, mutex_is_locked(mtx) ? "LOCKED" : "UNLOCKED", "N/A");
}

void dump_mutex(struct mutex *mtx)
{
	if (!mtx) {
		printf("\n--- MUTEXES TABLE ----------------------\n\n");
	}
	printf("%-15s   %11s   %8s   %s\n", "MUTEX NAME", "LOCKER TID", "STATE", "WAITING TIDs");
	printf("______________________________________________\n");
	if (mtx) {
		dump_internal(mtx);
	} else {
		mtx = list_head(&mutexes_list);

		while (mtx) {
			dump_internal(mtx);
			mtx = (struct mutex *)mtx->header.next;
		}
	}
	printf("----------------------------------------------\n\n");
}
