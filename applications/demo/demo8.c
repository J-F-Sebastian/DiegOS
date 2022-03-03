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

#include <diegos/kernel.h>
#include <diegos/kernel_dump.h>
#include <diegos/timers.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void timer1(void *a)
{
	printf("1\n");
}

static void timer2(void *a)
{
	printf("2\n");
}

static void demo_thread_entry(void)
{
	unsigned i = 0;

	timer_t *alm = timer_create("testalm", 123, TRUE, timer1, NULL);
	timer_t *alm2 = timer_create("testalm2", 253, TRUE, timer2, NULL);

	timer_set(alm, TRUE);
	timer_set(alm2, TRUE);

	timers_dump(NULL);

	while (++i < 30) {
		sleep(1);
	}

	timer_done(alm);
	timer_done(alm2);
	timers_dump(NULL);
	thread_terminate();
}

void platform_run(void)
{
	uint8_t pid;

	thread_create("Demo", THREAD_PRIO_NORMAL, demo_thread_entry, 0, 4096, &pid);
}
