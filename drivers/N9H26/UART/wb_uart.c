/****************************************************************************
 * @file     wb_uart.c
 * @version  V3.00
 * @brief    N9H26 series SYS driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/****************************************************************************
* FILENAME
*   wb_uart.c
*
* VERSION
*   1.0
*
* DESCRIPTION
*   The UART related function of Nuvoton ARM9 MCU
*
* HISTORY
*   2008-06-25  Ver 1.0 draft by Min-Nan Cheng
*   2025-03-15  Ver 2.0 by Diego
*
* REMARK
*   None
 **************************************************************************/

#include <types_common.h>
#include <errno.h>
#include <diegos/kernel.h>
#include <diegos/poll.h>
#include <diegos/interrupts.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libs/cbuffers.h>

#include <platform/N9H26.h>
#include <platform/interrupts.h>
#include "N9H26_UART.h"

#define TX_SEQ_MAX  (16UL)
#define RX_SEQ_MAX  (14UL)
#define BUF_SIZE    (1024)

static const unsigned speed_bps[] = {
	1200,
	2400,
	4800,
	9600,
	19200,
	38400,
	57600,
	115200
};

/*
 * 0 is high speed, 1 is normal speed.
 * Context for UART units.
 */

struct UART_ctx {
	/*
	* circular buffers for asynch operations.
	*/
	char *tx_buf, *rx_buf;
	/*
	* circular buffer handlers.
	*/
	struct cbuffer tx_cbuf, rx_cbuf;
	/*
	* FIFO status
	*/
	BOOL fifo_ok;
	/*
	* Line errors
	*/
	unsigned overrun_errors, parity_errors, framing_errors;
	/*
	 * Driver instance flags
	 */
	volatile unsigned flags;
	/*
	 * Wait queues
	 */
	wait_queue_t wq_w, wq_r;
};

static struct UART_ctx context[2];

static inline void enable_uart_tx(unsigned unitno)
{
	unsigned base = (unitno == 0) ? N9H26UART_BASE0 : N9H26UART_BASE1;
	/* Enable hold register empty interrupt, this will start tx */
	N9H26u(base, UA_IER, THRE_IEN);
	context[unitno].flags |= DRV_WRITING;
}

static inline void disable_uart_tx(unsigned unitno)
{
	unsigned base = (unitno == 0) ? N9H26UART_BASE0 : N9H26UART_BASE1;
	/* Disable hold register interrupt, no more tx */
	N9H26c(base, UA_IER, THRE_IEN);
	context[unitno].flags &= ~DRV_WRITING;
}

static void uart_handle_recv_data_fifo(unsigned unitno)
{
	struct UART_ctx *ctx = context + unitno;
	unsigned base = (unitno == 0) ? N9H26UART_BASE0 : N9H26UART_BASE1;
	unsigned rx_count;
	unsigned buffer_left = cbuffer_free_space(&ctx->rx_cbuf);
	uint32_t fifo_count = (N9H26r(base, UA_FSR) & Rx_Pointer) >> 8;

	ctx->flags |= DRV_READING;

	/*
	 * No space left, make FIFO RX buffer empty and drop data
	 */
	if (!buffer_left) {
		
		while (fifo_count--) {
			(void)N9H26r(base, UA_RBR);
		}

	} else {

		if (fifo_count < buffer_left) {
			rx_count = fifo_count;
		} else {
			rx_count = buffer_left;
		}

		while (rx_count--) {
			ctx->rx_buf[ctx->rx_cbuf.tail] = (char)(N9H26r(base, UA_RBR) & 0xFF);
			cbuffer_add(&ctx->rx_cbuf);
		}
	
		buffer_left = cbuffer_free_space(&ctx->rx_cbuf);
		fifo_count = N9H26r(base, UA_FSR) & Rx_Empty;
		while (buffer_left-- && !fifo_count) {
			ctx->rx_buf[ctx->rx_cbuf.tail] = (char)(N9H26r(base, UA_RBR) & 0xFF);
			cbuffer_add(&ctx->rx_cbuf);
			fifo_count = N9H26r(base, UA_FSR) & Rx_Empty;
		}
	}

	(void)thread_io_resume(&ctx->wq_r);
}

static void uart_handle_recv_data_timeout(unsigned unitno)
{
	struct UART_ctx *ctx = context + unitno;
	unsigned base = (unitno == 0) ? N9H26UART_BASE0 : N9H26UART_BASE1;
	uint32_t retval = N9H26r(base, UA_FSR) & Rx_Empty;
	unsigned buffer_left = cbuffer_free_space(&ctx->rx_cbuf);

	while (buffer_left-- && !retval) {
		ctx->rx_buf[ctx->rx_cbuf.tail] = (char)(N9H26r(base, UA_RBR) & 0xFF);
		cbuffer_add(&ctx->rx_cbuf);
		retval = N9H26r(base, UA_FSR) & Rx_Empty;
	}

	retval = N9H26r(base, UA_FSR) & Rx_Empty;
	if (retval) {
		ctx->flags &= ~DRV_READING;
	}

	(void)thread_io_resume(&ctx->wq_r);
}

