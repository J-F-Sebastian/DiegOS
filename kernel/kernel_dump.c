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
#include "platform_include.h"

extern long _text_start, _text_end, _data_start, _data_end, _bss_start, _bss_end;

void threads_dump()
{
	uint32_t i;
	thread_t *ptr;

	printf("\n--- THREADS TABLE ----------------------------\n\n");
	printf("%-15s   %3s   %6s   %s\n", "THREAD NAME", "TID", "STACK", "STATE");
	printf("______________________________________________\n");
	for (i = 0; i < DIEGOS_MAX_THREADS; i++) {
		ptr = get_thread(i);
		if (ptr) {
			printf("%-15s | %3u | %6u | %s\n",
			       ptr->name, ptr->tid, ptr->stack_size, state2str(ptr->state));
		}
	}
	printf("----------------------------------------------\n\n");
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

void diegos_dump()
{
	void *heap_start;
	unsigned long heap_size;
	void *iomem_start;
	unsigned long iomem_size;
	unsigned long mem_size;

	total_memory(&mem_size);
	cacheable_memory(&heap_start, &heap_size);
	io_memory(&iomem_start, &iomem_size);

	printf("\n--- SYSTEM TABLE -----------------------------------------------\n");
	printf("        |  Text    |  Data    |  BSS     |  Heap    |  I/O\n");
	printf("_________________________________________________________________\n");
	printf("Start   | %#8x | %#8x | %#8x | %#8x | %#8x\n",
	       &_text_start, &_data_start, &_bss_start, (uintptr_t) (heap_start),
	       (uintptr_t) iomem_start);
	printf("--------+----------+----------+----------+----------+----------\n");
	printf("End     | %#8x | %#8x | %#8x | %#8x | %#8x\n",
	       &_text_end, &_data_end, &_bss_end, (uintptr_t) (heap_start) + heap_size,
	       (uintptr_t) (iomem_start) + iomem_size);
	printf("-----------------------------------------------------------------\n\n");
	printf("Total RAM Size .....: %d MBytes\n", mem_size / MBYTE);
	printf("Heap Memory Size ...: %d KBytes\n", heap_size / KBYTE);
	printf("I/O Memory Size ....: %d KBytes\n", iomem_size / KBYTE);
}
