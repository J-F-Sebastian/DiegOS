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

#include <stdio.h>
#include <diegos/kernel_dump.h>
#include <diegos/barriers.h>

#include "threads.h"
#include "mutex_private.h"

void threads_dump()
{
	uint32_t i;
	thread_t *ptr;

	fprintf(stderr, "\n--- THREADS TABLE ----------------------------\n\n");
	fprintf(stderr, "%-15s   %3s   %6s   %s\n", "THREAD NAME", "TID", "STACK", "STATE");
	fprintf(stderr, "______________________________________________\n");
	for (i = 0; i < DIEGOS_MAX_THREADS; i++) {
		ptr = get_thread(i);
		if (ptr) {
			fprintf(stderr,
				"%-15s | %3u | %6u | %s\n",
				ptr->name, ptr->tid, ptr->stack_size, state2str(ptr->state));
		}
	}
	fprintf(stderr, "----------------------------------------------\n\n");
}

void mutexes_dump()
{
	dump_mutex(NULL);
}

void barriers_dump()
{
	barrier_dump(NULL);
}

void threads_check()
{
	check_thread_stack();
}
