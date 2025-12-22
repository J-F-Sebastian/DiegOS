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

#include <stdlib.h>
#include <string.h>

#include "platform_include.h"
#include "threads.h"
#include "scheduler.h"
#include "fail_safe.h"
#include "kprintf.h"

static thread_t *thread_storage = NULL;

static unsigned thread_num = 0;

BOOL init_thread_lib()
{
	unsigned i;

	if (DIEGOS_MAX_THREADS >= THREAD_TID_INVALID) {
		return (FALSE);
	}

	thread_storage = (thread_t *) malloc(DIEGOS_MAX_THREADS * sizeof(*thread_storage) + CACHE_ALN);
	if (!thread_storage) {
		return (FALSE);
	}

	/*
	 * We are loosing pointers here, it is voluntary as no one will ever
	 * claim memory back, unless you reboot/reset/reload, and in that case
	 * anything will be cleared out.
	 */
	thread_storage = (thread_t *) ALNC((uintptr_t) thread_storage);
	memset(thread_storage, 0, DIEGOS_MAX_THREADS * sizeof(*thread_storage));

	for (i = 0; i < DIEGOS_MAX_THREADS; i++) {
		thread_storage[i].tid = THREAD_TID_INVALID;
	}

	return (TRUE);
}

uint8_t init_thread(const char *name,
		    uint8_t prio, void (*entry_ptr) (void), void *stack, uint32_t stack_size)
{
	uint32_t i, new_tid = THREAD_TID_INVALID;

	if ((DIEGOS_MAX_THREADS == thread_num) || (!name) || (!entry_ptr) || !(prio < THREAD_PRIORITIES)) {
		return THREAD_TID_INVALID;
	}

	/*
	 * Double function : trace first available slot,
	 * seek no other processes have his name.
	 * Do NOT bail out once an empty slot is found, need to go all
	 * way down to DIEGOS_MAX_THREADS to check all names.
	 */
	for (i = 0; i < DIEGOS_MAX_THREADS; i++) {
		if ((new_tid == THREAD_TID_INVALID) &&
		    !thread_storage[i].name[0] && (thread_storage[i].tid == THREAD_TID_INVALID)) {
			new_tid = i;
		} else if (!strncmp(thread_storage[i].name, name, THREAD_NAME_MAX)) {
			return THREAD_TID_INVALID;
		}
	}

	/*
	 * Stack check ...
	 */
	if (stack) {
		thread_storage[new_tid].stack = stack;
	} else {
		thread_storage[new_tid].stack = malloc(stack_size);
		thread_storage[new_tid].flags |= THREAD_FLAG_REL_STACK;
	}

	if (!thread_storage[new_tid].stack) {
		thread_storage[new_tid].flags = 0;
		return THREAD_TID_INVALID;
	}

	thread_storage[new_tid].stack_size = stack_size;

	/* Mark stack space for tracing */
	memset(thread_storage[new_tid].stack, 0xdf, stack_size);

	/*
	 * OK GO!
	 */
	strncpy(thread_storage[new_tid].name, name, THREAD_NAME_MAX);
	thread_storage[new_tid].name[THREAD_NAME_MAX] = 0;
	thread_storage[new_tid].priority = prio;
	thread_storage[new_tid].state = THREAD_READY;
	thread_storage[new_tid].tid = new_tid;
	thread_storage[new_tid].entry_ptr = entry_ptr;

	thread_num++;

	setup_context(thread_storage[new_tid].stack + stack_size,
		      scheduler_fail_safe,
		      thread_storage[new_tid].entry_ptr, &thread_storage[new_tid].context);

	kmsgprintf("Created thread %s TID %u Stack at %p\n",
		   thread_storage[new_tid].name, new_tid, thread_storage[new_tid].stack);
	return (new_tid);
}

BOOL done_thread(uint8_t tid)
{
	thread_t *th = get_thread(tid);

	if (th) {
		if (th->flags & THREAD_FLAG_REL_STACK) {
			free(th->stack);
		}
		cleanup_context(th->context, tid);
		memset(th, 0, sizeof(*th));
		th->tid = THREAD_TID_INVALID;
		--thread_num;
		kprintf("TID %u killed\n", tid);
		return (TRUE);
	}

	return (FALSE);
}

thread_t *get_thread(uint8_t tid)
{
	if ((tid < THREAD_TID_INVALID) && (tid < DIEGOS_MAX_THREADS) && (thread_storage[tid].tid == tid)) {
		return (&thread_storage[tid]);
	}

	return (NULL);
}

const char *state2str(uint8_t state)
{
	switch (state) {
	case THREAD_RUNNING:
		return ("RUNNING");
	case THREAD_WAITING:
		return ("WAITING");
	case THREAD_READY:
		return ("READY");
	case THREAD_DEAD:
		return ("DEAD");
	}

	return ("N/A");
}

void check_thread_stack()
{
	unsigned i, j;
	uint32_t *stack_top;

	for (i = 0; i < DIEGOS_MAX_THREADS; i++) {
		if (thread_storage[i].tid == THREAD_TID_INVALID) {
			continue;
		}
		j = 0;
		stack_top = (uint32_t *) (thread_storage[i].stack);
		while (0xdfdfdfdfUL == *stack_top++) {
			++j;
		}
		if (!j) {
			kerrprintf("Thread %u [%s] stack overflow !!!\n",
				   i, thread_storage[i].name);
		} else {
			kprintf("Thread %u [%s] stack free: %u Bytes\n",
				i, thread_storage[i].name, j * 4);
		}
	}
}
