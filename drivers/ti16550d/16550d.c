/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2025 Diego Gallizioli
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

#include <types_common.h>
#include <processor/ports.h>
#include <platform/i8259.h>
#include <errno.h>
#include <diegos/kernel.h>
#include <diegos/poll.h>
#include <diegos/interrupts.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libs/cbuffers.h>

#include "16550d_private.h"
#include "16550d.h"

#define TX_SEQ_MAX  (16UL)
#define RX_SEQ_MAX  (14UL)
#define BUF_SIZE    (1024)
#define XON         (0x11)
#define XOFF        (0x13)

/*
 * Arrays must be addressable by BPS_xxx enum values
 */
static const uint16_t divisor_latch[] = {
	2304,
	1047,
	768,
	384,
	192,
	96,
	48,
	24,
	12,
	6,
	3,
	2,
	1
};

/*
 * Speeds are divided by 10 to store a uint16_t
 * instead of an int.
 */
static const uint16_t speed_bps[] = {
	5,
	11,
	15,
	30,
	60,
	120,
	240,
	480,
	960,
	1920,
	3840,
	5760,
	11520
};

static BOOL fifo_ok = FALSE;

/*
 * buffers for asynch operations.
 */
static char *tx_buf;
static char *rx_buf;
static struct cbuffer tx_cbuf, rx_cbuf;

/*
 * Line errors
 */
static unsigned overrun_errors = 0, parity_errors = 0, framing_errors = 0;
static volatile unsigned flags = DRV_IS_CHAR | DRV_READ_BLOCK | DRV_WRITE_BLOCK;
static wait_queue_t wq_w, wq_r;

static inline void write_register(const uint16_t com, enum uart_registers reg, uint8_t data)
{
	out_byte(com + reg, data);
}

static inline uint8_t read_register(const uint16_t com, enum uart_registers reg)
{
	return (in_byte(com + reg));
}

static inline void enable_uart_tx(void)
{
	uint8_t retval;

	flags |= DRV_WRITING;
	retval = read_register(COM1, IER);
	/* Enable hold register empty interrupt, this will start tx */
	retval |= IER_ETBEI;
	write_register(COM1, IER, retval);
}

static inline void disable_uart_tx(void)
{
	uint8_t retval;

	retval = read_register(COM1, IER);
	/* Disable hold register interrupt, no more tx */
	retval &= ~IER_ETBEI;
	write_register(COM1, IER, retval);
	flags &= ~DRV_WRITING;
}

static void uart_handle_recv_line_status(void)
{
	/*
	 * Only Overhead, Parity and Framing Errors are true errors.
	 * I still need to understand what the BI Break Interrupt stands for and
	 * what to do with it.
	 */
	uint8_t retval = read_register(COM1, LSR);

	if (retval & LSR_OE) {
		++overrun_errors;
	}
	if (retval & LSR_PE) {
		++parity_errors;
	}
	if (retval & LSR_FE) {
		++framing_errors;
	}
}