static void uart_handle_tx_data(unsigned unitno)
{
	struct UART_ctx *ctx = context + unitno;
	unsigned base = (unitno == 0) ? N9H26UART_BASE0 : N9H26UART_BASE1;
	unsigned buffer = TX_SEQ_MAX;

	while (buffer-- && !cbuffer_is_empty(&ctx->tx_cbuf)) {
		N9H26w(base, UA_THR, (uint32_t)ctx->tx_buf[ctx->tx_cbuf.head]);
		cbuffer_remove(&ctx->tx_cbuf);
	}

	if (cbuffer_is_empty(&ctx->tx_cbuf)) {
		disable_uart_tx(unitno);
	}

	(void)thread_io_resume(&ctx->wq_w);
}

static BOOL hsuart_interrupt(void)
{
	return (TRUE);
}

static BOOL uart_interrupt(void)
{
	uint32_t interrupts = N9H26r(N9H26UART_BASE1, UA_ISR);

	if (interrupts & THRE_IF) {
		uart_handle_tx_data(1);
	}
	if (interrupts & RDA_IF) {
		uart_handle_recv_data_fifo(1);
	}
	if (interrupts & Tout_IF) {
		uart_handle_recv_data_timeout(1);
	}

	return (TRUE);
}

static void initialize_uart(unsigned unitno)
{
	if (unitno == 0) {
		//High Speed UART
		N9H26c(N9H26CLK_BASE, CLKDIV3, UART0_N1 | UART0_S | UART0_N0);
		N9H26cu(N9H26GCR_BASE, GPDFUN0, GPDFUN0, MF_GPD1 | MF_GPD2, 0x110);
		N9H26u(N9H26CLK_BASE, APBCLK, UART0_CKE);
	} else {
		//Normal Speed UART
		N9H26c(N9H26CLK_BASE, CLKDIV3, UART1_N1 | UART1_S | UART1_N0);
		N9H26cu(N9H26GCR_BASE, GPAFUN1, GPAFUN1, MF_GPA10 | MF_GPA11, 0x3300);
		N9H26u(N9H26CLK_BASE, APBCLK, UART1_CKE);
	}
}

static unsigned regunit = 0;

static int uart_init(unsigned unitno)
{
	/*
	 * Unit 0 is High Speed, unit 1 is normal speed
	 */
	if (unitno > 1) {
		return (ENXIO);
	}

	/*
	 * Check if already initialized
	 */
	if (regunit & (1 << unitno)) {
		return (EPERM);
	}
	regunit |= (1 << unitno);

	memset(context + unitno, 0, sizeof(context[unitno]));
	context[unitno].flags = DRV_IS_CHAR | DRV_READ_BLOCK | DRV_WRITE_BLOCK;
	context[unitno].tx_buf = calloc(1, BUF_SIZE);
	if (!context[unitno].tx_buf)
		return ENOMEM;

	context[unitno].rx_buf = calloc(1, BUF_SIZE);
	if (!context[unitno].rx_buf) {
		free(context[unitno].tx_buf);
		return ENOMEM;
	}

	initialize_uart(unitno);

	if (unitno == 0) {
		if (EOK != add_int_cb(hsuart_interrupt, HUART_INT)) {
			return (EGENERIC);
		}
	} else {
		if (EOK != add_int_cb(uart_interrupt, UART_INT)) {
			return (EGENERIC);
		}
	}

	cbuffer_init(&context[unitno].tx_cbuf, BUF_SIZE);
	cbuffer_init(&context[unitno].rx_cbuf, BUF_SIZE);

	return (EOK);
}

static int uart_start(unsigned unitno)
{
	unsigned base;
	struct UART_ctx *ctx;

	if (unitno > 1) {
		return (ENXIO);
	}

	ctx = context + unitno;

	if (ctx->flags & DRV_STATUS_RUN) {
		return (EOK);
	}

	if ((EOK != thread_io_wait_init(&ctx->wq_w)) || (EOK != thread_io_wait_init(&ctx->wq_r))) {
		return (EPERM);
	}

	ctx->flags &= ~DRV_STATUS_STOP;
	ctx->flags |= DRV_STATUS_RUN;

	base = (unitno == 0) ? N9H26UART_BASE0 : N9H26UART_BASE1;

	/*
	 * Enable 8N1 serial communications.
	 * 8 bit data, no parity bit, 1 stop bit.
	 */
	N9H26w(base, UA_LCR, 3);

	/*
	 * Setup rx timeout, 40 bits i.e. 4 octets when 8N1 is configured
	 */
	N9H26w(base, UA_TOR, 40);

	/*
	 * Enable FIFO in rx, make use of the maximum FIFO size,
	 * enable interrupt threshold to the maximum extent,
	 * and reset TX and RX.
	 */
	if (unitno == 0) {
		N9H26w(base, UA_FCR, (6 << 16) | (6 << 4) | TX_RST | RX_RST | 1);
		enable_int(HUART_INT);
	} else {
		N9H26w(base, UA_FCR, (3 << 16) | (3 << 4) | TX_RST | RX_RST | 1);
		enable_int(UART_INT);
	}

	/*
	 * Enable RX and TIMEOUT interrupts in UART device
	 */
	N9H26u(base, UA_IER, Time_out_EN | RTO_IEN | RDA_IEN);

	return (EOK);
}

