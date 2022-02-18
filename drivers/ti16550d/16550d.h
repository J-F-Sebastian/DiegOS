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

#ifndef _16550D_H_
#define _16550D_H_

#include <diegos/drivers.h>

extern char_driver_t uart16550d_drv;

enum uart_speed {
	/* 0 */
	BPS_50,
	/* 1 */
	BPS_110,
	/* 2 */
	BPS_150,
	/* 3 */
	BPS_300,
	/* 4 */
	BPS_600,
	/* 5 */
	BPS_1200,
	/* 6 */
	BPS_2400,
	/* 7 */
	BPS_4800,
	/* 8 */
	BPS_9600,
	/* 9 */
	BPS_19200,
	/* 10 */
	BPS_38400,
	/* 11 */
	BPS_57600,
	/* 12 */
	BPS_115200
};

enum uart_bits {
	CHAR_5BITS = (0 << 0),
	CHAR_6BITS = (1 << 0),
	CHAR_7BITS = (2 << 0),
	CHAR_8BITS = (3 << 0),
	STOP_BIT = (1 << 2),
	PARITY_BIT = (1 << 3),
	PARITY_EVEN = (1 << 5),
	STICKY_BIT = (1 << 6)
};

#endif
