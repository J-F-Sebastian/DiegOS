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

#ifndef _8042_PRIVATE_H_
#define _8042_PRIVATE_H_

#define i8042_DATA_PORT  (0x60)
#define i8042_CMD_PORT   (0x64)

enum i8042_status_flags {
/* Must be set before reading data, must be clear before writing data */
	OBF = (1 << 0),
/* If set a new data byte can be read */
	IBF = (1 << 1),
/* Cleared on reset, set if POST is successful */
	SYS = (1 << 2),
/*
 * 0 = data written to 0x60
 * 1 = data written to 0x64
 */
	A2 = (1 << 3),
/*
 * 0 = Keyboard Clock OFF, keyboard is inhibited
 * 1 = Keyboard Clock ON, keyboard is not inhibited
 */
	INH = (1 << 4),
/* Mouse output buffer empty or full */
	MOBF = (1 << 5),
/* General Timeout */
	TO = (1 << 6),
/* Communication error with the keyboard */
	PERR = (1 << 7)
};

enum i8042_ctrl_config {
	PORT1_INT = (1 << 0),
	PORT2_INT = (1 << 1),
	SYS_FLAG = (1 << 2),
	PORT1_CLK = (1 << 4),
	PORT2_CLK = (1 << 5),
	PORT1_TSL = (1 << 6)
};

#define READ_CFG_BYTE   (0x20)
#define WRITE_CFG_BYTE  (0x60)
#define DISABLE_PORT2   (0xA7)
#define ENABLE_PORT2    (0xA8)
#define TEST_PORT2      (0xA9)
#define TEST_PORT1      (0xAB)
#define DISABLE_PORT1   (0xAD)
#define ENABLE_PORT1    (0xAE)

#endif
