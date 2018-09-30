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

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "queue_type.h"

STATUS queue_init(queue_inst *queue);

STATUS queue_enqueue(queue_inst *queue, queue_node *data);

STATUS queue_dequeue(queue_inst *queue, queue_node **data);

STATUS queue_roll(queue_inst *queue);

STATUS queue_insert(queue_inst *queue, queue_node *data, queue_node *after);

inline void *queue_head(queue_inst *queue)
{
    return ((queue) ? (queue->head) : (NULL));
}

inline void *queue_tail(queue_inst *queue)
{
    return ((queue) ? (queue->tail) : (NULL));
}

inline unsigned queue_count(queue_inst *queue)
{
    return ((queue) ? (queue->counter) : (0));
}

#endif