static int uart_stop(unsigned unitno)
{
	struct UART_ctx *ctx;
	unsigned base = (unitno == 0) ? N9H26UART_BASE0 : N9H26UART_BASE1;

	if (unitno > 1) {
		return (ENXIO);
	}

	ctx = context + unitno;

	if (ctx->flags & DRV_STATUS_STOP) {
		return (EOK);
	}

	ctx->flags &= ~DRV_STATUS_RUN;
	ctx->flags |= DRV_STATUS_STOP;

	disable_int((unitno == 0) ? HUART_INT : UART_INT);
	
	/*
	 * Disable RX and TIMEOUT interrupts in UART device
	 */
	N9H26c(base, UA_IER, RTO_IEN | RDA_IEN);


	return (EOK);
}

static int uart_done(unsigned unitno)
{
	struct UART_ctx *ctx;

	if (unitno > 1) {
		return (ENXIO);
	}

	ctx = context + unitno;

	free(ctx->tx_buf);
	free(ctx->rx_buf);
	ctx->tx_buf = ctx->rx_buf = NULL;
	ctx->flags = DRV_STATUS_DONE;
	return (EOK);
}

static int uart_write(const void *buf, unsigned bytes, unsigned unitno)
{
	unsigned copy_bytes;
	unsigned buffer_space;
	int retval = 0;
	struct UART_ctx *ctx;

	if (unitno > 1) {
		return (ENXIO);
	}

	if (!buf) {
		return (EINVAL);
	}

	if (!bytes) {
		return (EOK);
	}

	ctx = context + unitno;

	/*
	 * Wait for some room in the buffer, avoid copying
	 * few bytes at a time.
	 */
	while (cbuffer_free_space(&ctx->tx_cbuf) < TX_SEQ_MAX) {
		(void)thread_io_wait(&ctx->wq_w);
	}

	buffer_space = cbuffer_free_space(&ctx->tx_cbuf);

	/*
	 * Trim the amount of bytes to be copied into the buffer.
	 * If the buffer cannot accomodate a single run length, either
	 * for the size or because of the buffer roll-over, the while
	 * loop will continue to the end.
	 */
	if (bytes > buffer_space) {
		copy_bytes = buffer_space;
	} else {
		copy_bytes = bytes;
	}

	if (ctx->tx_cbuf.tail + copy_bytes >= BUF_SIZE) {
		copy_bytes = BUF_SIZE - ctx->tx_cbuf.tail;
	}

	memcpy(ctx->tx_buf + ctx->tx_cbuf.tail, buf, copy_bytes);
	cbuffer_add_n(&ctx->tx_cbuf, copy_bytes);
	retval += copy_bytes;
	buf += copy_bytes;
	bytes -= copy_bytes;

	enable_uart_tx(unitno);

	while (bytes) {

		while (cbuffer_free_space(&ctx->tx_cbuf) < TX_SEQ_MAX) {
			(void)thread_io_wait(&ctx->wq_w);
		}

		buffer_space = cbuffer_free_space(&ctx->tx_cbuf);

		if (bytes > buffer_space) {
			copy_bytes = buffer_space;
		} else {
			copy_bytes = bytes;
		}

		if (ctx->tx_cbuf.tail + copy_bytes >= BUF_SIZE) {
			copy_bytes = BUF_SIZE - ctx->tx_cbuf.tail;
		}

		memcpy(ctx->tx_buf + ctx->tx_cbuf.tail, buf, copy_bytes);
		cbuffer_add_n(&ctx->tx_cbuf, copy_bytes);
		retval += copy_bytes;
		buf += copy_bytes;
		bytes -= copy_bytes;

		enable_uart_tx(unitno);
	}

	return (retval);
}

static int uart_read(void *buf, unsigned bytes, unsigned unitno)
{
	unsigned copy_bytes;
	unsigned ret_copy_bytes = 0;
	struct UART_ctx *ctx;

	if (!buf) {
		return (EINVAL);
	}

	if (!bytes) {
		return (EOK);
	}

	ctx = context + unitno;

	while (cbuffer_is_empty(&ctx->rx_cbuf)) {
		(void)thread_io_wait(&ctx->wq_r);
	}

	copy_bytes = cbuffer_in_use(&ctx->rx_cbuf);
	if (bytes >= copy_bytes) {
		ret_copy_bytes = copy_bytes;
	} else {
		ret_copy_bytes = bytes;
	}

	copy_bytes = ret_copy_bytes;
	if (ctx->rx_cbuf.head + copy_bytes >= BUF_SIZE) {
		memcpy(buf, ctx->rx_buf + ctx->rx_cbuf.head, BUF_SIZE - ctx->rx_cbuf.head);
		copy_bytes -= BUF_SIZE - ctx->rx_cbuf.head;
		buf += BUF_SIZE - ctx->rx_cbuf.head;
		cbuffer_remove_n(&ctx->rx_cbuf, BUF_SIZE - ctx->rx_cbuf.head);
	}
	memcpy(buf, ctx->rx_buf + ctx->rx_cbuf.head, copy_bytes);
	cbuffer_remove_n(&ctx->rx_cbuf, copy_bytes);

	return (ret_copy_bytes);
}

