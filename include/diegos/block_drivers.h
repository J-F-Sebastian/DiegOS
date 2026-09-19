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

#ifndef BLOCK_DRIVERS_H_INCLUDED
#define BLOCK_DRIVERS_H_INCLUDED

#include <stdint.h>

/*
 * Do not include this file directly - include drivers.h
 */

 typedef struct block_request {
	/*
	 * Blocks numbering starts at 0
	 */
	uint64_t start_block;
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
	 * Disk number for a specific controller
	 */
	uint16_t disk;
	/*
	 * READ 0
	 * WRITE 1
	 */
	uint16_t cmd;
	/*
	 * Errors counter ?
	 */
	int errors;
 } block_request_t;

typedef struct block_driver {
	/*
	 * Common functionalities
	 */
	driver_header_t cmn;
	/*
	 * block size in bytes
	 */
	unsigned block_size;
	/*
	 * total number of blocks controlled by this driver
	 */
	uint64_t block_count;
	/*
	 * Write function, the data buffer pointed by buf will be output to the device
	 */
	int (*write_fn)(block_request_t *req, unsigned unitno);
	/*
	 * Read function, the data buffer pointed by buf will be written with data
	 */
	int (*read_fn)(block_request_t *req, unsigned unitno);
	/*
	 * Multi Write function, the data buffer list pointed by buf
	 * will be output to the device
	 */
	int (*write_multi_fn)(block_request_t **reqs, unsigned reqnum, unsigned unitno);
	/*
	 * Multi Read function, the data buffer list pointed by buf
	 * will be written with data
	 */
	int (*read_multi_fn)(block_request_t **reqs, unsigned reqnum, unsigned unitno);

} block_driver_t;

#endif
