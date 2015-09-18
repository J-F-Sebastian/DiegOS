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

#ifndef _16550D_PRIVATE_H_
#define _16550D_PRIVATE_H_

#include <types_common.h>

enum uart_ports {
    COM1 = 0x3F8,
    COM2 = 0x2F8,
    COM3 = 0x3E8,
    COM4 = 0x2E8
};

/*
 * All 8250/16450/16550 registers are 8 bit long
 */
enum uart_registers {
    /* Transmitter Holding buffer */
    THR = 0,
    /* Receiver Buffer */
    RBR = 0,
    /* Divisor Latch Low byte */
    DLL = 0,
    /* Interrupt Enable */
    IER = 1,
    /* Divisor Latch High byte */
    DLH = 1,
    /* Interrupt Identification */
    IIR = 2,
    /* FIFO Control */
    FCR = 2,
    /* Line Control */
    LCR = 3,
    /* Modem Control */
    MCR = 4,
    /* Line Status */
    LSR = 5,
    /* Modem Status */
    MSR = 6,
    /* Scratch */
    SR = 7
};


/*******************************************************************/
enum uart_ier {
    /* Enable Received Data Available Interrupt */
    IER_ERBFI  = (1 << 0),
    /* Enable Transmitter Holding Register Empty Interrupt */
    IER_ETBEI  = (1 << 1),
    /* Enable Receiver Line Status Interrupt */
    IER_ELSI   = (1 << 2),
    /* Enable MODEM Status Interrupt */
    IER_EDSSI  = (1 << 3)
};

/*******************************************************************/
enum uart_iir {
    IIR_NO_INT_PEND    = (1 << 0),
    /*
     * Interrupts in descending priority order
     * Highest to lowest
     */
    IIR_RCVR_LINE_STAT = (3 << 1),
    IIR_RCV_DATA_AVAIL = (2 << 1),
    IIR_CHAR_TIMEOUT   = (6 << 1),
    IIR_TRSR_HOLD_EMPTY= (1 << 1),
    IIR_MODEM_STATUS   = (0 << 1),
    IIR_NO_FIFO        = (0 << 6),
    IIR_FIFO_NWORK     = (2 << 6),
    IIR_FIFO_ENA       = (3 << 6),
    IIR_INT_MASK       = 0x0E
};

/*******************************************************************/
enum uart_fcr {
    FCR_ENA_FIFOS      = (1 << 0),
    FCR_RCVR_FIFO_RST  = (1 << 1),
    FCR_XMIT_FIFO_RST  = (1 << 2),
    FCR_DMA_MODE       = (1 << 3),
    FCR_FIFO_1B        = (0 << 6),
    FCR_FIFO_4B        = (1 << 6),
    FCR_FIFO_8B        = (2 << 6),
    FCR_FIFO_14B       = (3 << 6)
};

/*******************************************************************/
enum uart_lcr {
    LCR_5_DATA_BITS    = (0 << 0),
    LCR_6_DATA_BITS    = (1 << 0),
    LCR_7_DATA_BITS    = (2 << 0),
    LCR_8_DATA_BITS    = (3 << 0),
    LCR_1_STOP_BIT     = (0 << 2),
    LCR_2_STOP_BIT     = (1 << 2),
    LCR_PARITY_BIT     = (1 << 3),
    LCR_EVEN_PARITY    = (1 << 4),
    LCR_STICKY_BIT     = (1 << 5),
    LCR_BREAK_CTRL     = (1 << 6),
    LCR_DLAB           = (1 << 7)
};

/*******************************************************************/
enum uart_mcr {
    MCR_DTR    = (1 << 0),
    MCR_RTS    = (1 << 1),
    MCR_AUX1   = (1 << 2),
    MCR_AUX2   = (1 << 3),
    MCR_LPBK   = (1 << 4)
};

/*******************************************************************/
enum uart_lsr {
    LSR_DR     = (1 << 0),
    LSR_OE     = (1 << 1),
    LSR_PE     = (1 << 2),
    LSR_FE     = (1 << 3),
    LSR_BI     = (1 << 4),
    LSR_THRE   = (1 << 5),
    LSR_TEMT   = (1 << 6),
    LSR_ERR    = (1 << 7)
};

/*******************************************************************/
enum uart_msr {
    MSR_DCTS   = (1 << 0),
    MSR_DDSR   = (1 << 1),
    MSR_TERI   = (1 << 2),
    MSR_DDCD   = (1 << 3),
    MSR_CTS    = (1 << 4),
    MSR_DSR    = (1 << 5),
    MSR_RI     = (1 << 6),
    MSR_DCD    = (1 << 7)
};

#endif