static void uart_handle_recv_data_fifo(void)
{
	uint8_t retval;
	unsigned rx_count;
	unsigned buffer_left = cbuffer_free_space(&rx_cbuf);

	flags |= DRV_READING;

	/*
	 * Negative case: we are flooded with data and cannot
	 * service the FIFO RX buffer; we need to drop data.
	 * Well the correct action would be to stop the sender
	 * but I do not want to implement XON/XOFF or check for
	 * hardware flow control.
	 */
	if (!buffer_left) {

		rx_count = RX_SEQ_MAX;
		while (rx_count--) {
			(void)read_register(COM1, RBR);
		}

		while (read_register(COM1, LSR) & LSR_DR) {
			(void)read_register(COM1, RBR);
		}

	} else {

		/*
		 * Make the FIFO RX buffer empty with a strategy:
		 * first run will retrieve data present in the buffer,
		 * as RX_SEQ_MAX is the threshold to trigger this interrupt.
		 * Second run will poll for data available in the FIFO RX buffer,
		 * slowing down a bit the loop.
		 */
		if (RX_SEQ_MAX < buffer_left) {
			rx_count = RX_SEQ_MAX;
		} else {
			rx_count = buffer_left;
		}

		/*
		 * Read up to RX_SEQ_MAX bytes; if less space is left in rx_cbuf,
		 * then the second half of the algorithm will not be performed.
		 */
		while (rx_count--) {
			rx_buf[rx_cbuf.tail] = read_register(COM1, RBR);
			cbuffer_add(&rx_cbuf);
		}

		/*
		 * If we have space in rx_cbuf, empty the FIFO RX buffer.
		 */
		buffer_left = cbuffer_free_space(&rx_cbuf);
		if (buffer_left) {
			rx_count = buffer_left;
			retval = read_register(COM1, LSR);
			while (rx_count-- && (retval & LSR_DR)) {
				rx_buf[rx_cbuf.tail] = read_register(COM1, RBR);
				cbuffer_add(&rx_cbuf);
				retval = read_register(COM1, LSR);
			}
		}
	}

	(void)thread_io_resume(&wq_r);
}

static void uart_handle_recv_data_timeout(void)
{
	uint8_t retval = 0;
	unsigned buffer_left = cbuffer_free_space(&rx_cbuf);

	/*
	 * Empty the FIFO RX buffer.
	 */
	retval = read_register(COM1, LSR);
	while (buffer_left-- && (retval & LSR_DR)) {
		rx_buf[rx_cbuf.tail] = read_register(COM1, RBR);
		cbuffer_add(&rx_cbuf);
		retval = read_register(COM1, LSR);
	}

	retval = read_register(COM1, LSR);
	if (~retval & LSR_DR) {
		flags &= ~DRV_READING;
	}

	(void)thread_io_resume(&wq_r);
}

static void uart_handle_tx_data(void)
{
	unsigned buffer = TX_SEQ_MAX;

	while (buffer-- && !cbuffer_is_empty(&tx_cbuf)) {
		write_register(COM1, THR, tx_buf[tx_cbuf.head]);
		cbuffer_remove(&tx_cbuf);
	}

	if (cbuffer_is_empty(&tx_cbuf)) {
		disable_uart_tx();
	}

	(void)thread_io_resume(&wq_w);
}

static void uart_handle_modem_status(void)
{
	/*
	 * Don't know what to do with this, just ack the interrupt
	 */
	const uint8_t MSR_VAL = (MSR_DCTS | MSR_DDSR | MSR_TERI | MSR_DDCD);
	uint8_t retval = read_register(COM1, MSR);

	if (retval & MSR_VAL) {
	}
}

static BOOL uart_interrupt(void)
{
	uint8_t retval, intval;

	do {
		retval = read_register(COM1, IIR);
		intval = retval & IIR_INT_MASK;

		/*
		 * Interrupts services in descending priority order
		 * Highest to lowest
		 */
		switch (intval) {
		case IIR_RCVR_LINE_STAT:
			uart_handle_recv_line_status();
			break;
		case IIR_RCV_DATA_AVAIL:
			uart_handle_recv_data_fifo();
			break;
		case IIR_CHAR_TIMEOUT:
			uart_handle_recv_data_timeout();
			break;
		case IIR_TRSR_HOLD_EMPTY:
			uart_handle_tx_data();
			break;
		case IIR_MODEM_STATUS:
			uart_handle_modem_status();
			break;
		}

	} while (!(retval & IIR_NO_INT_PEND));

	return (TRUE);
}

