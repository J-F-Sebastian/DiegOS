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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <diegos/drivers.h>
#include <diegos/net_drivers.h>
#include <diegos/if.h>

#include "local_loop.h"

static uint32_t lo_buffer[(SHRT_MAX / 4) + 1];
static unsigned head = 0;
static unsigned tail = 0;
static wait_queue_t wq_r;

static int lo_init(unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	return (EOK);
}

static int lo_start(unsigned unitno)
{
	if (EOK != thread_io_wait_init(&wq_r)) {
		return (EPERM);
	}

	kdrvprintf("Local Loop started\n");
	return (EOK);
}

static int lo_stop(unsigned unitno)
{
	kdrvprintf("Local Loop cannot stop !!!\n");
	return (EPERM);
}

static int lo_done(unsigned unitno)
{
	return (EOK);
}

static unsigned lo_status(unsigned unitno)
{
	return (DRV_STATUS_RUN | DRV_IS_NET);
}

static int lo_tx(const struct packet *buf, unsigned unitno)
{
	if (!buf || (sizeof(lo_buffer) < (tail + (buf->data_payload_size + 8) / 4))) {
		return (ENOBUFS);
	}

	if (lo_drv.mtu < buf->data_payload_size) {
		return (EPACKSIZE);
	}

	lo_buffer[tail] = buf->data_payload_size;
	memcpy(lo_buffer + tail + 1, buf->data_payload_cursor, buf->data_payload_size);
	/*
	 * Tail is rounded to a multiple of 4 so the next packet is aligned to
	 * a 32 bit boundary
	 */
	tail += (buf->data_payload_size + 8) / 4;

	thread_io_resume(&wq_r);

	return (EOK);
}

static int lo_rx(struct packet *buf, unsigned unitno)
{
	if (!buf) {
		return (EIO);
	}

	if (buf->data_size < lo_buffer[head]) {
		return (EPACKSIZE);
	}

	while (tail == head) {
		(void)thread_io_wait(&wq_r);
	}

	memcpy(buf->data_payload_start, lo_buffer + head + 1, lo_buffer[head]);
	buf->data_payload_size = lo_buffer[head];
	buf->data_payload_cursor = buf->data_payload_start;
	head += (buf->data_payload_size + 8) / 4;

	if (head == tail) {
		head = tail = 0;
	}

	return (EOK);
}

static int lo_peak(unsigned *bsize, unsigned items, unsigned unitno)
{
	unsigned lochead;

	if (!bsize || !items) {
		return (EINVAL);
	}

	lochead = head;
	while (items-- && (lochead != tail)) {
		*bsize++ = lo_buffer[lochead];
		lochead += (lo_buffer[lochead] + 8) / 4;
	}

	return (EOK);
}

static short lo_poll(unsigned unitno, poll_table_t *table)
{
	short ret = 0;

	if (tail != head) {
		ret |= (POLLIN | POLLRDNORM);
	}

	poll_wait(&wq_r, table);

	return (ret);
}

net_driver_t lo_drv = {
	.cmn = {
		.name = "lo",
		.init_fn = lo_init,
		.start_fn = lo_start,
		.stop_fn = lo_stop,
		.done_fn = lo_done,
		.status_fn = lo_status,
		.poll_fn = lo_poll}
	,
	.ifname = "lo",
	.mtu = SHRT_MAX,
	.iftype = 0,
	.addr = {0x7F, 0x0, 0x0, 0x1}
	,
	.ifflags = (IFF_UP | IFF_LOOPBACK),
	.tx_fn = lo_tx,
	.rx_fn = lo_rx,
	.tx_multi_fn = NULL,
	.rx_multi_fn = NULL,
	.rx_peak_fn = lo_peak
};
