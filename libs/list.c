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

#include "list.h"

int list_init(list_inst *list)
{
    if (!list) {
        return (EINVAL);
    }

    list->head = NULL;
    list->tail = NULL;
    list->counter = 0;

    return (EOK);
}


int list_add(list_inst *list, list_node *prev, list_node *data)
{
    if (!list || !data) {
        return (EINVAL);
    }

    if (!list->counter) {
        list->head = list->tail = data;
        list->counter = 1;
        data->prev = data->next = NULL;
        return (EOK);
    }

    data->prev = prev;
    list->counter++;

    /* insert head */
    if (!prev) {
        data->next = list->head;
        list->head->prev = data;
        list->head = data;
    } else if (prev == list->tail) {
        /*insert tail */
        data->next = NULL;
        list->tail->next = data;
        list->tail = data;
    } else {
        data->next = prev->next;
        prev->next = data;
        data->next->prev = data;
    }

    return (EOK);
}

int list_remove(list_inst *list, list_node *data)
{
    if (!list || !list->counter || !data) {
        return (EINVAL);
    }

    list->counter--;

    if (list->head == data) {
        list->head = data->next;
    }

    if (list->tail == data) {
        list->tail = data->prev;
    }

    if (data->prev) {
        data->prev->next = data->next;
    }

    if (data->next) {
        data->next->prev = data->prev;
    }

    data->prev = data->next = NULL;

    return (EOK);
}

list_node *list_search(list_inst *list,
                       void *param,
                       BOOL (*fn)(list_node *, void *))
{
    list_node *cursor;

    if (!list || !list->counter || !fn) {
        return (NULL);
    }

    cursor = list->head;

    while (cursor) {
        if (fn(cursor, param)) {
            return (cursor);
        }
        cursor = cursor->next;
    }

    return (NULL);
}