static int uart_ioctrl(void *data, unsigned opcode, unsigned unitno)
{
	unsigned temp, base;
	unsigned *decode = (unsigned *)data;
	unsigned i;

	if (unitno > 1) {
		return (ENXIO);
	}

	base = (unitno == 0) ? N9H26UART_BASE0 : N9H26UART_BASE1;

	switch (opcode) {
	case UART_SET_SPEED:
		if (*decode < speed_bps[0]) {
			return (EINVAL);
		}

		if (*decode >= speed_bps[NELEMENTS(speed_bps) - 1]) {
			i = NELEMENTS(speed_bps);
		} else {
			for (i = 1; i < NELEMENTS(speed_bps); i++) {
				if (*decode < speed_bps[i]) {
					break;
				}
			}
		}

		/*
		 * The true value is the lower speed, so that we can
		 * avoid forcing high speed on unsupporting hw
		 */
		--i;

		temp = (EXT_XTAL_CLK / speed_bps[i]) - 2;

		N9H26cu(base, UA_BAUD, UA_BAUD, BAUD_RATE_DIVISOR, DIV_X_EN | DIV_X_ONE | temp);

		return (EOK);

	case UART_SET_FLOW_CTRL:
		return (EOK);

	case UART_SET_BITS:
//              write_register(COM1, LCR, (uint8_t) (*decode));
		return (EOK);
	}

	return (EINVAL);
}

static unsigned uart_status(unsigned unitno)
{
	if (unitno < 2) {
		return (context[unitno].flags);
	}

	return (0);
}

static short uart_poll(unsigned unitno, poll_table_t * table)
{
	short ret = 0;
	struct UART_ctx *ctx;

	if (unitno < 2) {
		ctx = context + unitno;
		if (!cbuffer_is_empty(&ctx->rx_cbuf)) {
			ret |= (POLLIN | POLLRDNORM);
		}

		if (!cbuffer_is_empty(&ctx->tx_cbuf)) {
			ret |= (POLLOUT | POLLWRNORM);
		}

		poll_wait(&ctx->wq_r, table);
		poll_wait(&ctx->wq_w, table);
	}

	return (ret);
}

/*
 * This has been placed at the end so we do not need to
 * forward static declarations etc. etc.
 * Moreover, it is an initialization, no more than this...
 */
char_driver_t n9h26_uart_drv = {
	.cmn = {
		.name = "uart",
		.init_fn = uart_init,
		.start_fn = uart_start,
		.stop_fn = uart_stop,
		.done_fn = uart_done,
		.ioctrl_fn = uart_ioctrl,
		.status_fn = uart_status,
		.poll_fn = uart_poll}
	,
	.write_fn = uart_write,
	.read_fn = uart_read,
	.write_multi_fn = NULL,
	.read_multi_fn = NULL
};

#if 0
PFN_SYS_UART_CALLBACK(pfnUartIntHandlerTable)[2][2] = { 0 };

/* Global variables */
BOOL volatile _sys_bIsUARTInitial = FALSE;
BOOL volatile _sys_bIsUseUARTInt = TRUE;
uint32_t _sys_uUARTClockRate = EXTERNAL_CRYSTAL_CLOCK;
//uint32_t UART_BA = UART0_BA;

#define RX_ARRAY_NUM 100
char uart_rx[RX_ARRAY_NUM] = { 0 };

//void _PutChar_f(char ucCh);
uint32_t volatile rx_cnt = 0;

#define sysTxBufReadNextOne()   (((_sys_uUartTxHead+1)==UART_BUFFSIZE)? (uint32_t)NULL: _sys_uUartTxHead+1)
#define sysTxBufWriteNextOne()  (((_sys_uUartTxTail+1)==UART_BUFFSIZE)? (uint32_t)NULL: _sys_uUartTxTail+1)
#define UART_BUFFSIZE   256
char _sys_ucUartTxBuf[UART_BUFFSIZE];
uint32_t volatile _sys_uUartTxHead, _sys_uUartTxTail;
void *_sys_pvOldUartVect;
uint32_t u32UartPort = 0x100;

#ifdef __FreeRTOS__
SemaphoreHandle_t _uart_mutex;
int _uart_refcnt = 0;
#endif

