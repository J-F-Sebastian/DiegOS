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

static uint64_t ticks = 0;
static uint64_t ticks_msecs = 0;
/* boot ticks can be set to configure the current time and date */
static uint64_t boot_ticks = 0;

struct ticks_incr {
    uint32_t    increment_msecs_per_tick;
    uint32_t    err_per_tick;
    uint32_t    total_err_per_tick;
} ticks_incr_var;

static kernel_clock_cb callbacks[4] = {};

static void clock_int_handler(void)
{
    unsigned i;

    ++ticks;
    ticks_msecs += ticks_incr_var.increment_msecs_per_tick;
    ticks_incr_var.total_err_per_tick += ticks_incr_var.err_per_tick;
    if (ticks_incr_var.total_err_per_tick > DEFAULT_CLOCK_RES) {
        ++ticks_msecs;
        ticks_incr_var.total_err_per_tick = 0;
    }

    for (i = 0; i < NELEMENTS(callbacks); i++) {
        if (callbacks[i]) callbacks[i](ticks);
    }
}

/*
 * Dependencies: devices, drivers
 */
BOOL clock_init()
{
    /*
     * Any init value is fine, as the return code is NOT an errno
     */
    int retcode = EINVAL;
    device_t *clock = device_lookup(DEV_RTC, 0);
    int i = DEFAULT_CLOCK_RES;

    if (clock) {
        retcode = clock->cmn->ioctrl_fn(clock_int_handler, RTC_SET_CB, 0);
        if (EOK != retcode) {
            return (FALSE);
        }
        retcode = clock->cmn->ioctrl_fn(&i, RTC_SET_FREQ, 0);
    }

    ticks_incr_var.increment_msecs_per_tick = 1000/DEFAULT_CLOCK_RES;
    ticks_incr_var.err_per_tick = 1000%DEFAULT_CLOCK_RES;
    ticks_incr_var.total_err_per_tick = 0;

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

    /*
     * If a system tick lasts for a millisecond or more,
     * divide the required msecs by the msecs_per_tick.
     * If a system tick lasts for less than a millisecond,
     * use the fractional err_per_tick. Return value might be 0.
     */
    if (ticks_incr_var.increment_msecs_per_tick) {
        return (retval/ticks_incr_var.increment_msecs_per_tick);
    } else {
        return (retval*DEFAULT_CLOCK_RES/ticks_incr_var.err_per_tick);
    }
}
