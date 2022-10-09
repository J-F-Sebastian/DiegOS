/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2022 Diego Gallizioli
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

#include <diegos/kernel.h>
#include <diegos/kernel_ticks.h>
#include <diegos/kernel_dump.h>
#include <diegos/events.h>
#include <libs/iomalloc.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

extern void diegos_malloc_stats(unsigned long bytes[2]);
extern void diegos_iomalloc_stats(unsigned long bytes[2]);

static int running = 20 * 80 + 1;

static void demo_thread_entry(void)
{
	void *iomem[20];
	unsigned i = 0;
	unsigned long sbytes[2];
	unsigned long ebytes[2];
	unsigned long iosbytes[2];
	unsigned long ioebytes[2];

	diegos_malloc_stats(sbytes);
	diegos_iomalloc_stats(iosbytes);

	while (running--) {
		iomem[i++] = iomalloc(rand() % 256 * 1024 + rand() % 64 * 1024, 16);
		if (i == 20) {
			putchar('\n');
			while (i--) {
				if (iomem[i]) {
					printf("%X\t", iomem[i]);
					iofree(iomem[i]);
				} else
					printf("NULL\t");
			}
			i++;
		}
	}
	while (i--) {
		if (iomem[i])
			iofree(iomem[i]);
	}
	diegos_malloc_stats(ebytes);
	diegos_iomalloc_stats(ioebytes);
	printf("\nINITIAL FREE %d\tALLOC %d\n", sbytes[0], sbytes[1]);
	printf("FINAL FREE %d\tALLOC %d\n", ebytes[0], ebytes[1]);
	printf("INITIAL IO FREE %d\tALLOC %d\n", iosbytes[0], iosbytes[1]);
	printf("FINAL IO FREE %d\tALLOC %d\n", ioebytes[0], ioebytes[1]);
	printf("Going going gone!!!\n");
	thread_terminate();
}

void platform_run(void)
{
	uint8_t pid;

	thread_create("Demo", THREAD_PRIO_NORMAL, demo_thread_entry, 0, 4096, &pid);
}
