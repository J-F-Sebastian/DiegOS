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
    //int i;
    //ev_queue_t *evt = event_init_queue(NULL);
    //mutex_t *newm = thread_create_mutex(NULL);
    //barrier_t *bar = barrier_create(NULL,NULL);
    //alarm_t *alm = alarm_create(NULL, my_thread_id(),10000,TRUE,evt);
    char *ehehe = malloc(4000);

    memset(ehehe, 96, 4000);
    ehehe[3999] = 0;
    unsigned t = 0;

    while (TRUE) {
        //thread_lock_mutex(mutex);
#if 0
        if (rand()%16 == my_thread_id()) {
            printf("My thread is %s and I am waiting\n",my_thread_name());
            //thread_delay(3000);
        } else {
            printf("%s\n",ehehe);
            //printf("My thread is %s\n",my_thread_name());
        }

        //i = rand()%5000;
        //i = 0;
        //printf("My thread is %s and I am waiting for %dms\n",my_thread_name(),i);
        //thread_delay(i);
        if (thread_mutex_is_locked(newm)) {
            printf("Is locked\n");
        }
        if (barrier_open(bar)) {
            //printf("Is open\n");
        }
        if (alarm_update(alm, my_thread_id(), TRUE)) {
        }
        //i = rand()%5;
//        i = 1;
//        printf("My thread is %s and I am waiting for %ds\n",my_thread_name(),i);
        //sleep(i);
//        if (my_thread_pid()<21) {
//            mdelay(i*1000);
//        } else if (my_thread_pid()<41) {
//            udelay(i*1000000);
//        } else {
//            ndelay(i*1000000);
//        }

        //thread_unlock_mutex(mutex);
        if (my_thread_id() == 19) {
            threads_dump();
            thread_delay(3000);
            mutexes_dump();
            thread_delay(3000);
            barrier_dump(NULL);
            thread_delay(3000);
            alarm_dump(NULL);
            thread_delay(3000);
            event_dump(NULL);
            thread_delay(3000);
        }
        #endif
        mdelay(1000);
        udelay(1000);
        ndelay(1000);
        printf("Counting %u\n",t++);
        //thread_suspend();
    }
}

void platform_run(void)
{
    uint8_t pid;
    volatile unsigned i = 0;
    char name[10];
    BOOL retcode;

    mutex = thread_create_mutex("test mutex");

    do {
        sprintf(name, "demo%d",i);

        retcode = thread_create(name,
                                THREAD_PRIO_NORMAL,
                                demo_thread_entry,
                                0,
                                4096,
                                &pid);
        i++;
        break;
    } while (retcode);
}
