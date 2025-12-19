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

#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#include "list_type.h"

#define LIST_STATIC_INIT(listname) \
list_inst listname = {            \
    .head = NULL,                   \
    .tail = NULL,                   \
    .counter = 0                    \
}

int list_init(list_inst * list);

int list_add(list_inst * list, list_node * prev, list_node * data);

inline int list_append(list_inst *list, list_node *data)
{
	return (list_add(list, list->tail, data));
}

int list_remove(list_inst * list, list_node * data);

list_node *list_search(list_inst * list, void *param, BOOL(*fn) (list_node *, void *));

inline void *list_head(list_inst *list)
{
	return ((list) ? (list->head) : (NULL));
}

inline void *list_tail(list_inst *list)
{
	return ((list) ? (list->tail) : (NULL));
}

inline uint32_t list_count(list_inst *list)
{
	return ((list) ? (list->counter) : (0));
}

#endif				// LIST_H_INCLUDED
