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

#ifndef _N9H26_UART_H_
#define _N9H26_UART_H_

#include <types_common.h>

/*
 * All registers are 32 bit wide
 */

#define N9H26UART_BASE0 0xB8008000UL
#define N9H26UART_BASE1 0xB8008100UL

enum N9H26UART {
	UA_RBR,
	UA_THR = UA_RBR,
	UA_IER,
	UA_FCR,
	UA_LCR,
	UA_RSV1,
	UA_RSV2,
	UA_FSR,
	UA_ISR,
	UA_TOR,
	UA_BAUD
};

// REG_UART_RBR         (UART_BA+0x00)                  /* Receive Buffer Register */
// REG_UART_THR         (UART_BA+0x00)                  /* Transmit Holding Register */
// REG_UART_IER         (UART_BA+0x04)                  /* Interrupt Enable Register */
#define nDBGACK_EN	BIT32(31)	// ICE debug mode acknowledge enable
#define EDMA_RX_EN	BIT32(15)	// RX EDMA Enable
#define EDMA_TX_EN	BIT32(14)	// TX EDMA Enable
#define Auto_CTS_EN	BIT32(13)	// CTS Auto Flow Control Enable
#define Auto_RTS_EN	BIT32(12)	// RTS Auto Flow Control Enable
#define Time_out_EN	BIT32(11)	// Time Out Counter Enable
#define Wake_IEN	BIT32(6)	// Wake up interrupt enable for INTR[wakeup]
#define BUF_ERR_IEN	BIT32(5)	// Buffer Error interrupt enable
#define RTO_IEN		BIT32(4)	// RX Time out Interrupt Enable
#define MS_IEN		BIT32(3)	// MODEM Status Interrupt (Irpt_MOS) Enable
#define RLS_IEN		BIT32(2)	// Receive Line Status Interrupt (Irpt_RLS) Enable
#define THRE_IEN	BIT32(1)	// Transmit Holding Register Empty Interrupt (Irpt_THRE) Enable
#define RDA_IEN		BIT32(0)	// Receive Data Available Interrupt (Irpt_RDA) Enable and

// REG_UART_FCR         (UART_BA+0x08)                  /* FIFO Control Register */
#define RTS_Tri_lev	BITMASK32(19,16)	// RTS Trigger Level
#define RFITL		BITMASK32(7,4)	// RX FIFO Interrupt (Irpt_RDA) Trigger Level
#define TX_RST		BIT32(2)	// TX Software Reset
#define RX_RST		BIT32(1)	// RX Software Reset

// REG_UART_LCR         (UART_BA+0x0C)                  /* Line Control Register */
#define DLAB		BIT32(7)	// Divider Latch Access Bit
#define BCB		BIT32(6)	// Break Control Bit
#define SPE		BIT32(5)	// Stick Parity Enable.
#define EPE		BIT32(4)	// Even Parity Enable
#define PBE		BIT32(3)	// Parity Bit Enable
#define NSB		BIT32(2)	// Number of "STOP bit"
#define WLS		BITMASK32(1,0)	// Word Length Select.

// REG_UART_MCR         (UART_BA+0x10)                  /* Modem Control Register */
#define RTS_st		BIT32(13)
#define Lev_RTS		BIT32(9)
#define LBME		BIT32(4)	// Loop-back Mode Enable
#define CRTS		BIT32(1)	// Complement version of DTR# (Data-Terminal-Ready) signal

// REG_UART_LSR (UART_BA+0x14)                  /* Line Status Register */
// REG_UART_MSR         (UART_BA+0x14)                  /* MODEM Status Register */
#define Lev_CTS		BIT32(8)
#define DCD		BIT32(7)	// Complement version of Data Carrier Detect (nDCD#) input
#define RI		BIT32(6)	// Complement version of ring indicator (RI#) input
#define DSR		BIT32(5)	// Complement version of data set ready (DSR#) input
#define CTS_st		BIT32(4)	// Complement version of clear to send (CTS#) input
#define DDCD		BIT32(3)	// DCD# State Change
#define TERI		BIT32(2)	// Tailing Edge of RI#
#define DDSR		BIT32(1)	// DSR# State Change
#define DCTS		BIT32(0)	// CTS# State Change

