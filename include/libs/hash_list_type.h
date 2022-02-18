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

#ifndef HASH_LIST_TYPE_H_INCLUDED
#define HASH_LIST_TYPE_H_INCLUDED

#include <stdint.h>

typedef uint16_t hash_t;

typedef struct hash_list_item {
	void *data;
	hash_t hash;
	struct hash_list_item *next;
} hash_list_item_t;

typedef struct hash_list_inst {
	/*char *name */
	hash_list_item_t *list;
	unsigned hash_size;
} hash_list_inst_t;

#endif				// HASH_LIST_TYPE_H_INCLUDED
