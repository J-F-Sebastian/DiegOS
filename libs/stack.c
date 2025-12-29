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

#include <errno.h>

#include <libs/stack.h>

STATUS stack_init(queue_inst *queue)
{
	if (!queue) {
		return (EINVAL);
	}

	queue->head = NULL;
	queue->tail = NULL;
	queue->counter = 0;

	return (EOK);
}

STATUS stack_push(queue_inst *queue, queue_node *data)
{
	if (!queue || !data) {
		return (EINVAL);
	}

	if (!queue->head) {
		queue->head = data;
		queue->counter = 1;
		data->next = NULL;
		return (EOK);
	}

	data->next = queue->head;
	queue->head = data;
	queue->counter++;

	return (EOK);
}

STATUS stack_pop(queue_inst *queue, queue_node **data)
{
	if (!queue || !data) {
		return (EINVAL);
	}

	if (!queue->counter) {
		*data = NULL;
		return (EGENERIC);
	}

	*data = queue->head;
	queue->head = queue->head->next;
	(*data)->next = NULL;
	queue->counter--;

	return (EOK);
}
