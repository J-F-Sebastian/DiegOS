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

#ifndef LIST_TYPE_H_INCLUDED
#define LIST_TYPE_H_INCLUDED

#include <types_common.h>

typedef struct _list_node {
	struct _list_node *prev;
	struct _list_node *next;
} list_node;

typedef struct _list_inst {
	list_node *head;
	list_node *tail;
	uint32_t counter;
} list_inst;

#endif				// LIST_TYPE_H_INCLUDED
