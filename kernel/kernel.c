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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <diegos/kernel.h>
#include <diegos/kernel_ticks.h>
#include <diegos/mutexes.h>
#include <diegos/devices.h>

#include "threads.h"
#include "kprintf.h"
#include "scheduler.h"
#include "idle_thread.h"
#include "mutex_private.h"
#include "clock.h"
#include "events_private.h"
#include "alarms_private.h"
#include "barriers_private.h"
#include "io_waits_private.h"
#include "devices_private.h"
#include "net_interfaces_private.h"
#include "drivers_private.h"
#include "poll_private.h"
#include "platform_include.h"

static const char *messages[] = { "cannot init clock",
                                  "cannot init scheduler" };

static const char *messages2[] = { "cannot init threads",
                                   "cannot init mutexes",
                                   "cannot init events",
                                   "cannot init alarms",
                                   "cannot init barriers",
                                   "cannot init I/O waits",
                                   "cannot init poll",
                                   "cannot init drivers",
                                   "cannot init devices",
                                   "cannot init interfaces" };

typedef BOOL (*initlibfn)(void);

static const initlibfn init_array[] = { clock_init,
                                        scheduler_init };

static const initlibfn libs_init_array[] = { init_thread_lib,
                                             init_mutex_lib,
                                             init_events_lib,
                                             init_alarms_lib,
                                             init_barriers_lib,
                                             init_io_waits_lib,
                                             init_poll_lib,
                                             init_drivers_lib,
                                             init_devices_lib,
                                             init_net_interfaces_lib };

typedef void (*startlibfn)(void);

static const startlibfn start_array[] = { start_devices_lib,
                                          start_net_interfaces_lib };

void kernel_init()
{
    BOOL resval;
    unsigned i;

    for (i = 0; i < NELEMENTS(init_array); i++) {
        resval = init_array[i]();
        if (!resval) {
            kernel_panic(messages[i]);
        }
    }

    kprintf("kernel setup complete.\n");
}

void kernel_libs_init()
{
    BOOL resval;
    unsigned i;

    for (i = 0; i < NELEMENTS(libs_init_array); i++) {
        resval = libs_init_array[i]();
        if (!resval) {
            kernel_panic(messages2[i]);
        }
    }

    kprintf("kernel setup complete.\n");
}

void kernel_threads_init()
{
    uint8_t tid;

    tid = init_thread("Idle",
                      THREAD_PRIO_IDLE,
                      idle_thread_entry,
                      NULL,
                      1 * KBYTE);

    if ((THREAD_TID_INVALID == tid) || (THREAD_TID_IDLE != tid)) {
        kernel_panic("cannot create the IDLE thread.");
        return;
    }

    if (!scheduler_add_thread(tid)) {
        kernel_panic("cannot add IDLE to the scheduler.");
    }

    kprintf("kernel threads setup complete.\n");
}

void kernel_panic(const char *msg)
{
    kerrprintf("KERNEL PANIC! %s\n", msg);
    abort();
}

void kernel_run()
{
    thread_t *init;
    unsigned i;

    for (i = 0; i < NELEMENTS(start_array); i++) {
        start_array[i]();
    }

    kprintf("system is running.\n");

    schedule_thread();

    init = scheduler_running_thread();

    load_context(init->context);
}

