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

#ifndef _CX82310_HST_H_
#define _CX82310_HST_H_

/*
 * All registers are 32 bit wide
 */

#define CX82310HST_BASE 0x2D0000UL

enum CX82310HST {
	HST_CTRL,
	HST_RWST,
	HST_WWST,
	HST_XFER_CNTL,
	HST_READ_CNTL1,
	HST_READ_CNTL2,
	HST_WRITE_CNTL1,
	HST_WRITE_CNTL2,
	MSTR_INTF_WIDTH,
	MSTR_HANDSHAKE,
	HDMA_SRC_ADDR,
	HDMA_DST_ADDR,
	HDA_BCNT,
	HDA_TIMERS
};

#endif