int32_t i32UsedPort = 0;
void sysUartPort(uint32_t u32Port)
{
	u32UartPort = (u32Port & 0x1) * 0x100;
	if (u32Port == 0) {
		//High Speed UART
		outp32(REG_CLKDIV3, (inp32(REG_CLKDIV3) & (~(UART0_N1 | UART0_S | UART0_N0))));
		outp32(REG_GPDFUN0, (inp32(REG_GPDFUN0) & ~(MF_GPD1 | MF_GPD2)) | 0x110);
		outp32(REG_APBCLK, inp32(REG_APBCLK) | UART0_CKE);
	} else if (u32Port == 1) {
		//Nornal Speed UART
		outp32(REG_CLKDIV3, (inp32(REG_CLKDIV3) & (~(UART1_N1 | UART1_S | UART1_N0))));
		outp32(REG_GPAFUN1, (inp32(REG_GPAFUN1) & ~(MF_GPA10 | MF_GPA11)) | 0x3300);
		outp32(REG_APBCLK, inp32(REG_APBCLK) | UART1_CKE);

	}
	i32UsedPort = u32Port;
}

#if 1
void sysUartInstallcallback(uint32_t u32IntType, PFN_SYS_UART_CALLBACK pfnCallback)
{
	if (u32IntType > 1)
		return;
	if (u32IntType == 0) {
		pfnUartIntHandlerTable[i32UsedPort][0] = (PFN_SYS_UART_CALLBACK) (pfnCallback);
	} else if (u32IntType == 1) {
		pfnUartIntHandlerTable[i32UsedPort][1] = (PFN_SYS_UART_CALLBACK) (pfnCallback);
	}
}

void sysUartISR()
{

	uint32_t volatile regIIR, i;
	uint32_t volatile regIER, u32EnableInt;
	//sysprintf("UART ISR\n");
	regIIR = inpb(REG_UART_ISR + u32UartPort);
	regIER = inpb(REG_UART_IER + u32UartPort);
	u32EnableInt = regIER & regIIR;
	if (u32EnableInt & THRE_IF) {
		// buffer empty
		if (_sys_uUartTxHead == _sys_uUartTxTail) {
			//Disable interrupt if no any request!
			outpb((REG_UART_IER + u32UartPort),
			      inp32(REG_UART_IER + u32UartPort) & (~THRE_IEN));
		} else {
			//Transmit data
			for (i = 0; i < 8; i++) {
#ifdef __HW_SIM__

#else
				outpb(REG_UART_THR + u32UartPort,
				      _sys_ucUartTxBuf[_sys_uUartTxHead]);
#endif
				_sys_uUartTxHead = sysTxBufReadNextOne();
				if (_sys_uUartTxHead == _sys_uUartTxTail)	// buffer empty
					break;
			}
		}
	}
	if (u32EnableInt & RDA_IF) {
		uint32_t u32Count;
		u32Count = (inpw(REG_UART_FSR + u32UartPort) & Rx_Pointer) >> 8;
#if 0
		sysprintf("\nRx data available %d bytes\n", u32Count);
#endif
		for (i = 0; i < u32Count; i++) {
			if (rx_cnt == RX_ARRAY_NUM) {
				rx_cnt = 0;
			}
			uart_rx[rx_cnt] = (inpb(REG_UART_RBR + u32UartPort));
#if 0
			//debug _PutChar_f(uart_rx[rx_cnt + i]);
#endif
			rx_cnt++;
		}
#if 0
		sysprintf("\nRx array index %d\n", rx_cnt);
#else
		//callback to uplayer(bufptr, len);

		if (pfnUartIntHandlerTable[i32UsedPort][0] != 0)
			pfnUartIntHandlerTable[i32UsedPort][0] (&(uart_rx[0]), u32Count);
		rx_cnt = 0;
#endif

	}
	if (u32EnableInt & Tout_IF)	//½ÓÊÕ×´Ì¬·ÖÎö
	{
		uint32_t u32Count;
		u32Count = (inpw(REG_UART_FSR + u32UartPort) & Rx_Pointer) >> 8;
#if 0
		sysprintf("\nRx data time out %d bytes\n", u32Count);
#endif
		for (i = 0; i < u32Count; i++) {
			if (rx_cnt == RX_ARRAY_NUM) {
				rx_cnt = 0;
			}
			uart_rx[rx_cnt] = (inpb(REG_UART_RBR + u32UartPort));
#if 0
			_PutChar_f(uart_rx[rx_cnt + i]);
#endif
			rx_cnt++;

		}
#if 0
		sysprintf("\nRx array index %d\n", rx_cnt);
#else
		//callback to uplayer(bufptr, len);
		if (pfnUartIntHandlerTable[i32UsedPort][1] != 0)
			pfnUartIntHandlerTable[i32UsedPort][1] (&(uart_rx[0]), u32Count);
		rx_cnt = 0;
#endif
	}
	//sysprintf("\n");
}

