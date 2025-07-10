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

#include <libs/queue.h>
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

/*
 * poll function performs polling on one or more file descriptors.
 * Polling a file descriptor means checking for any pending I/O events which
 * might be queued by the underlying driver.
 * If no events are present the thread is set to WAIT state
 * and the thread will sleep until at least one file descriptor generates an
 * event.
 * If one or more events are now present, the scheduler will wake up
 * the sleeping thread, and the thread will be able to perform one or more I/O
 * operations as it is guaranteed that at least one operation (read or write)
 * can be performed without being blocked.
 * If no events are present but the timeout has expired, the scheduler will wake up
 * the sleeping thread which is indeed informed that no events happened.
 * The thread should refrain from performing I/O operations on the selected devices,
 * as the thread will surely block.
 *
 * PARAMETERS IN
 * struct pollfd ufds[] - an array of pollfd structures, used to check if files
 *                        have pending I/O events.
 *                        the fd and revents fields MUST BE properly compiled to
 *                        be able to receive notifications of a I/O status change
 * unsigned nfds - the size of pollfd array in items
 * int timeout - the timeout in milliseconds to wait for events before waking up
 *               the waiting thread.
 *
 * RETURNS
 * EINVAL if ufds is NULL or nfds is 0 or greater than the maximum open files.
 * EAGAIN if the operation cannot be performed due to resources starvation
 * ENOSYS if poll cannot be performed by one of the underlying drivers.
 * EPERM if the thread cannot be suspended to wait for completion.
 * EOK if the operation completed successfully.
 */
int poll(struct pollfd ufds[], unsigned nfds, int timeout);

/*
 * poll function performs polling on one or more socket descriptors.
 * Polling a file descriptor means checking for any pending I/O events which
 * might be queued by the underlying driver.
 * If no events are present the thread is set to WAIT state
 * and the thread will sleep until at least one file descriptor generates an
 * event.
 * If one or more events are now present, the scheduler will wake up
 * the sleeping thread, and the thread will be able to perform one or more I/O
 * operations as it is guaranteed that at least one operation (read or write)
 * can be performed without being blocked.
 * If no events are present but the timeout has expired, the scheduler will wake up
 * the sleeping thread which is indeed informed that no events happened.
 * The thread should refrain from performing I/O operations on the selected devices,
 * as the thread will surely block.
 *
 * PARAMETERS IN
 * struct pollfd ufds[] - an array of pollfd structures, used to check if files
 *                        have pending I/O events.
 *                        the fd and revents fields MUST BE properly compiled to
 *                        be able to receive notifications of a I/O status change
 * unsigned nfds - the size of pollfd array in items
 * int timeout - the timeout in milliseconds to wait for events before waking up
 *               the waiting thread.
 *
 * RETURNS
 * EINVAL if ufds is NULL or nfds is 0 or greater than the maximum open files.
 * EAGAIN if the operation cannot be performed due to resources starvation
 * ENOSYS if poll cannot be performed by one of the underlying drivers.
 * EPERM if the thread cannot be suspended to wait for completion.
 * EOK if the operation completed successfully.
 */
int poll_network(struct pollfd ufds[], unsigned nfds, int timeout);

int poll_wait(wait_queue_t * wq, poll_table_t * table);

#endif
