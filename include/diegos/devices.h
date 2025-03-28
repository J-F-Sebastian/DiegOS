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

#ifndef _DEVICES_H_
#define _DEVICES_H_

#include <types_common.h>
#include <stddef.h>
#include <diegos/drivers.h>

#define DEV_NAME_LEN (7)

enum DEVICE_TYPE {
	/*
	 * Device is a character stream device
	 */
	DEV_TYPE_CHAR,
	/*
	 * Device is a block device
	 */
	DEV_TYPE_BLOCK,
	/*
	 * Device is a graphical video card
	 */
	DEV_TYPE_GFX_UI,
	/*
	 * Device is a text video card
	 */
	DEV_TYPE_TXT_UI
};

/*
 * Read and write function prototypes.
 * Declare these and set them into a device by calling device_set_access()
 * to specialize the devices behavior.
 *
 * PARAMETERS IN
 * const void *buf - the source buffer
 * size_t bytes - the amount of data in bytes to be transferred to the driver
 * void *drv - a driver instance
 * unsinged unitno - the unit number of the driver
 *
 * RETURNS
 * EINVAL if buf or drv are NULL
 * The number of transferred bytes in case of success; zero is a valid amount of transferred bytes.
 * Any error code returned from the driver function call
 */
typedef int (*device_write_fn)(const void *buf, size_t bytes, void *drv, unsigned unitno);
/*
 * PARAMETERS IN
 * void *buf - the destination buffer
 * size_t bytes - the amount of data in bytes to be transferred from the driver
 * void *drv - a driver instance
 * unsinged unitno - the unit number of the driver
 *
 * RETURNS
 * EINVAL if buf or drv are NULL
 * The number of transferred bytes in case of success; zero is a valid amount of transferred bytes.
 * Any error code returned from the driver function call
 */
typedef int (*device_read_fn)(void *buf, size_t bytes, void *drv, unsigned unitno);

typedef struct device_header {
	/*
	 * Name of the device, to be filled at creation time.
	 */
	char name[DEV_NAME_LEN + 1];
	/*
	 * The driver's unit number used by this device
	 */
	unsigned unitno;
	/*
	 * Device type, from enum DEVICE_TYPE
	 */
	unsigned type;
	/*
	 * Write function, if set, is invoked in place of raw driver write access
	 */
	device_write_fn write_fn;
	/*
	 * Read function, if set, is invoked in place of raw driver read access
	 */
	device_read_fn read_fn;

} device_header_t;

typedef struct device {
	device_header_t header;
	union {
		void *drv;
		driver_header_t *cmn;
		char_driver_t *cdrv;
		text_driver_t *tdrv;
		grafics_driver_t *gdrv;
		net_driver_t *ndrv;
	};
} device_t;

/*
 * Called by platform init routines, device_create() return a pointer to
 * the newly created device.
 * Devices are linked in a doubly-linked list.
 *
 * PARAMETERS IN
 * const void *inst - a pointer to a driver interface.
 * unsigned unitno  - the device unit.
 *
 * RETURNS
 * A valid pointer to a new device object in case of success, NULL in any other
 * case.
 */
device_t *device_create(const void *inst, unsigned unitno);

/*
 * device_lookup() retrieves a pointer to a device.
 * The search key is the driver's name and the unit number.
 *
 * PARAMETERS IN
 * const char *drvname - the driver name.
 * unsigned unitno - the device unit.
 *
 * RETURNS
 * A valid pointer to an existing device object in case of success,
 * NULL in any other case.
 */
device_t *device_lookup(const char *drvname, unsigned unitno);

/*
 * device_lookup_name() retrieves a pointer to a device.
 * The search key is the full device's name (name plus unit).
 *
 * PARAMETERS IN
 * const char *devname - the device name.
 *
 * RETURNS
 * A valid pointer to an existing device object in case of success,
 * NULL in any other case.
 */
device_t *device_lookup_name(const char *devname);

/*
 * Perform writing to devices. If the driver support suspending (blocking)
 * write operations and the driver is busy, the thread will suspend and
 * wait for completion.
 * If the device is busy EAGAIN error code is returned.
 *
 * PARAMETERS IN
 * device_t *dev - the device performing I/O
 * char *buf     - the data buffer to be transmitted by the device
 * size_t bytes  - the size of the data buffer
 *
 * RETURNS
 * EINVAL if parameters are not valid
 * EAGAIN if the device is busy transmitting data
 * EIO if the driver failed to complete the write operation
 * EPERM if the device is not a character stream device
 * A value greater than or equal to 0 in case of successful transmission, the
 * returned value is the amount of bytes transmitted (it might not match the
 * value passed in with the parameter bytes).
 */
int device_io_tx(device_t * dev, const char *buf, size_t bytes);

/*
 * Perform reading from devices. If the driver support suspending (blocking)
 * read operations and the driver is busy or has no data to read, the thread
 * will suspend and wait for completion.
 * If the device is busy EAGAIN error code is returned.
 *
 * PARAMETERS IN
 * device_t *dev - the device performing I/O
 * char *buf     - the data buffer to store data read from the device
 * size_t bytes  - the size of the data buffer
 *
 * RETURNS
 * EINVAL if parameters are not valid
 * EAGAIN if the device is busy reading data
 * EIO if the driver failed to complete the read operation
 * EPERM if the device is not a character stream device
 * A value greater than or equal to 0 in case of successful read, the
 * returned value is the amount of bytes read in the buffer (it might not match
 * the value passed in with the parameter bytes).
 */
int device_io_rx(device_t * dev, char *buf, size_t bytes);

#endif