#else
void sysUartISR()
{
	uint32_t volatile regIIR, i;
	regIIR = inpb(REG_UART_ISR + u32UartPort * 0x100);

	if (regIIR & THRE_IF) {
		// buffer empty
		if (_sys_uUartTxHead == _sys_uUartTxTail) {
			//Disable interrupt if no any request!
			outpb((REG_UART_IER + u32UartPort),
			      inp32(REG_UART_IER + u32UartPort) & (~THRE_IEN));
		} else {
			//Transmit data
			for (i = 0; i < 8; i++) {
#ifdef __HW_SIM__

#else
				outpb(REG_UART_THR + u32UartPort,
				      _sys_ucUartTxBuf[_sys_uUartTxHead]);
#endif
				_sys_uUartTxHead = sysTxBufReadNextOne();
				if (_sys_uUartTxHead == _sys_uUartTxTail)	// buffer empty
					break;
			}
		}
	}
}
#endif
static void sysSetBaudRate(uint32_t uBaudRate)
{
	uint32_t _mBaudValue;

	/* First, compute the baudrate divisor. */
#if 0
	// mode 0
	_mBaudValue = (_sys_uUARTClockRate / (uBaudRate * 16));
	if ((_sys_uUARTClockRate % (uBaudRate * 16)) > ((uBaudRate * 16) / 2))
		_mBaudValue++;
	_mBaudValue -= 2;
	outpw(REG_UART_BAUD + u32UartPort, _mBaudValue);
#else
	// mode 3
	_mBaudValue = (_sys_uUARTClockRate / uBaudRate) - 2;
	outpw(REG_UART_BAUD + u32UartPort, ((0x30 << 24) | _mBaudValue));
#endif
}

void sysUartEnableInt(int32_t eIntType)
{
	if (eIntType == UART_INT_RDA)
		outp32(REG_UART_IER + u32UartPort, inp32(REG_UART_IER + u32UartPort) | RDA_IEN);
	else if (eIntType == UART_INT_RDTO)
		outp32(REG_UART_IER + u32UartPort,
		       inp32(REG_UART_IER + u32UartPort) | RTO_IEN | Time_out_EN);
	else if (eIntType == UART_INT_NONE)
		outp32(REG_UART_IER + u32UartPort, 0x0);
}

int32_t sysInitializeUART(WB_UART_T * uart)
{
	/* Enable UART multi-function pins */
	//outpw(REG_PINFUN, inpw(REG_PINFUN) | 0x80);
	//outpw(REG_GPAFUN, inpw(REG_GPAFUN) | 0x00F00000); //Normal UART pin function
	static BOOL bIsResetFIFO = FALSE;
	int32_t i32Ret = 0;

	UART_MUTEX_INIT();

	//UART_MUTEX_LOCK();
	if (uart->uart_no == 0)
		sysUartPort(uart->uart_no);
	else
		sysUartPort(uart->uart_no);

	/* Check the supplied parity */
	if ((uart->uiParity != WB_PARITY_NONE) &&
	    (uart->uiParity != WB_PARITY_EVEN) && (uart->uiParity != WB_PARITY_ODD)) {
		i32Ret = (int32_t) WB_INVALID_PARITY;
		goto exit_sysInitializeUART;

		/* The supplied parity is not valid */
		//return (int32_t)WB_INVALID_PARITY;
	}
	/* Check the supplied number of data bits */
	else if ((uart->uiDataBits != WB_DATA_BITS_5) &&
		 (uart->uiDataBits != WB_DATA_BITS_6) &&
		 (uart->uiDataBits != WB_DATA_BITS_7) && (uart->uiDataBits != WB_DATA_BITS_8)) {
		i32Ret = (int32_t) WB_INVALID_DATA_BITS;
		goto exit_sysInitializeUART;
		/* The supplied data bits value is not valid */
		//return (int32_t)WB_INVALID_DATA_BITS;
	}
	/* Check the supplied number of stop bits */
	else if ((uart->uiStopBits != WB_STOP_BITS_1) && (uart->uiStopBits != WB_STOP_BITS_2)) {
		i32Ret = (int32_t) WB_INVALID_STOP_BITS;
		goto exit_sysInitializeUART;
		/* The supplied stop bits value is not valid */
		//return (int32_t)WB_INVALID_STOP_BITS;
	}

	/* Verify the baud rate is within acceptable range */
	else if (uart->uiBaudrate < 1200) {
		i32Ret = (int32_t) WB_INVALID_BAUD;
		goto exit_sysInitializeUART;
		/* The baud rate is out of range */
		//return (int32_t)WB_INVALID_BAUD;
	}
	/* Reset the TX/RX FIFOs */
	if (bIsResetFIFO == FALSE) {
		outpw(REG_UART_FCR + u32UartPort, 0x07);
		bIsResetFIFO = TRUE;
	}
	/* Setup reference clock */
	_sys_uUARTClockRate = uart->uiFreq;

	/* Setup baud rate */
	sysSetBaudRate(uart->uiBaudrate);

	/* Set the modem control register. Set DTR, RTS to output to LOW,
	   and set int output pin to normal operating mode */
	//outpb(UART_MCR, (WB_DTR_Low | WB_RTS_Low | WB_MODEM_En));

	/* Setup parity, data bits, and stop bits */
	outpw(REG_UART_LCR + u32UartPort, (uart->uiParity | uart->uiDataBits | uart->uiStopBits));

	/* Timeout if more than ??? bits xfer time */
	outpw(REG_UART_TOR + u32UartPort, 0x80 + 0x20);

	/* Setup Fifo trigger level and enable FIFO */
	outpw(REG_UART_FCR + u32UartPort, (uart->uiRxTriggerLevel << 4) | 0x01);

	/* Enable HUART interrupt Only (Receive Data Available Interrupt & RX Time out Interrupt) */
#if 0
#if 1				//Enable RX data time out
	outp32(REG_UART_IER + u32UartPort,
	       inp32(REG_UART_IER + u32UartPort) | RDA_IEN | RTO_IEN | Time_out_EN);
#else
	outp32(REG_UART_IER + u32UartPort, inp32(REG_UART_IER + u32UartPort) | RDA_IEN);
#endif
#endif

	/* Timeout if more than ??? bits xfer time */
	outpw(REG_UART_TOR + u32UartPort, 0x7F);

	// hook UART interrupt service routine
	if (u32UartPort) {
		//==1 NORMAL UART
		_sys_uUartTxHead = _sys_uUartTxTail = (uint32_t) NULL;
		_sys_pvOldUartVect = sysInstallISR(IRQ_LEVEL_1, IRQ_UART, (void *)sysUartISR);
		sysEnableInterrupt(IRQ_UART);
	} else {
		//==0 High SPEED
		_sys_uUartTxHead = _sys_uUartTxTail = (uint32_t) NULL;
		_sys_pvOldUartVect = sysInstallISR(IRQ_LEVEL_1, IRQ_HUART, (void *)sysUartISR);
		sysEnableInterrupt(IRQ_HUART);
	}
	_sys_bIsUARTInitial = TRUE;

	i32Ret = Successful;

 exit_sysInitializeUART:
	//UART_MUTEX_UNLOCK();

	return i32Ret;
}

