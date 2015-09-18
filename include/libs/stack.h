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

#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include "queue_type.h"

STATUS stack_init(queue_inst *queue);

STATUS stack_push(queue_inst *queue, queue_node *data);

STATUS stack_pop(queue_inst *queue, queue_node **data);

extern inline uint32_t stack_count(queue_inst *queue)
{
    return ((queue) ? (queue->counter) : (0));
}

#endif // STACK_H_INCLUDED
