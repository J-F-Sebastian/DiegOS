/**
 *
 *                                                   ,----..
 *        ,---,                                     /   /   \   .--.--.
 *      .'  .' `\    ,--,                          /   .     : /  /    '.
 *    ,---.'     \ ,--.'|                         .   /   ;.  \  :  /`. /
 *    |   |  .`\  ||  |,                ,----._,..   ;   /  ` ;  |  |--`
 *    :   : |  '  |`--'_       ,---.   /   /  ' /;   |  ; \ ; |  :  ;_
 *    |   ' '  ;  :,' ,'|     /     \ |   :     ||   :  | ; | '\  \    `.
 *    '   | ;  .  |'  | |    /    /  ||   | .\  ..   |  ' ' ' : `----.   \
 *    |   | :  |  '|  | :   .    ' / |.   ; ';  |'   ;  \; /  | __ \  \  |
 *    '   : | /  ; '  : |__ '   ;   /|'   .   . | \   \  ',  / /  /`--'  /
 *    |   | '` ,/  |  | '.'|'   |  / | `---`-'| |  ;   :    / '--'.     /
 *    ;   :  .'    ;  :    ;|   :    | .'__/\_: |   \   \ .'    `--'---'
 *    |   ,.'      |  ,   /  \   \  /  |   :    :    `---`
 *    '---'         ---`-'    `----'    \   \  /
 *                                       `--`-'
 */

/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2026 Diego Gallizioli
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
#include <diegos/alarms.h>
#include <diegos/mutexes.h>
#include <diegos/barriers.h>
#include <diegos/events.h>
#include <diegos/io_waits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

static ev_queue_t *events;
static barrier_t *barrier;
static mutex_t *mutex;

static void demo_thread_events(void)
{
        mutex = thread_create_mutex("Mutex1");
	event_t *evt;
	unsigned i = 0;
	event_watch_queue(events);

	alarm_t *alm = alarm_create("testalm", 100, 123, TRUE, events);
	alarm_t *alm2 = alarm_create("testalm2", 101, 253, TRUE, events);

	alarm_set(alm, TRUE);
	alarm_set(alm2, TRUE);

	while (i < 10) {
		if (wait_for_events(events)) {
			printf("Event error\n");
			exit(1);
		}
		i++;

		while (event_queue_size(events)) {

			evt = event_get(events);

			while (thread_lock_mutex_timed(mutex, 50) == ETIMEDOUT) {
				printf("Mutex lock timeout\n");
			}
			if (100 == evt->eventid)
				alarm_acknowledge(alm);
			if (101 == evt->eventid)
				alarm_acknowledge(alm2);

			if (evt) {
				printf("%d classid %d eventid %d\n", event_queue_size(events),
				       evt->classid, evt->eventid);
			}

			if (!(i % 7)) {
				//alarm_update(alm, 1234 + i*2, TRUE);
			}
			//thread_lock_mutex(mutex);
		}
	}

	alarm_update(alm, 1234, TRUE);
	alarm_update(alm2, 1253, TRUE);
	i = 0;
	while (i < 10) {
		while (ETIMEDOUT == wait_for_events_timed(events, 196)) {
			//printf("Event timeout\n");
		}
		i++;

		while (event_queue_size(events)) {

			evt = event_get(events);

			if (100 == evt->eventid)
				alarm_acknowledge(alm);
			if (101 == evt->eventid)
				alarm_acknowledge(alm2);

			if (evt) {
				printf("XXX %d classid %d eventid %d\n", event_queue_size(events),
				       evt->classid, evt->eventid);
			}

			if (!(i % 7)) {
				//alarm_update(alm, 1234 + i*2, TRUE);
			}
		}
	}

	alarm_set(alm, FALSE);
	alarm_set(alm2, FALSE);
	alarm_done(alm);
	alarm_done(alm2);
	thread_terminate();
}

static void demo_xxx_wait_on_brt(void)
{
	thread_lock_mutex(mutex);
        printf("My ID %d\n", my_thread_id());
	thread_unlock_mutex(mutex);
        wait_for_barrier(barrier);
}

static void demo_thread_barriers(void)
{
        uint8_t pid;
        unsigned i;
        char name[6];

        for (i = 0; i < 25; i++) {
                snprintf(name, sizeof(name), "XXX%2d", i);        
                if (!thread_create(name, THREAD_PRIO_HIGH, demo_xxx_wait_on_brt, 0, 4096, &pid))
                {
                        printf("TOP %d\n", pid);
                        break;
                }
        }

	thread_lock_mutex(mutex);
        thread_delay(10000);
	thread_unlock_mutex(mutex);
	thread_delay(10000);
        barrier_open(barrier);
        printf("Barrier opened\n");
        thread_delay(1000);
}

static void demo_thread_mutexes(void)
{
        printf("--- My ID %d\n", my_thread_id());
        thread_delay(1000);
}

void platform_run(void)
{
	uint8_t pid;

	events = event_init_queue("Teste");
        barrier = barrier_create("Testb", TRUE);

	thread_create("Evt", THREAD_PRIO_NORMAL, demo_thread_events, 0, 4096, &pid);
        thread_create("Brt", THREAD_PRIO_NORMAL, demo_thread_barriers, 0, 4096, &pid);
        thread_create("Mtx", THREAD_PRIO_NORMAL, demo_thread_mutexes, 0, 4096, &pid);
}
