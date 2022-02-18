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

#ifndef _POLL_H_
#define _POLL_H_

#include <queue.h>
#include <diegos/io_waits.h>

enum {
	POLLIN = (1 << 0),
	POLLRDNORM = (1 << 1),
	POLLRDBAND = (1 << 2),
	POLLPRI = (1 << 3),
	POLLHUP = (1 << 4),
	POLLERR = (1 << 5),
	POLLOUT = (1 << 6),
	POLLWRNORM = (1 << 7),
	POLLWRBAND = (1 << 8)
};

struct pollfd {
	/*
	 * File descriptor for files and raw devices, or ifindex for
	 * network interfaces
	 */
	int fd;
	/*
	 * Events read from the device, file, or network interface
	 */
	short events;
	/*
	 * Events we want to be notified for
	 */
	short revents;
};

typedef struct poll_table poll_table_t;

int poll(struct pollfd ufds[], unsigned nfds, int timeout);

int poll_network(struct pollfd ufds[], unsigned nfds, int timeout);

int poll_wait(wait_queue_t * wq, poll_table_t * table);

#endif
