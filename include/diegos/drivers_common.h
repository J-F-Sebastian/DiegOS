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

#ifndef _DRIVERS_COMMON_H_
#define _DRIVERS_COMMON_H_

/*
 * Do not include this file directly - include drivers.h
 */

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
	DRV_STATUS_RUN = (1 << 2),
	DRV_STATUS_STOP = (1 << 3),
	DRV_READING = (1 << 4),
	DRV_WRITING = (1 << 5),
	DRV_STATUS_BUSY = (DRV_READING | DRV_WRITING),
	DRV_CTRL = (1 << 6),
	DRV_READ_BLOCK = (1 << 8),
	DRV_WRITE_BLOCK = (1 << 9),
	DRV_IS_CHAR = (1 << 16),
	DRV_IS_BLOCK = (1 << 17),
	DRV_IS_NET = (1 << 18),
	DRV_IS_TXT_UI = (1 << 19),
	DRV_IS_GFX_UI = (1 << 20)
};

typedef struct driver_header {
	/*
	 * Name of the driver, to be filled at compile time.
	 */
	char name[DRV_NAME_LEN + 1];
	/*
	 * Init function, it is to be invoked at startup to discover the supported devices,
	 * assign resources for all units, and init the device.
	 * Note: init does not mean start, run, enable interrupts, enable DMA.
	 *       The kernel will not be able to handle events to or from devices until
	 *       the start_fn function is invoked.
	 *       The device should be configured to be ready to start working but kept
	 *       reset or idle.
	 * Return values:
	 * EOK in case of success (device unit inited)
	 * ENXIO in case the unitno is not supported or inited
	 * other errnos for failures
	 */
	int (*init_fn) (unsigned unitno);
	/*
	 * Start the device's operations. This include interrupt handlers, DMA, I/O activities.
	 * Return values:
	 * EOK in case of success (device unit inited)
	 * ENXIO in case the unitno is not supported or inited
	 * other errnos for failures
	 */
	int (*start_fn) (unsigned unitno);
	/*
	 * Stop the device's operations. This include I/O activities as well as events, but the
	 * device should not fail or be reset. Calling start_fn again the device should recover
	 * gracefully and start working.
	 * Return values:
	 * EOK in case of success (device unit inited)
	 * ENXIO in case the unitno is not supported or inited
	 * other errnos for failures
	 */
	int (*stop_fn) (unsigned unitno);
	/*
	 * This function should shut down the device. It will be invoked on shutdown/reboot
	 * of the kernel.
	 * Return values:
	 * EOK in case of success (device unit inited)
	 * ENXIO in case the unitno is not supported or inited
	 * other errnos for failures
	 */
	int (*done_fn) (unsigned unitno);
	/*
	 * Retrieve the driver status
	 */
	unsigned (*status_fn) (unsigned unitno);
	/*
	 * I/O control, driver specific
	 * Return values:
	 * EOK in case of success (device unit inited)
	 * ENXIO in case the unitno is not supported or inited
	 * EINVAL in case the opcode is invalid or data is NULL
	 * other errnos for failures
	 */
	int (*ioctrl_fn) (void *data, unsigned opcode, unsigned unitno);
	/*
	 * Implement poll.
	 * Return the poll status.
	 */
	short (*poll_fn) (poll_table_t * table);
} driver_header_t;

#endif
