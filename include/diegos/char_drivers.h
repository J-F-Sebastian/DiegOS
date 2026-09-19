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

typedef struct scatter_gather_request {
	/*
	 * Data buffer
	 */
	void *buffer;
	/*
	 * Amount of bytes to be transferred.
	 * This has no implications with the buffer size.
	 */
	uint32_t bytes;
	/*
	 * Return code for the single request, to be filled by the driver.
	 * EOK in case of success
	 * EAGAIN in case the device is busy and cannot accept more data
	 * EPERM in case the device is not ready to accept data
	 */
	int retcode;
 } sg_request_t;

typedef struct char_driver {
	/*
	 * Common functionalities
	 */
	driver_header_t cmn;
	/*
	 * Write function, the data buffer pointed by buf will be output to the device.
	 *
	 * PARAMETERS IN
	 * const void *buf - pointer to the data buffer to be written
	 * unsigned *bytes - pointer to the number of bytes to be written
	 * unsigned unitno - device unit number
	 *
	 * PARAMETERS OUT
	 * unsigned *bytes - pointer to the number of bytes actually written
	 *
	 * RETURNS
	 * EOK in case the transfer is successful
	 * ENXIO in case the unitno is not supported
	 * EINVAL in case the buffer is NULL
	 * EAGAIN in case the device is busy and cannot accept more data
	 * EPERM in case the device is not ready to accept data
	 */
	int (*write_fn)(const void *buf, unsigned *bytes, unsigned unitno);
	/*
	 * Read function, the data buffer pointed by buf will be written with data.
	 *
	 * PARAMETERS IN
	 * void *buf - pointer to the data buffer to be filled with data read from the device
	 * unsigned *bytes - pointer to the number of bytes to be read
	 * unsigned unitno - device unit number
	 *
	 * PARAMETERS OUT
	 * unsigned *bytes - pointer to the number of bytes actually read
	 *
	 * RETURNS
	 * EOK in case the transfer is successful
	 * ENXIO in case the unitno is not supported
	 * EINVAL in case the buffer is NULL
	 * EAGAIN in case the device is busy and cannot provide more data
	 * EPERM in case the device is not ready to accept data
	 */
	int (*read_fn)(void *buf, unsigned *bytes, unsigned unitno);
	/*
	 * Multi Write function, the data buffers list pointed by buf will be output to the device.
	 * Errors for each request will be stored in the retcode field of the sg_request_t structure.
	 * Data written by the device will be stored in the buffer pointed by the buffer field of the sg_request_t structure.
	 * The amont of bytes actually written will be stored in the bytes field of the sg_request_t structure.
	 *
	 * PARAMETERS IN
	 * sg_request_t *reqs - pointer to the array of data buffers to be written
	 * unsigned items - number of items in the array
	 * unsigned unitno - device unit number
	 *
	 * PARAMETERS OUT
	 * sg_request_t *reqs - pointer to the array of data buffers to be written
	 *
	 * RETURNS
	 * EOK in case the transfer is successful
	 * ENXIO in case the unitno is not supported
	 * EINVAL in case the request list is NULL or items is zero
	 * EAGAIN in case the device is busy and cannot accept more data
	 * EPERM in case the device is not ready to accept data
	 */
	int (*write_multi_fn)(sg_request_t *reqs, unsigned items, unsigned unitno);
	/*
	 * Multi Read function, the data buffers array pointed by buf will be written with data.
	 * Errors for each request will be stored in the retcode field of the sg_request_t structure.
	 * Data read by the device will be stored in the buffer pointed by the buffer field of the sg_request_t structure.
	 * The amont of bytes actually read will be stored in the bytes field of the sg_request_t structure.
	 *
	 * PARAMETERS IN
	 * sg_request_t *reqs - pointer to the array of data buffers to be filled with data read from the device
	 * unsigned items - number of items in the array
	 * unsigned unitno - device unit number
	 *
	 * PARAMETERS OUT
	 * sg_request_t *reqs - pointer to the array of data buffers to be filled with data read from the device
	 *
	 * RETURNS
	 * EOK in case the transfer is successful
	 * ENXIO in case the unitno is not supported
	 * EINVAL in case the request list is NULL or items is zero
	 * EAGAIN in case the device is busy and cannot provide more data
	 * EPERM in case the device is not ready to accept data
	 */
	int (*read_multi_fn)(sg_request_t *reqs, unsigned items, unsigned unitno);

} char_driver_t;

#endif				// CHAR_DRIVERS_H_INCLUDED
