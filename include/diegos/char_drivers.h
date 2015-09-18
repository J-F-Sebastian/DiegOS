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

#ifndef CHAR_DRIVERS_H_INCLUDED
#define CHAR_DRIVERS_H_INCLUDED

/*
 * Do not include this file directly - include drivers.h
 */

typedef struct char_driver {
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
    int (*init_fn)(unsigned unitno);
    /*
     * Start the device's operations. This include interrupt handlers, DMA, I/O activities.
     */
    int (*start_fn)(unsigned unitno);
    /*
     * Stop the device's operations. This include I/O activities as well as events, but the
     * device should not fail or be reset. Calling start_fn again the device should recover
     * gracefully and start working.
     */
    int (*stop_fn)(unsigned unitno);
    /*
     * This function should shut down the device. It will be invoked on shutdown/reboot
     * of the kernel.
     */
    int (*done_fn)(unsigned unitno);
    /*
     * Retrieve the driver status
     */
    unsigned (*status_fn)(unsigned unitno);
    /*
     * Write function, the data buffer pointed by buf will be output to the device
     */
    int (*write_fn)(const void *buf, unsigned bytes, unsigned unitno);
    /*
     * Read function, the data buffer pointed by buf will be written with data
     */
    int (*read_fn)(void *buf, unsigned bytes, unsigned unitno);
    /*
     * Multi Write function, the data buffer list pointed by buf
     * will be output to the device
     */
    int (*write_multi_fn)(const void **buf,
                          const unsigned *bytes,
                          unsigned items,
                          unsigned unitno);
    /*
     * Multi Read function, the data buffer list pointed by buf
     * will be written with data
     */
    int (*read_multi_fn)(void **buf,
                         unsigned *bytes,
                         unsigned items,
                         unsigned unitno);
    /*
     * I/O control, driver specific
     */
    int (*ioctrl_fn)(void *data, unsigned opcode, unsigned unitno);
    /*
     * Implement poll
     */
    short (*poll_fn)(poll_table_t *table);
} char_driver_t;

#endif // CHAR_DRIVERS_H_INCLUDED
