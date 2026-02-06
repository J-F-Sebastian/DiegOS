/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2026 Diego Gallizioli
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

#include <diegos/net_buffers.h>
#include <diegos/interrupts.h>
#include <diegos/barriers.h>
#include <libs/cbuffers.h>
#include <libs/pakman.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

static barrier_t *netb = NULL;
static pakman *packet_manager = NULL;
static struct cbuffer in_cb, out_cb;
static struct packet **in_queue;
static struct my_out
{
	struct packet *pkt;
	net_interface_t *itf;
} *out_queue;

int netbuf_init(unsigned bytes, unsigned packets)
{
	if ((bytes < CACHE_ALN) || (packets < 8))
		return EINVAL;

	in_queue = calloc(sizeof(void *), packets);
	out_queue = calloc(sizeof(struct my_out), packets);

	if ((in_queue && out_queue) == 0) {
		if (in_queue)
			free(in_queue);
		if (out_queue)
			free(out_queue);
		return ENOMEM;
	}

	cbuffer_init(&in_cb, packets);
	cbuffer_init(&out_cb, packets);

	if (EOK!= init_pakman(bytes, packets, &packet_manager))	{
		free(in_queue);
		free(out_queue);
		return ENOMEM;
	}

	netb = barrier_create("network", FALSE);

	if (!netb) {
		free(in_queue);
		free(out_queue);
		delete_pakman(packet_manager);
		return ENOMEM;
	}

	return (EOK);
}

int netbuf_get(struct packet **pkt, unsigned bytes)
{
	lock();
	*pkt = pakman_get_packet(packet_manager, bytes);
	unlock();

	return (*pkt) ? (EOK) : (ENOBUFS);
}

int netbuf_put(struct packet *pkt)
{
	int retval;

	if (!pkt)
		return (EINVAL);

	lock();
	retval = pakman_put_packet(packet_manager, pkt);
	unlock();

	return (retval);
}

int netbuf_in(struct packet *pkt)
{
	if (!pkt)
		return (EINVAL);

	/*
	 * In and out queues are contention free by design,
	 * no need to lock interrupts
	 */
	if (cbuffer_is_full(&in_cb))
		return ENOBUFS;

	in_queue[in_cb.tail] = pkt;

	cbuffer_add(&in_cb);

	barrier_open(netb);

	return (EOK);
}

int netbuf_out(struct packet *pkt, net_interface_t *intf)
{
	if (!pkt || !intf)
		return (EINVAL);
	/*
	 * In and out queues are contention free by design,
	 * no need to lock interrupts
	 */
	if (cbuffer_is_full(&out_cb))
		return ENOBUFS;

	out_queue[out_cb.tail].pkt = pkt;
	out_queue[out_cb.tail].itf = intf;

	cbuffer_add(&out_cb);

	barrier_open(netb);

	return (EOK);
}

int netbuf_process_in(struct packet **pkt)
{
	if (!pkt)
		return EINVAL;

	if (cbuffer_is_empty(&in_cb)) {
		return EAGAIN;
	}

	*pkt = in_queue[in_cb.head];

	cbuffer_remove(&in_cb);

	return (EOK);
}

int netbuf_process_out(struct packet **pkt, net_interface_t **intf)
{
	if (!pkt || !intf)
		return EINVAL;

	if (cbuffer_is_empty(&out_cb)) {
		return EAGAIN;
	}

	*pkt = out_queue[out_cb.head].pkt;
	*intf = out_queue[out_cb.head].itf;

	cbuffer_remove(&out_cb);

	return (EOK);
}

void netbuf_wait()
{
	if (EOK != wait_for_barrier(netb))
		fprintf(stderr, "%s failed wait_for_barrier\n", __FUNCTION__);

	if (barrier_close(netb))
		fprintf(stderr, "%s failed barrier_close\n", __FUNCTION__);
}