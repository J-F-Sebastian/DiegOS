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

enum i8042_status_register {
	/*
	 * Output Buffer Status, must be set before reading data from DATA_PORT
	 * 0 Output Buffer Empty
	 * 1 Output Buffer Full
	 */
	OBF = (1 << 0),
	/*
	 * Input Buffer Status, must be clear before writing to DATA_PORT or CMD_PORT
	 * 0 Input Buffer Empty
	 * 1 Input Buffer Full
	 */
	IBF = (1 << 1),
	/* System flag, cleared on reset, set if POST is successful */
	SYS = (1 << 2),
	/*
	 * Command or Data
	 * 0 = data written is data for PS/2 device (data byte)
	 * 1 = data written is data for PS/2 controller command (command byte)
	 */
	A2 = (1 << 3),
	/*
	 * Inhibit Switch
	 * 0 = keyboard is inhibited
	 * 1 = keyboard is not inhibited
	 */
	INH = (1 << 4),
	/*
	 * Transmit Time Out
	 * 0 No Transmit Time Out Error
	 * 1 Transmit Time Out Error
	 */
	TTO = (1 << 5),
	/*
	 * Receive Time Out
	 * 0 No Receive Time Out Error
	 * 1 Receive Time Out Error
	 */
	RTO = (1 << 6),
	/*
	 * Parity Error
	 * 0 Odd Parity (No Error)
	 * 1 Even Parity (Error)
	 */
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

#define KEYBOARD_IVT    (32 + KEYBOARD_IRQ)
#define PS2MOUSE_IVT    (32 + PS2MOUSE_IRQ)

#endif
