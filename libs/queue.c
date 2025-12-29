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

#include <libs/queue.h>

STATUS queue_init(queue_inst *queue)
{
	if (!queue) {
		return (EINVAL);
	}

	queue->head = NULL;
	queue->tail = NULL;
	queue->counter = 0;

	return (EOK);
}

STATUS queue_enqueue(queue_inst *queue, queue_node *data)
{
	if (!queue || !data) {
		return (EINVAL);
	}

	if (!queue->head) {
		queue->head = queue->tail = data;
		queue->counter = 1;
		data->next = NULL;
		return (EOK);
	}

	queue->tail->next = data;
	queue->tail = data;
	queue->counter++;
	data->next = NULL;

	return (EOK);
}

STATUS queue_dequeue(queue_inst *queue, queue_node **data)
{
	if (!queue || !data) {
		return (EINVAL);
	}

	if (!queue->counter) {
		*data = NULL;
		return (EAGAIN);
	}

	*data = queue->head;
	queue->head = queue->head->next;
	(*data)->next = NULL;
	queue->counter--;

	if (!queue->counter) {
		queue->tail = NULL;
	}

	return (EOK);
}

STATUS queue_roll(queue_inst *queue)
{
	if (!queue) {
		return (EINVAL);
	}

	if (queue->counter > 1) {
		queue->tail->next = queue->head;
		queue->head = queue->head->next;
		queue->tail = queue->tail->next;
		queue->tail->next = NULL;
	}

	return (EOK);
}

STATUS queue_insert(queue_inst *queue, queue_node *data, queue_node *after)
{
	if (!queue || !data) {
		return (EINVAL);
	}

	if (!queue_count(queue)) {
		return (queue_enqueue(queue, data));
	}

	if (!after) {
		data->next = queue->head;
		queue->head = data;
		queue->counter++;
	} else {
		data->next = after->next;
		after->next = data;
		queue->counter++;
		if (!data->next) {
			queue->tail = data;
		}
	}

	return (EOK);
}
