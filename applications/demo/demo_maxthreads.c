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
#include <diegos/mutexes.h>
#include <diegos/barriers.h>
#include <diegos/alarms.h>
#include <diegos/kernel_dump.h>
#include <diegos/delays.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static mutex_t *mutex = NULL;

static void demo_thread_entry(void)
{
	char *ehehe = malloc(4000);

	memset(ehehe, 96, 4000);
	ehehe[3999] = 0;
	uint8_t t = 0;

	while (t < my_thread_id() + 5) {
		thread_lock_mutex(mutex);

		if (thread_mutex_is_locked(mutex)) {
			printf("%s is locked\n", my_thread_name());
		}
		//thread_delay(250);
		//sleep(1);
		thread_suspend();

		thread_unlock_mutex(mutex);

		//		thread_suspend();
		if (my_thread_id() == 19) {
			threads_dump();
			//thread_delay(3000);
			mutexes_dump();
			//thread_delay(3000);
			barrier_dump(NULL);
			//thread_delay(3000);
			alarm_dump(NULL);
			//thread_delay(3000);
			event_dump(NULL);
			//thread_delay(3000);
		}

		//mdelay(1000);
		//udelay(1000);
		//ndelay(1000);
		printf("Counting %s %u\n", my_thread_name(), t++);
	}
	free(ehehe);
}

void platform_run(void)
{
	uint8_t pid;
	volatile unsigned i = 0;
	char name[10];
	BOOL retcode;

	mutex = thread_create_mutex("test mutex");

	do {
		sprintf(name, "demo%d", i);

		retcode = thread_create(name, THREAD_PRIO_NORMAL, demo_thread_entry, 0, 4096, &pid);
		i++;
	} while (retcode);
	printf("Created %u threads\n", i - 1);
}
