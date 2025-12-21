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
#include <libs/kernel_time.h>
#include <errno.h>
#include <string.h>
#include "clock.h"
#include "kprintf.h"

/*
 * boot ticks can be set to configure the current time and date
 */
static uint64_t boot_ticks = 0;

/*
 *  master clock period in clock ticks
 */
static uint32_t period = 0;

/*
 * clock mode (defaults to PERIODIC)
 */
static unsigned mode = 0;

/*
 * callbacks invoked after ticks accounting
 */
static kernel_clock_cb callbacks[CLK_INST_MAX];

/*
 * periods associated to callback instances
 */
static uint32_t inst_period[CLK_INST_MAX];

/*
 * clock instance made global, so runtime calls to change frequency
 * are much faster
 */
static device_t *clock = NULL;

/*
 * System ticks structure used with kernel_time lib
 */
static struct time_util sys_ticks;

/*
 * CLK device interrupt handler
 */
static void clock_int_handler(void)
{
	unsigned i;

	kernel_time_update_elapsed_counter(period, &sys_ticks);

	for (i = 0; i < NELEMENTS(callbacks); i++) {
		if (callbacks[i])
			callbacks[i] (kernel_time_get_elapsed_msecs(&sys_ticks));
	}
	if (mode == 1) {
		if (EOK != clock->cmn->ioctrl_fn(&period, CLK_SET_PERIOD, 0))
			kprintf("FAILED\n");
	}
}

/*
 * Dependencies: devices, drivers
 */
BOOL clock_init(void)
{
	uint32_t params[3] = { 0, 0, 0 };
	unsigned i;

	for (i = 0; i < NELEMENTS(callbacks); i++) {
		callbacks[i] = NULL;
		inst_period[i] = -1U;
	}

	clock = device_lookup(DEV_CLK, 0);

	if (!clock)
		return (FALSE);

	if (EOK != clock->cmn->ioctrl_fn(clock_int_handler, CLK_SET_CB, 0)) {
		return (FALSE);
	}

	if (EOK != clock->cmn->ioctrl_fn(params, CLK_GET_PARAMS, 0)) {
		return (FALSE);
	}

	if (EOK != kernel_time_init(params[0], params[1], params[2], &sys_ticks)) {
		return (FALSE);
	}

	if (EOK != clock->cmn->ioctrl_fn(&mode, CLK_SET_MODE, 0))
		return (FALSE);

	period = kernel_time_get_value(1000 / DEFAULT_CLOCK_RES, &sys_ticks);

	if (EOK != clock->cmn->ioctrl_fn(&period, CLK_SET_PERIOD, 0))
		return (FALSE);

	return (TRUE);
}

BOOL clock_add_cb(kernel_clock_cb cb, enum clock_client_id id)
{
	if (id < CLK_INST_MAX) {

		lock();

		if (!callbacks[id]) {
			callbacks[id] = cb;
			unlock();
			return (TRUE);
		}

		unlock();
	}

	return (FALSE);
}

BOOL clock_del_cb(kernel_clock_cb cb, enum clock_client_id id)
{
	if (id < CLK_INST_MAX) {

		lock();

		if (cb == callbacks[id]) {
			callbacks[id] = NULL;
			unlock();
			return (TRUE);
		}

		unlock();
	}

	return (FALSE);
}

static BOOL clock_set_mode(unsigned newmode)
{
	int retcode = EINVAL;

	if (mode == newmode)
		return (TRUE);

	if (clock) {
		lock();
		retcode = clock->cmn->ioctrl_fn(&newmode, CLK_SET_MODE, 0);
		if (EOK != retcode) {
			unlock();
			return (FALSE);
		}
		mode = newmode;
		retcode = clock->cmn->ioctrl_fn(&period, CLK_SET_PERIOD, 0);
		unlock();
	}

	return ((EOK == retcode) ? TRUE : FALSE);
}

BOOL clock_set_periodic(void)
{
	return clock_set_mode(0);
}

BOOL clock_set_oneshot(void)
{
	return clock_set_mode(1);
}

static inline uint32_t get_lowest()
{
	uint32_t retval = inst_period[0];
	unsigned i;

	for (i = 1; i < NELEMENTS(inst_period); i++)
		if (inst_period[i] < retval)
			retval = inst_period[i];

	return retval;
}

BOOL clock_set_period(unsigned ms, enum clock_client_id instance)
{
	int retcode = EINVAL;
	uint32_t temp;

	if (!(instance < CLK_INST_MAX))
		return FALSE;

	/* adjust ms value to be in range */
	ms = kernel_time_adjust_msecs(ms, &sys_ticks);
	/* store the new period as a clock value for this specific instance */
	inst_period[instance] = kernel_time_get_value(ms, &sys_ticks);
	/* select the lowest value to be configured system-wide */
	temp = get_lowest();

	if (temp == period)
		return TRUE;

	if (clock) {
		/*
		 * A reliable method to compensate lost ticks is still required !!!
		 */
		lock();
		retcode = clock->cmn->ioctrl_fn(&temp, CLK_SET_PERIOD, 0);
		unlock();
		if (EOK == retcode) {
			period = temp;
		}
	}

	return ((EOK == retcode) ? TRUE : FALSE);
}

uint64_t clock_get_ticks(void)
{
	uint64_t temp = kernel_time_get_elapsed_msecs(&sys_ticks);
	return (clock_convert_msecs_to_ticks(temp));
}

uint64_t clock_get_seconds(void)
{
	return (clock_get_milliseconds() / 1000);
}

uint64_t clock_get_milliseconds(void)
{
	return (kernel_time_get_elapsed_msecs(&sys_ticks));
}

void clock_set_boot_seconds(unsigned seconds)
{
	boot_ticks = (uint64_t) seconds *DEFAULT_CLOCK_RES;
}

unsigned clock_get_boot_seconds(void)
{
	return (unsigned)(boot_ticks / DEFAULT_CLOCK_RES);
}

uint64_t clock_convert_msecs_to_ticks(unsigned msecs)
{
	uint64_t retval = (uint64_t) msecs;

	retval *= DEFAULT_CLOCK_RES;
	retval /= 1000;

	return retval;
}