static int uart_init(unsigned unitno)
{
	const uint8_t DETECT = (FCR_ENA_FIFOS |
				FCR_RCVR_FIFO_RST | FCR_XMIT_FIFO_RST | FCR_FIFO_14B);
	uint8_t retval;

	/*
	 * Only COM0 right now
	 */
	if (unitno) {
		return (ENXIO);
	}

	tx_buf = calloc(1, BUF_SIZE);
	if (!tx_buf)
		return ENOMEM;

	rx_buf = calloc(1, BUF_SIZE);
	if (!rx_buf) {
		free(tx_buf);
		return ENOMEM;
	}

	/* Start detecting the device */
	write_register(COM1, FCR, DETECT);
	retval = read_register(COM1, IIR);

	if (retval & IIR_FIFO_ENA) {
		fifo_ok = TRUE;
		flags |= (DRV_READ_BLOCK | DRV_WRITE_BLOCK);
	}

	if (EOK != add_int_cb(uart_interrupt, UART_IVT)) {
		return (EGENERIC);
	}

	cbuffer_init(&tx_cbuf, BUF_SIZE);
	cbuffer_init(&rx_cbuf, BUF_SIZE);

	return (EOK);
}

static int uart_start(unsigned unitno)
{
	/*
	 * Enable 8N1 serial communications.
	 * 8 bit data, no paritiy bit, 1 stop bit.
	 */
	const uint8_t LCR_VAL = (LCR_8_DATA_BITS | LCR_1_STOP_BIT);

	/*
	 * Enable FIFO in rx and tx, make use of the maximum FIFO size
	 */
	const uint8_t FCR_VAL = (FCR_ENA_FIFOS |
				 FCR_RCVR_FIFO_RST |
				 FCR_XMIT_FIFO_RST | FCR_DMA_MODE | FCR_FIFO_14B);

	/*
	 * Enable interrupts
	 */
	const uint8_t IER_VAL = (IER_ERBFI | IER_ELSI | IER_EDSSI);

	if (unitno) {
		return (EINVAL);
	}

	if (flags & DRV_STATUS_RUN) {
		return (EOK);
	}

	if ((EOK != thread_io_wait_init(&wq_w)) || (EOK != thread_io_wait_init(&wq_r))) {
		return (EPERM);
	}

	flags &= ~DRV_STATUS_STOP;
	flags |= DRV_STATUS_RUN;

	write_register(COM1, LCR, LCR_VAL);
	write_register(COM1, FCR, FCR_ENA_FIFOS);
	write_register(COM1, FCR, FCR_VAL);
	write_register(COM1, IER, IER_VAL);

	enable_int(UART_IVT);

	return (EOK);
}

static int uart_stop(unsigned unitno)
{
	/*
	 * Disable all interrupts
	 */
	const uint8_t IER_VAL = (IER_ERBFI | IER_ETBEI | IER_ELSI | IER_EDSSI);

	if (unitno) {
		return (EINVAL);
	}

	if (flags & DRV_STATUS_STOP) {
		return (EOK);
	}

	flags &= ~DRV_STATUS_RUN;
	flags |= DRV_STATUS_STOP;

	write_register(COM1, IER, IER_VAL);

	disable_int(UART_IVT);

	return (EOK);
}

static int uart_done(unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	free(tx_buf);
	free(rx_buf);
	flags = DRV_STATUS_DONE;

	return (del_int_cb(UART_IVT));
}

static int uart_write(const void *buf, unsigned bytes, unsigned unitno)
{
	unsigned copy_bytes;
	unsigned buffer_space;
	int retval = 0;

	if (!buf || unitno) {
		return (EINVAL);
	}

	if (!bytes) {
		return (EOK);
	}

	/*
	 * Wait for some room in the buffer, avoid copying
	 * few bytes at a time.
	 */
	while (cbuffer_free_space(&tx_cbuf) < TX_SEQ_MAX) {
		(void)thread_io_wait(&wq_w);
	}

	buffer_space = cbuffer_free_space(&tx_cbuf);

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

	if (tx_cbuf.tail + copy_bytes >= BUF_SIZE) {
		copy_bytes = BUF_SIZE - tx_cbuf.tail;
	}

	memcpy(tx_buf + tx_cbuf.tail, buf, copy_bytes);
	cbuffer_add_n(&tx_cbuf, copy_bytes);
	retval += copy_bytes;
	buf += copy_bytes;
	bytes -= copy_bytes;

	enable_uart_tx();

	while (bytes) {

		while (cbuffer_free_space(&tx_cbuf) < TX_SEQ_MAX) {
			(void)thread_io_wait(&wq_w);
		}

		buffer_space = cbuffer_free_space(&tx_cbuf);

		if (bytes > buffer_space) {
			copy_bytes = buffer_space;
		} else {
			copy_bytes = bytes;
		}

		if (tx_cbuf.tail + copy_bytes >= BUF_SIZE) {
			copy_bytes = BUF_SIZE - tx_cbuf.tail;
		}

		memcpy(tx_buf + tx_cbuf.tail, buf, copy_bytes);
		cbuffer_add_n(&tx_cbuf, copy_bytes);
		retval += copy_bytes;
		buf += copy_bytes;
		bytes -= copy_bytes;

		enable_uart_tx();
	}

	return (retval);
}