#if 0
void _PutChar_f(char ucCh)
{
	if (_sys_bIsUseUARTInt == TRUE) {
		while (sysTxBufWriteNextOne() == _sys_uUartTxHead) ;	// buffer full

		_sys_ucUartTxBuf[_sys_uUartTxTail] = ucCh;
		_sys_uUartTxTail = sysTxBufWriteNextOne();

		if (ucCh == '\n') {
			while (sysTxBufWriteNextOne() == _sys_uUartTxHead) ;	// buffer full

			_sys_ucUartTxBuf[_sys_uUartTxTail] = '\r';
			_sys_uUartTxTail = sysTxBufWriteNextOne();
		}

		if (!(inpw(REG_UART_IER + u32UartPort) & 0x02))
			outpw(REG_UART_IER + u32UartPort, 0x02);
	} else {
		/* Wait until the transmitter buffer is empty */
		while (!(inpw(REG_UART_FSR + u32UartPort) & 0x400000)) ;
		/* Transmit the character */
		outpb(REG_UART_THR + u32UartPort, ucCh);

		if (ucCh == '\n') {
			/* Wait until the transmitter buffer is empty */
			while (!(inpw(REG_UART_FSR + u32UartPort) & 0x400000)) ;
			outpb(REG_UART_THR + u32UartPort, '\r');
		}
	}
}

void sysPutString(char *string)
{
	while (*string != '\0') {
		_PutChar_f(*string);
		string++;
	}
}

static void sysPutRepChar(char c, int count)
{
	while (count--)
		_PutChar_f(c);
}

static void sysPutStringReverse(char *s, int index)
{
	while ((index--) > 0)
		_PutChar_f(s[index]);
}

static void sysPutNumber(int value, int radix, int width, char fill)
{
	char buffer[40];
	int bi = 0;
	uint32_t uvalue;
	uint16_t digit;
	uint16_t left = FALSE;
	uint16_t negative = FALSE;

	if (fill == 0)
		fill = ' ';

	if (width < 0) {
		width = -width;
		left = TRUE;
	}

	if (width < 0 || width > 80)
		width = 0;

	if (radix < 0) {
		radix = -radix;
		if (value < 0) {
			negative = TRUE;
			value = -value;
		}
	}

	uvalue = value;

	do {
		if (radix != 16) {
			digit = uvalue % radix;
			uvalue = uvalue / radix;
		} else {
			digit = uvalue & 0xf;
			uvalue = uvalue >> 4;
		}
		buffer[bi] = digit + ((digit <= 9) ? '0' : ('A' - 10));
		bi++;

		if (uvalue != 0) {
			if ((radix == 10)
			    && ((bi == 3) || (bi == 7) || (bi == 11) | (bi == 15))) {
				buffer[bi++] = ',';
			}
		}
	}
	while (uvalue != 0);

	if (negative) {
		buffer[bi] = '-';
		bi += 1;
	}

	if (width <= bi)
		sysPutStringReverse(buffer, bi);
	else {
		width -= bi;
		if (!left)
			sysPutRepChar(fill, width);
		sysPutStringReverse(buffer, bi);
		if (left)
			sysPutRepChar(fill, width);
	}
}

