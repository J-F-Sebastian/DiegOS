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
#include <diegos/kernel_ticks.h>
#include <diegos/kernel_dump.h>
#include <diegos/events.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

static ev_queue_t *events;

static int running = 1;

static void demo_thread_entry(void)
{
	event_t *evt;
	unsigned i = 0;
	event_watch_queue(events);

	while (running) {
		wait_for_events(events);

		while (event_queue_size(events)) {
			evt = event_get(events);

			if (evt) {
				printf("%d classid %d eventid %d progress %u\n",
				       event_queue_size(events), evt->classid, evt->eventid, i++);
				free(evt);
			} else {
				printf("SKIP\n");
			}
		}
		printf("Going to wait PID %d\n", my_thread_id());
	}
	printf("Going going gone!!!\n");
	thread_terminate();
}

static void demo_thread_entry2(void)
{
	event_t *evt;
	unsigned i = 0;

	while (i < 513) {
		if (i > 500)
			thread_delay(1000);
		else if (i > 250)
			thread_delay(333);
		else
			thread_delay(100);
		printf("c %lld\n", clock_get_seconds());
		i++;
		evt = malloc(sizeof(event_t));
		evt->classid = 1234;
		evt->eventid = 5678;
		if (EOK != event_put(events, evt, NULL)) {
			printf("FAILURE 1 !!!\n");
		}
		evt = malloc(sizeof(event_t));
		evt->classid = 1234;
		evt->eventid = 0x9ABC;
		if (EOK != event_put(events, evt, NULL)) {
			printf("FAILURE 2 !!!\n");
		}
		printf("Going to delay PID %d\n", my_thread_id());
	}
	printf("Going going gone!!!\n");
	running = 0;
	thread_terminate();
}

static void demo_thread_entry3(void)
{
	unsigned i = 1000;
	while (i--) {
		thread_suspend();
	}

	printf("Going going gone!!!\n");

	thread_terminate();
}

void platform_run(void)
{
	uint8_t pid;

	events = event_init_queue("Test");

	thread_create("Demo", THREAD_PRIO_NORMAL, demo_thread_entry, 0, 4096, &pid);

	thread_create("Demo2", THREAD_PRIO_NORMAL, demo_thread_entry2, 0, 4096, &pid);

	thread_create("Demo3", THREAD_PRIO_IDLE, demo_thread_entry3, 0, 4096, &pid);
}
