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

#include "char_drivers.h"
#include "net_drivers.h"
#include "ui_drivers.h"

/*
 * The following enum values are returned by
 * the status_fn function call to assert the driver
 * overall status, not the status of a single op.
 * Blocking flags mark those drivers that will suspend
 * and resume the calling thread on reading or writing or both.
 */
enum {
    DRV_STATUS_DONE = (1 << 0),
    DRV_STATUS_FAIL = (1 << 1),
    DRV_STATUS_RUN  = (1 << 2),
    DRV_STATUS_STOP = (1 << 3),
    DRV_READING     = (1 << 4),
    DRV_WRITING     = (1 << 5),
    DRV_STATUS_BUSY = (DRV_READING | DRV_WRITING),
    DRV_CTRL        = (1 << 6),
    DRV_READ_BLOCK  = (1 << 8),
    DRV_WRITE_BLOCK = (1 << 9),
    DRV_IS_CHAR     = (1 << 16),
    DRV_IS_BLOCK    = (1 << 17),
    DRV_IS_NET      = (1 << 18)
};

/*
 * Predefined, system-wide ioctrl values.
 * Adhering to these interfaces avoid platform dependencies
 */
#define DEV_RTC "rtc"

enum rtc_ioctrl {
    /* Set rtc ticks in milliseconds; 1 unsigned as parameter */
    RTC_SET_FREQ,
    /*
     * Set the callback to be invoked when the rtc expires; 1 pointer
     * to function as parameter (i.e. void (*fn)(void) )
     */
    RTC_SET_CB
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
int char_drivers_list_init(char_driver_t *drvlist[], unsigned drvlistsize);

int driver_def_ok(unsigned unitno);

int driver_def_error(unsigned unitno);

int kdrvprintf(const char *fmt, ...);

#endif