static char *FormatItem(char *f, int a)
{
	char c;
	int fieldwidth = 0;
	int leftjust = FALSE;
	int radix = 0;
	char fill = ' ';

	if (*f == '0')
		fill = '0';

	while ((c = *f++) != 0) {
		if (c >= '0' && c <= '9') {
			fieldwidth = (fieldwidth * 10) + (c - '0');
		} else
			switch (c) {
			case '\000':
				return (--f);
			case '%':
				_PutChar_f('%');
				return (f);
			case '-':
				leftjust = TRUE;
				break;
			case 'c':
				{
					if (leftjust)
						_PutChar_f(a & 0x7f);

					if (fieldwidth > 0)
						sysPutRepChar(fill, fieldwidth - 1);

					if (!leftjust)
						_PutChar_f(a & 0x7f);
					return (f);
				}
			case 's':
				{
					if (leftjust)
						sysPutString((char *)a);

					if (fieldwidth > strlen((char *)a))
						sysPutRepChar(fill, fieldwidth - strlen((char *)a));

					if (!leftjust)
						sysPutString((char *)a);
					return (f);
				}
			case 'd':
			case 'i':
				radix = -10;
				break;
			case 'u':
				radix = 10;
				break;
			case 'x':
				radix = 16;
				break;
			case 'X':
				radix = 16;
				break;
			case 'o':
				radix = 8;
				break;
			default:
				radix = 3;
				break;	/* unknown switch! */
			}
		if (radix)
			break;
	}

	if (leftjust)
		fieldwidth = -fieldwidth;

	sysPutNumber(a, radix, fieldwidth, fill);

	return (f);
}

void sysPrintf(char *pcStr, ...)
{
	WB_UART_T uart;
	char *argP;

	UART_MUTEX_LOCK();

	_sys_bIsUseUARTInt = TRUE;
	if (!_sys_bIsUARTInitial) {
		sysUartPort(1);
		uart.uart_no = WB_UART_1;
		uart.uiFreq = sysGetExternalClock();
		uart.uiBaudrate = 115200;
		uart.uiDataBits = WB_DATA_BITS_8;
		uart.uiStopBits = WB_STOP_BITS_1;
		uart.uiParity = WB_PARITY_NONE;
		uart.uiRxTriggerLevel = LEVEL_1_BYTE;
		sysInitializeUART(&uart);
	}

	vaStart(argP, pcStr);	/* point at the end of the format string */
	while (*pcStr) {
		/* this works because args are all ints */
		if (*pcStr == '%')
			pcStr = FormatItem(pcStr + 1, vaArg(argP, int));
		else
			_PutChar_f(*pcStr++);
	}
	UART_MUTEX_UNLOCK();
}

void sysprintf(char *pcStr, ...)
{
	WB_UART_T uart;
	char *argP;

	UART_MUTEX_LOCK();
	_sys_bIsUseUARTInt = FALSE;
	if (!_sys_bIsUARTInitial) {
		//Default use external clock 12MHz as source clock.
		sysUartPort(1);
		uart.uart_no = WB_UART_1;
		uart.uiFreq = sysGetExternalClock();
		uart.uiBaudrate = 115200;
		uart.uiDataBits = WB_DATA_BITS_8;
		uart.uiStopBits = WB_STOP_BITS_1;
		uart.uiParity = WB_PARITY_NONE;
		uart.uiRxTriggerLevel = LEVEL_1_BYTE;
		sysInitializeUART(&uart);
	}

	vaStart(argP, pcStr);	/* point at the end of the format string */
	while (*pcStr) {
		/* this works because args are all ints */
		if (*pcStr == '%')
			pcStr = FormatItem(pcStr + 1, vaArg(argP, int));
		else
			_PutChar_f(*pcStr++);
	}
	UART_MUTEX_UNLOCK();
}

char sysGetChar()
{
	while (1) {
		if (inpw(REG_UART_ISR + u32UartPort) & 0x01)
			return (inpb(REG_UART_RBR + u32UartPort));
	}
}

void sysPutChar(char ucCh)
{
	/* Wait until the transmitter buffer is empty */
	while (!(inpw(REG_UART_FSR + u32UartPort) & 0x400000)) ;
	/* Transmit the character */
	outpb(REG_UART_THR + u32UartPort, ucCh);
}

void sysUartTransfer(char *pu8buf, uint32_t u32Len)
{
	do {
		if ((inp32(REG_UART_FSR + u32UartPort) & Tx_Full) == 0) {
			outpb(REG_UART_THR + u32UartPort, *pu8buf++);
			u32Len = u32Len - 1;
		}
	}
	while (u32Len != 0);
}
#endif
#endif
