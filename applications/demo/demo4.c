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
#include <diegos/alarms.h>
#include <stdio.h>
#include <stdlib.h>

static ev_queue_t *events;

static void demo_thread_entry(void)
{
    event_t *evt;
    unsigned i = 0;
    event_watch_queue(events);

    alarm_t *alm = alarm_create("testalm",100,123,TRUE,events);
    alarm_t *alm2 = alarm_create("testalm2",101,253,TRUE,events);

    alarm_set(alm, TRUE);
    alarm_set(alm2, TRUE);

    while (1) {
        wait_for_events(events);
        i++;

        while (event_queue_size(events)) {

            evt = event_get(events);

            if (100 == evt->eventid) alarm_acknowledge(alm);
            if (101 == evt->eventid) alarm_acknowledge(alm2);

            if (evt) {
                printf("%d classid %d eventid %d\n",event_queue_size(events),
                       evt->classid,evt->eventid);
            }

            if (!(i%7)) {
                //alarm_update(alm, 1234 + i*2, TRUE);
            }
        }
    }
}

void platform_run(void)
{
    uint8_t pid;

    events = event_init_queue("Test");

    thread_create("Demo",
                  THREAD_PRIO_NORMAL,
                  demo_thread_entry,
                  0,
                  4096,
                  &pid);
}