void kernel_done()
{
    uint8_t i;

    for (i = 0; i < THREAD_TID_INVALID; i++) {
        if (get_thread(i)) {
            thread_kill(i);
        }
    }

    kprintf("system shutdown is complete.\n");
    while (1) {
    };
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void thread_suspend()
{
    thread_t *prev, *next;

    prev = scheduler_running_thread();

    if (!scheduler_suspend_thread()) {
        kernel_panic("cannot suspend a thread.\n");
        return;
    }

    schedule_thread();

    next = scheduler_running_thread();

    /* No one to wait for */
    if (prev == next) {
        return;
    }

    switch_context(prev->context, next->context);
}

void thread_may_suspend()
{
    thread_t *prev = scheduler_running_thread();
    uint8_t total;

    update_schedule();

    total = scheduler_ready_threads(prev->priority);

    if (!total) {
        return;
    }

    thread_suspend();
}

void thread_delay(unsigned msecs)
{
    thread_t *prev, *next;
    uint64_t st;
    uint64_t delay;

    if (!msecs)
        return;

    st = clock_get_ticks();
    delay = clock_convert_msecs_to_ticks(msecs);

    prev = scheduler_running_thread();

    if (!scheduler_delay_thread(st + delay)) {
        kernel_panic("cannot delay a thread.\n");
        return;
    }

    schedule_thread();

    next = scheduler_running_thread();

    switch_context(prev->context, next->context);
}

void thread_terminate()
{
    thread_t *me = scheduler_running_thread();

    (void) scheduler_remove_thread(me->tid);
    schedule_thread();
    me = scheduler_running_thread();
    load_context(me->context);
}

BOOL thread_create(const char *name,
                   diegos_prio_t prio,
                   void (*entry_ptr)(void),
                   void *stack,
                   unsigned stack_size,
                   uint8_t *tid)
{
    uint8_t ntid;

    if (!tid || !stack_size || !entry_ptr || !name) {
        return (FALSE);
    }

    ntid = init_thread(name, prio, entry_ptr, stack, stack_size);

    if (THREAD_TID_INVALID == ntid) {
        kerrprintf("cannot init a new thread.\n");
        return (FALSE);
    }

    if (!scheduler_add_thread(ntid)) {
        kernel_panic("cannot schedule a thread.\n");
        return (FALSE);
    }

    *tid = ntid;

    return (TRUE);
}

void thread_kill(uint8_t tid)
{
    if (tid == scheduler_running_tid()) {
        kprintf("cannot kill the running process\n");
        return;
    }

    if (!scheduler_remove_thread(tid)) {
        kprintf("killing TID %d failed\n", tid);
        return;
    }

    kprintf("TID %d scheduled to die\n", tid);
}

uint8_t my_thread_id()
{
    return (scheduler_running_tid());
}

const char * my_thread_name()
{
    thread_t * myself = get_thread(scheduler_running_tid());

    return ((myself) ? (myself->name) : (NULL));
}

/********************************************************
 **************** M U T E X E S *************************
 ********************************************************/

mutex_t *thread_create_mutex(const char *name)
{
    mutex_t *tmp = init_mutex(name);

    if (!tmp) {
        kernel_panic("cannot create a mutex.\n");
    }

    return (tmp);
}

void thread_lock_mutex(mutex_t *mtx)
{
    thread_t *prev, *next;
    BOOL is_locked;

    if (!mtx) {
        return;
    }

    prev = scheduler_running_thread();

    is_locked = mutex_is_locked(mtx);

    if (lock_mutex(prev->tid, mtx) && is_locked) {
        scheduler_wait_thread(THREAD_FLAG_WAIT_MUTEX);
        schedule_thread();
        next = scheduler_running_thread();
        switch_context(prev->context, next->context);
    }
}

void thread_unlock_mutex(mutex_t *mtx)
{
    uint8_t ptid;
    thread_t *prev;

    if (!mtx) {
        return;
    }

    if (unlock_mutex(my_thread_id(), mtx)) {
        ptid = mtx->locker_tid;
        prev = get_thread(ptid);

        if (prev) {
            scheduler_resume_thread(THREAD_FLAG_WAIT_MUTEX, ptid);
        }
    }
}

BOOL thread_mutex_is_locked(mutex_t *mtx)
{
    return (mutex_is_locked(mtx));
}

void thread_destroy_mutex(mutex_t *mtx)
{
    if (!mtx) {
        return;
    }

    if (thread_mutex_is_locked(mtx)) {
        return;
    }

    if (!done_mutex(mtx)) {
        kerrprintf("Cannot destroy Mutex %s\n", ((struct mutex *) mtx)->name);
    }
}
