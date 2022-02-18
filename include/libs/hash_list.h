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

#ifndef HASH_LIST_H_INCLUDED
#define HASH_LIST_H_INCLUDED

/*
 * Generic hash list API.
 * Make use of 16 bit hashes.
 */
#include "hash_list_type.h"

int hash_list_init(hash_list_inst_t * inst, unsigned hash_size);

int hash_list_add(hash_list_inst_t * list, void *item, hash_t hash);

hash_list_item_t *hash_list_get(hash_list_inst_t * list, hash_t hash);

int hash_list_del(hash_list_inst_t * list, hash_t hash);

void hash_list_dump(hash_list_inst_t * list);

int hash_list_done(hash_list_inst_t * inst);

#endif				// HASH_LIST_H_INCLUDED
