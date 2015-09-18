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
#include <diegos/events.h>
#include <stdio.h>
#include <stdlib.h>

static ev_queue_t *events;

static int counter = 13;
static unsigned progress = 0;

static void demo_thread_entry(void)
{
    event_t *evt;
    unsigned i = 0;
    event_watch_queue(events);

    while (1) {
        wait_for_events(events);
        i++;

        while (event_queue_size(events)) {
            evt = event_get(events);

            if (evt) {
                printf("%d classid %d eventid %d progress %u\n",
                       event_queue_size(events),
                       evt->classid,evt->eventid,
                       progress);
                counter--;
                progress++;
                free(evt);
            } else {
                printf("SKIP\n");
            }
        }
        printf("Going to wait PID %d\n",my_thread_pid());
    }
}

static void demo_thread_entry2(void)
{
    event_t *evt;

    while (1) {
        thread_delay(1000);
        evt = malloc(sizeof(event_t));
        evt->classid = 1234;
        evt->eventid = 5678;
        if (EOK != event_put(events, evt)) {
            printf("FAILURE 1 !!!\n");
        }
        evt = malloc(sizeof(event_t));
        evt->classid = 1234;
        evt->eventid = 0x9ABC;
        if (EOK != event_put(events, evt)) {
            printf("FAILURE 2 !!!\n");
        }
        printf("Going to delay PID %d\n",my_thread_pid());
    }
}

static void demo_thread_entry3(void)
{
    while (1) {
        thread_may_suspend();
    }

    printf("Going going gone!!!\n");

    thread_terminate();
}

void platform_run(void)
{
    uint8_t pid;

    events = event_init_queue("Test");

    thread_create("Demo",
                  THREAD_PRIO_NORMAL,
                  demo_thread_entry,
                  0,
                  4096, &pid);

    thread_create("Demo2",
                  THREAD_PRIO_NORMAL,
                  demo_thread_entry2,
                  0,
                  4096, &pid);

    thread_create("Demo3",
                  THREAD_PRIO_IDLE,
                  demo_thread_entry3,
                  0,
                  4096, &pid);
}