static int uart_read(void *buf, unsigned bytes, unsigned unitno)
{
	unsigned copy_bytes;
	unsigned ret_copy_bytes = 0;

	if (!buf || unitno) {
		return (EINVAL);
	}

	if (!bytes) {
		return (EOK);
	}

	while (cbuffer_is_empty(&rx_cbuf)) {
		(void)thread_io_wait(&wq_r);
	}

	copy_bytes = cbuffer_in_use(&rx_cbuf);
	if (bytes >= copy_bytes) {
		ret_copy_bytes = copy_bytes;
	} else {
		ret_copy_bytes = bytes;
	}

	copy_bytes = ret_copy_bytes;
	if (rx_cbuf.head + copy_bytes >= BUF_SIZE) {
		memcpy(buf, rx_buf + rx_cbuf.head, BUF_SIZE - rx_cbuf.head);
		copy_bytes -= BUF_SIZE - rx_cbuf.head;
		buf += BUF_SIZE - rx_cbuf.head;
		cbuffer_remove_n(&rx_cbuf, BUF_SIZE - rx_cbuf.head);
	}
	memcpy(buf, rx_buf + rx_cbuf.head, copy_bytes);
	cbuffer_remove_n(&rx_cbuf, copy_bytes);

	return (ret_copy_bytes);
}

static int uart_ioctrl(void *data, unsigned opcode, unsigned unitno)
{
	uint8_t temp;
	unsigned *decode = (unsigned *)data;
	unsigned i;

	if (!data) {
		return (EINVAL);
	}

	switch (opcode) {
	case UART_SET_SPEED:
		*decode /= 10;
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

		temp = read_register(COM1, LCR);
		temp |= LCR_DLAB;
		write_register(COM1, LCR, temp);
		/* now we can write to DLL and DLM registers */
		write_register(COM1, DLL, (uint8_t) divisor_latch[i]);
		write_register(COM1, DLH, (uint8_t) (divisor_latch[i] >> 8));
		temp &= ~LCR_DLAB;
		write_register(COM1, LCR, temp);
		return (EOK);

	case UART_SET_FLOW_CTRL:
		return (EOK);

	case UART_SET_BITS:
		write_register(COM1, LCR, (uint8_t) (*decode));
		return (EOK);
	}

	return (EINVAL);
}

static unsigned uart_status(unsigned unitno)
{
	if (!unitno) {
		return (flags);
	}

	return (0);
}

static short uart_poll(unsigned unitno, poll_table_t * table)
{
	short ret = 0;

	if (!unitno) {
		if (!cbuffer_is_empty(&rx_cbuf)) {
			ret |= (POLLIN | POLLRDNORM);
		}

		if (!cbuffer_is_empty(&tx_cbuf)) {
			ret |= (POLLOUT | POLLWRNORM);
		}

		poll_wait(&wq_r, table);
		poll_wait(&wq_w, table);
	}

	return (ret);
}

/*
 * This has been placed at the end so we do not need to
 * forward static declarations etc. etc.
 * Moreover, it is an initialization, no more than this...
 */
char_driver_t uart16550d_drv = {
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
