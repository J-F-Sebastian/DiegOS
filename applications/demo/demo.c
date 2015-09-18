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
#include <stdio.h>
#include <stdlib.h>

static uint8_t mutex = 0;

static void demo_thread_entry(void)
{
    printf("Demo!\n");
    int i = 400;
    int j;

    while (i--) {
        thread_lock_mutex(mutex);
        j = 5;
        while (j--) {
            thread_suspend();
        }
        thread_unlock_mutex(mutex);
        thread_suspend();
    }
}

static void demo_thread_entry2(void)
{
    printf("Demo2!\n");
    int i = 500;
    int j;
    int k = -1;

    while (i--) {
        thread_delay(2);
        thread_lock_mutex(mutex);
        j = 200;
        while (j--) {
            k++;
            thread_lock_mutex(mutex);
        }
        thread_unlock_mutex(mutex);
        thread_may_suspend();
    }

    threads_dump();
    mutexes_dump();
}
static void demo_thread_entry3(void)
{
    printf("Demo3!\n");
    int i = 1000;
    int j;
    int k = -4;

    while (i--) {
        thread_delay(1);
        thread_lock_mutex(mutex);
        j = 100;
        while (j--) {
            k++;
            thread_lock_mutex(mutex);
        }

        thread_unlock_mutex(mutex);
        thread_suspend();
    }
}

void platform_run(void)
{
    uint8_t pid;

    mutex = thread_create_mutex("test mutex");

    thread_create("Demo",
                  THREAD_PRIO_HIGH,
                  demo_thread_entry,
                  0,
                  4096, &pid);

    thread_create("Demo2",
                  THREAD_PRIO_HIGH,
                  demo_thread_entry2,
                  0,
                  4096, &pid);

    thread_create("Demo3",
                  THREAD_PRIO_HIGH,
                  demo_thread_entry3,
                  0,
                  4096, &pid);
}
