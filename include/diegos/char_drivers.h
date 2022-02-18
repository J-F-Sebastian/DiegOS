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
	 * Common functionalities
	 */
	driver_header_t cmn;
	/*
	 * Write function, the data buffer pointed by buf will be output to the device
	 */
	int (*write_fn) (const void *buf, unsigned bytes, unsigned unitno);
	/*
	 * Read function, the data buffer pointed by buf will be written with data
	 */
	int (*read_fn) (void *buf, unsigned bytes, unsigned unitno);
	/*
	 * Multi Write function, the data buffer list pointed by buf
	 * will be output to the device
	 */
	int (*write_multi_fn) (const void **buf,
			       const unsigned *bytes, unsigned items, unsigned unitno);
	/*
	 * Multi Read function, the data buffer list pointed by buf
	 * will be written with data
	 */
	int (*read_multi_fn) (void **buf, unsigned *bytes, unsigned items, unsigned unitno);

} char_driver_t;

#endif				// CHAR_DRIVERS_H_INCLUDED
