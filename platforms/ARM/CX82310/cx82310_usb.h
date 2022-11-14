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

#ifndef _CX82310_USB_H_
#define _CX82310_USB_H_

/*
 * All registers are 32 bit wide, except for U0_DMA --> U3_DMA which are
 * 64 bit wide.
 */

#define CX82310USB_BASE 0x330000UL

enum CX82310USB {
	U0_DMA,
	U1_DMA = U0_DMA + 2,
	U2_DMA = U1_DMA + 2,
	U3_DMA = U2_DMA + 2,
	UT_DMA = U3_DMA + 2,
	U_CFG,
	U_IDAT,
	U_CTR1,
	U_CTR2,
	U_CTR3,
	U_STAT,
	U_IER,
	U_STAT2,
	U_IER2,
	EP0_IN_TX_INC,
	EP0_IN_TX_PEND,
	EP0_IN_TX_QWCNT,
	EP1_IN_TX_INC,
	EP1_IN_TX_PEND,
	EP1_IN_TX_QWCNT,
	EP2_IN_TX_INC,
	EP2_IN_TX_PEND,
	EP2_IN_TX_QWCNT,
	EP3_IN_TX_INC,
	EP3_IN_TX_PEND,
	EP3_IN_TX_QWCNT,
	EP_OUT_RX_DEC,
	EP_OUT_RX_PEND,
	EP_OUT_RX_QWCNT,
	EP_OUT_RX_BUFSIZE,
	U_CSR,
	UDC_TSR,
	UDC_STAT,
	USB_RXTIMER,
	USB_RXTIMERCNT,
	EP_OUT_RX_PENDLEVEL
};

#endif
