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
#include <libs/list.h>

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

	cbuffer_init(&tmp->thread_ids, DIEGOS_MAX_THREADS);
	memset(tmp->ids_buffer, THREAD_TID_INVALID, sizeof(tmp->ids_buffer));

	if (EOK != list_prepend(&mutexes_list, &tmp->header)) {
		free(tmp);
		return (NULL);
	}

	return (tmp);
}

BOOL done_mutex(struct mutex *mtx)
{
	if (!mtx) {
		return (FALSE);
	}

	if (THREAD_TID_INVALID != mtx->locker_tid) {
		return (FALSE);
	}

	if (!cbuffer_is_empty(&mtx->thread_ids)) {
		return (FALSE);
	}

	if (EOK != list_remove(&mutexes_list, &mtx->header)) {
		return (FALSE);
	}

	memset(mtx, 0, sizeof(*mtx));
	free(mtx);

	return (TRUE);
}

BOOL lock_mutex(uint8_t tid, struct mutex *mtx)
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

	if (cbuffer_free_space(&mtx->thread_ids) == 0) {
		return (FALSE);
	}

	mtx->ids_buffer[mtx->thread_ids.tail] = tid;
	cbuffer_add(&mtx->thread_ids);

	return (TRUE);
}

BOOL unlock_mutex(uint8_t tid, struct mutex *mtx)
{
	thread_t *ptr;

	if (!mtx) {
		return (FALSE);
	}

	if ((THREAD_TID_INVALID == mtx->locker_tid) || (tid != mtx->locker_tid)) {
		return (FALSE);
	}

	if (cbuffer_is_empty(&mtx->thread_ids) == FALSE) {
		/*
		 * Clean up cancelled threads
		 */
		while (mtx->ids_buffer[mtx->thread_ids.head] == THREAD_TID_INVALID) {
			cbuffer_remove(&mtx->thread_ids);
			if (cbuffer_is_empty(&mtx->thread_ids)) {
				break;
			}
		}
	}

	if (cbuffer_is_empty(&mtx->thread_ids) == FALSE) {
		ptr = get_thread(mtx->ids_buffer[mtx->thread_ids.head]);
		cbuffer_remove(&mtx->thread_ids);
		mtx->locker_tid = ptr->tid;
	} else {
		mtx->locker_tid = THREAD_TID_INVALID;
	}

	return (TRUE);
}

BOOL mutex_is_locked(struct mutex *mtx)
{
	return ((mtx && (THREAD_TID_INVALID != mtx->locker_tid))
		? (TRUE) : (FALSE));
}

int cancel_wait_on_mutex(uint8_t tid)
{
	unsigned i;
	struct mutex *curr = list_head(&mutexes_list);

	while (curr) {
		if (curr->locker_tid == tid) {
			return (unlock_mutex(tid, curr)) ? (EOK) : (EPERM);
		}
		else if (!cbuffer_is_empty(&curr->thread_ids)) {
			for (i = curr->thread_ids.head;
			     i != curr->thread_ids.tail;
			     i = (i + 1) % curr->thread_ids.bufsize) {
				if (curr->ids_buffer[i] == tid) {
					curr->ids_buffer[i] = THREAD_TID_INVALID;
					return (EOK);
				}
			}
		}
		curr = (struct mutex *)curr->header.next;
	}

	return (EINVAL);
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
