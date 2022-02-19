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

#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <diegos/poll.h>
#include <diegos/if.h>

/* Definitions MUST STAY HERE */

#define DRV_NAME_LEN        (5)
#define DRV_UNIT_MAX        (4)
#define DRV_MAX_ADDR_LEN    (16)

#include "drivers_common.h"
#include "char_drivers.h"
#include "block_drivers.h"
#include "net_drivers.h"
#include "ui_drivers.h"

/*
 * Predefined, system-wide ioctrl values.
 * Adhering to these interfaces avoid platform dependencies
 */
#define DEV_CLK "clk"

enum clk_ioctrl {
	/*
	 * Set clock period, i.e. the inverse of frequency in Hz.
	 * Value is expressed in clock ticks, the time base (i.e. clock ticks
	 * number corresponding to 1 second) is device dependent and can
	 * be retrieved using CLK_GET_PARAMS.
	 * 1 unsigned as parameter.
	 * The clock device starts counting as soon as this IOCTL is
	 * performed, if the clock mode is set to PERIODIC the ticks
	 * delay will change, if the clock mode is set to ONESHOT and
	 * the timer did not expire it will be re-triggered but no interrupt
	 * will be generated.
	 */
	CLK_SET_PERIOD,
	/*
	 * Get clock parameters in clock ticks.
	 * 3 unsigned as parameter.
	 * Values stored are: TIMER_BASE, TIMER_MIN, TIMER_MAX.
	 * They correspond to the number of clock ticks in 1 second,
	 * the minimum programmable number of ticks, and the maximum programmable
	 * number of ticks.
	 */
	CLK_GET_PARAMS,
	/*
	 * Get clock counter.
	 * 1 unsigned as parameter.
	 * The value is the reading of the clock counter, always considered as counting
	 * down to zero, starting from the value configured with CLK_SET_PERIOD.
	 * The value can be interpreted as the remaining number of clock ticks before
	 * expiration.
	 */
	CLK_GET_ELAPSED,
	/*
	 * Set clock mode; 1 unsigned as parameter.
	 * values are
	 * 0 for periodic mode
	 * 1 for one-shot mode
	 */
	CLK_SET_MODE,
	/*
	 * Set the callback to be invoked when the clock expires; 1 pointer
	 * to function as parameter (i.e. void (*fn)(void) )
	 */
	CLK_SET_CB
};

#define DEV_UART "uart"

enum uart_ioctrl {
	/* Set uart speed in bits per second; 1 unsigned as parameter */
	UART_SET_SPEED,
	/* set uart flow control, see uart docs for values */
	UART_SET_FLOW_CTRL,
	/* set uart bit format, see uart docs for values */
	UART_SET_BITS
};

/*
 * Init driver list and init data structures. To be called BEFORE
 * driver_lib_init()
 */
int drivers_list_init(const void *drvlist[], unsigned drvlistsize);

int driver_def_ok(unsigned unitno);

int driver_def_error(unsigned unitno);

int kdrvprintf(const char *fmt, ...);

#endif