// REG_UART_FSR         (UART_BA+0x18)          /* FIFO Status Register */
#define Tx_err_Flag	BIT32(31)	// Framing Error Indicator
#define TE_Flag		BIT32(28)	// Parity Error Indicator
#define Tx_Over_IF	BIT32(24)	// RX overflow Error IF
#define Tx_Full		BIT32(23)	// Transmitter FIFO Full
#define Tx_Empty	BIT32(22)	// Transmitter FIFO Empty
#define Tx_Pointer	BITMASK32(21,16)	// TX FIFO pointer
#define Rx_Full		BIT32(15)	// Receiver FIFO Full
#define Rx_Empty	BIT32(14)	// Receiver FIFO Empty
#define Rx_Pointer	BITMASK32(13,8)	// RX FIFO pointer
#define Rx_err_IF	BIT32(7)	// RX Error flag
#define BII		BIT32(6)	// Break Interrupt Indicator
#define FEI		BIT32(5)	// Framing Error Indicator
#define PEI		BIT32(4)	// Parity Error Indicator
#define Rx_Over_IF	BIT32(0)	// RX overflow Error IF

// REG_UART_ISR         (UART_BA+0x1C)                  /* Interrupt Status Register */
#define EDMA_RX_Flag	BIT32(31)	// EDMA RX Mode Flag
#define HW_Wake_INT	BIT32(30)	// Wake up Interrupt pin status
#define HW_Buf_Err_INT	BIT32(29)	// Buffer Error Interrupt pin status
#define HW_Tout_INT	BIT32(28)	// Time out Interrupt pin status
#define HW_Modem_INT	BIT32(27)	// MODEM Status Interrupt pin status
#define HW_RLS_INT	BIT32(26)	// Receive Line Status Interrupt pin status
#define Rx_ack_st	BIT32(25)	// TX ack pin status
#define Rx_req_St	BIT32(24)	// TX req pin status
#define EDMA_TX_Flag	BIT32(23)	// EDMA TX Mode Flag
#define HW_Wake_IF	BIT32(22)	// Wake up Flag
#define	HW_Buf_Err_IF	BIT32(21)	// Buffer Error Flag
#define HW_Tout_IF	BIT32(20)	// Time out Flag
#define HW_Modem_IF	BIT32(19)	// MODEM Status Flag
#define HW_RLS_IF	BIT32(18)	// Receive Line Status Flag
#define Tx_ack_st	BIT32(17)	// TX ack pin status
#define Tx_req_St	BIT32(16)	// TX req pin status
#define Soft_RX_Flag	BIT32(15)	// Software RX Mode Flag
#define Wake_INT	BIT32(14)	// Wake up Interrupt pin status
#define Buf_Err_INT	BIT32(13)	// Buffer Error Interrupt pin status
#define Tout_INT	BIT32(12)	// Time out interrupt Interrupt pin status
#define Modem_INT	BIT32(11)	// MODEM Status Interrupt pin status
#define RLS_INT		BIT32(10)	// Receive Line Status Interrupt pin status
#define THRE_INT	BIT32(9)	// Transmit Holding Register Empty Interrupt pin status
#define RDA_INT		BIT32(8)	// Receive Data Available Interrupt pin status
#define Soft_TX_Flag	BIT32(7)	// Software TX Mode Flag
#define Wake_IF		BIT32(6)	// Wake up Flag
#define Buf_Err_IF	BIT32(5)	// Buffer Error Flag
#define Tout_IF		BIT32(4)	// Time out interrupt Flag
#define Modem_IF	BIT32(3)	// MODEM Status Flag
#define RLS_IF		BIT32(2)	// Receive Line Status Flag
#define THRE_IF		BIT32(1)	// Transmit Holding Register Empty Flag
#define RDA_IF		BIT32(0)	// Receive Data Available Flag

// REG_UART_TOR         (UART_BA+0x20)                  /* Time Out Register */
#define TOIC		BITMASK32(6,0)	//Time Out Interrupt Comparator

// REG_UART_BAUD                (UART_BA+0x24)                  /* Baud Rate Divider Register */
#define DIV_X_EN		BIT32(29)	// Divisor X Enable
#define DIV_X_ONE		BIT32(28)	// Divisor X equal 1
#define Divider_X		BITMASK32(27,24)	// Disisor X
#define BAUD_RATE_DIVISOR	BITMASK32(15,0)	// Baud Rate Divisor

#endif
