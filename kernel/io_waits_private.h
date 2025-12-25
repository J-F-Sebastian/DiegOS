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

#ifndef IO_WAITS_PRIVATE_H_INCLUDED
#define IO_WAITS_PRIVATE_H_INCLUDED

#include <diegos/io_waits.h>

enum {
	IO_WAIT_DEFAULT,
	IO_WAIT_POLL
};

struct wait_queue_item {
	list_node header;
	unsigned char flags;
	union {
		uint8_t tid;
		void *pt;
	};
};

/*
 * Initialize the io_waits library.
 * Must be called internally by kernel init routine.
 *
 * RETURN VALUES
 * TRUE if initialization succeded.
 * FALSE in any other case
 *
 */
BOOL init_io_waits_lib(void);

struct wait_queue_item *io_wait_get_item(void);

void io_wait_put_item(struct wait_queue_item *item);

int io_wait_add(struct wait_queue_item *item, wait_queue_t * wq);

int io_wait_remove(struct wait_queue_item *item, wait_queue_t * wq);

void resume_on_io_wait(void);

#endif				// IO_WAITS_PRIVATE_H_INCLUDED
