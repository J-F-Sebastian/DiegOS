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

#include <diegos/devices.h>
#include <diegos/interrupts.h>
#include <diegos/kernel_ticks.h>
#include <errno.h>

#include "clock.h"
#include "kprintf.h"

/*
 * ticks per seconds measured as DEFAULT_CLOCK_RES per second
 */
static uint64_t ticks = 0;

/*
 * ticks expressed as milliseconds
 */
static uint64_t ticks_msecs = 0;

/*
 * boot ticks can be set to configure the current time and date
 */
static uint64_t boot_ticks = 0;

/*
 * clock period
 */
static unsigned period = 1000/DEFAULT_CLOCK_RES;

/*
 * clock delay (period) range in milliseconds
 */
static unsigned minmax[2] = {1000/DEFAULT_CLOCK_RES, 1000/DEFAULT_CLOCK_RES};

/*
 * callbacks invoked after ticks accounting
 */
static kernel_clock_cb callbacks[4] = {};

/*
 * clock instance made global, so runtime calls to change frequency
 * are much faster
 */
static device_t *clock = NULL;

/*
 * structure used to compute the amount of milliseconds passed,
 * this is a dynamic
 */
static struct ticks_incr {
    uint32_t    increment_msecs_per_tick;
    uint32_t    err_nsecs_per_tick;
    uint32_t    total_err_per_tick;
} ticks_incr_var;

/*
 * CLK device interrupt handler
 */
static void clock_int_handler (void)
{
    unsigned i;

    ticks_msecs += ticks_incr_var.increment_msecs_per_tick;
    ticks_incr_var.total_err_per_tick += ticks_incr_var.err_nsecs_per_tick;
    if (ticks_incr_var.total_err_per_tick > 999) {
        ++ticks_msecs;
        ticks_incr_var.total_err_per_tick -= 1000;
    }
    ticks = clock_convert_msecs_to_ticks(ticks_msecs);

    for (i = 0; i < NELEMENTS(callbacks); i++) {
        if (callbacks[i]) callbacks[i](ticks_msecs);
    }
    if (ticks % DEFAULT_CLOCK_RES == 0) kprintf("ticks %lld\tseconds %d\n", ticks, clock_get_seconds());
}

static inline void ticks_incr_reset (unsigned ms)
{
	ticks_incr_var.increment_msecs_per_tick = ms;
	ticks_incr_var.err_nsecs_per_tick = 0;
	ticks_incr_var.total_err_per_tick = 0;
}

static inline void ticks_incr_update (unsigned ms)
{
	ticks_incr_var.increment_msecs_per_tick = ms;
	ticks_incr_var.err_nsecs_per_tick = 0;
}

/*
 * Dependencies: devices, drivers
 */
BOOL clock_init (void)
{
    /*
     * Any init value is fine, as the return code is NOT an errno
     */
    int retcode = EINVAL;
    unsigned i = 1000/DEFAULT_CLOCK_RES;

    clock = device_lookup(DEV_CLK, 0);

    if (clock) {
        retcode = clock->cmn->ioctrl_fn(clock_int_handler, CLK_SET_CB, 0);
        if (EOK != retcode) {
            return (FALSE);
        }
	retcode = clock->cmn->ioctrl_fn(minmax, CLK_GET_PERIODS, 0);
        if (EOK != retcode) {
            minmax[0] = minmax[1] = 1000/DEFAULT_CLOCK_RES;
        }
	if ((i < minmax[0]) || (i > minmax[1])) {
	    return (FALSE);
	}
        retcode = clock->cmn->ioctrl_fn(&i, CLK_SET_PERIOD, 0);
    }

    ticks_incr_reset(i);

    return ((EOK == retcode) ? TRUE : FALSE);
}

BOOL clock_add_cb (kernel_clock_cb cb)
{
    unsigned i;

    lock();

    for (i = 0; i < NELEMENTS(callbacks); i++) {
        if (!callbacks[i]) {
            callbacks[i] = cb;
            unlock();
            return (TRUE);
        }
    }

    unlock();

    return (FALSE);
}

BOOL clock_del_cb (kernel_clock_cb cb)
{
    unsigned i;

    lock();

    for (i = 0; i < NELEMENTS(callbacks); i++) {
        if (cb == callbacks[i]) {
            callbacks[i] = NULL;
            unlock();
            return (TRUE);
        }
    }

    unlock();

    return (FALSE);
}

uint64_t clock_get_ticks (void)
{
    return (ticks);
}

uint64_t clock_get_seconds (void)
{
    return (ticks_msecs/1000);
}

uint64_t clock_get_milliseconds (void)
{
    return (ticks_msecs);
}

void clock_set_boot_seconds (unsigned seconds)
{
    boot_ticks = (uint64_t)seconds*DEFAULT_CLOCK_RES;
}

unsigned clock_get_boot_seconds (void)
{
    return (unsigned)(boot_ticks/DEFAULT_CLOCK_RES);
}

uint64_t clock_convert_msecs_to_ticks (unsigned msecs)
{
    uint64_t retval = (uint64_t)msecs;

    retval *= DEFAULT_CLOCK_RES;
    retval /= 1000;

    return retval;
}

static BOOL clock_set_mode (int mode)
{
    int retcode = EINVAL;
    int i = 1000/DEFAULT_CLOCK_RES;

    if (clock) {
        retcode = clock->cmn->ioctrl_fn(&mode, CLK_SET_MODE, 0);
        if (EOK != retcode) {
            return (FALSE);
        }
        retcode = clock->cmn->ioctrl_fn(&i, CLK_SET_PERIOD, 0);
    }

    return ((EOK == retcode) ? TRUE : FALSE);
}

BOOL clock_set_periodic (void)
{
	return clock_set_mode(0);
}

BOOL clock_set_oneshot (void)
{
	return clock_set_mode(1);
}

BOOL clock_set_period (unsigned ms)
{
    int retcode = EINVAL;

    if (clock) {
	if ((ms < minmax[0]) || (ms > minmax[1])) {
            return FALSE;
        }

        retcode = clock->cmn->ioctrl_fn(&ms, CLK_SET_PERIOD, 0);
	if (EOK == retcode) {
            period = ms;
	    lock();
	    ticks_incr_update(ms);
	    unlock();
        }
    }

    return ((EOK == retcode) ? TRUE : FALSE);
}
