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

/*
 * Static initializer for global lists
 * to be available prior to calling any
 * function
 */
#define LIST_STATIC_INIT(listname) \
list_inst listname = {            \
    .head = NULL,                   \
    .tail = NULL,                   \
    .counter = 0                    \
}

/*
 * Same initialization as LIST_STATIC_INIT, but dynamic.
 * The list instance must be allocated, no memory
 * management is performed by the function.
 *
 * PARAMS IN
 * list_inst *list - the list instance to be initialized
 *
 * RETURNS
 * EOK    in case of success
 * EINVAL if inst is NULL
 */
int list_init(list_inst * list);

/*
 * Add a node to the list by linking it to prev in different ways:
 * if prev is NULL the data node is inserted at the head of the list,
 * if prev is the last node in list the data node is inserted at the tail
 * if the list, otherwise it is inserted between prev and the prev successor.
 *
 * PARAMS IN
 * list_inst *list - the list instance
 * list_node *prev - the node predecessor, i.e. the node that will be the prev of data.
 * list_node *data - the node to be linked into the list
 *
 * RETURNS
 * EOK    in case of success
 * EINVAL in case list or data are NULL
 */
int list_add(list_inst * list, list_node * prev, list_node * data);

/*
 * Remove (unlink) a node from a list; no memory management is performed on data.
 *
 * PARAMS IN
 * list_inst *list - the list instance
 * list_node *data - the node to be unlinked
 *
 * RETURNS 
 * EOK    in case of success
 * EINVAL if list or data are NULL
 */ 
int list_remove(list_inst * list, list_node * data);

/*
 * Search for specific data by calling fn() function for each node in the list;
 * when the function return value is TRUE the search stops and the node selected
 * for processing is returned.
 * If fn() never returns TRUE, then the search failed and NULL is returned.
 *
 * PARAMS IN
 * list_inst *list - the list instance
 * void *param     - search data, to be used by fn()
 * BOOL(*fn)(list_node *, void *) - function to compare each node with data in param and
 *                                  evaluate if they are equal or not; if they are equal
 *                                  return TRUE.
 *
 * RETURNS
 * NULL in case no node could be found, or a pointer to a node inside the list matching param.
 */
list_node *list_search(list_inst * list, void *param, BOOL(*fn) (list_node *, void *));

/*
 * Utility functions, used to avoid typecasts and unaligned accesses
 */
inline void *list_head(list_inst * list)
{
	return ((list) ? (list->head) : (NULL));
}

inline void *list_tail(list_inst * list)
{
	return ((list) ? (list->tail) : (NULL));
}

inline uint32_t list_count(list_inst * list)
{
	return ((list) ? (list->counter) : (0));
}

inline void *list_node_next(list_node *node)
{
	return (void *)(node->next);
}

inline void *list_node_prev(list_node *node)
{
	return (void *)(node->prev);
}

#endif				// LIST_H_INCLUDED
