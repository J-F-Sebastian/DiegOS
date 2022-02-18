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

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>

#include "hash_list.h"

int hash_list_init(hash_list_inst_t * inst, unsigned hash_size)
{
	void *temp = NULL;

	if (!inst) {
		return EINVAL;
	}

	if (hash_size < 16) {
		hash_size = 16;
	} else if (hash_size > USHRT_MAX) {
		return EINVAL;
	}

	temp = calloc(hash_size, sizeof(hash_list_item_t));

	if (!temp) {
		return ENOMEM;
	}

	inst->list = temp;
	inst->hash_size = hash_size;

	return EOK;
}

int hash_list_add(hash_list_inst_t * list, void *item, hash_t hash)
{
	hash_list_item_t *cursor;

	if (!list || !item) {
		return EINVAL;
	}

	cursor = list->list + (hash % list->hash_size);

	/*
	 * Trivial case: no folding hashes
	 */
	if (!cursor->data) {
		cursor->data = item;
		cursor->hash = hash;
		return EOK;
	}

	/*
	 * Less trivial case: folding hashes
	 */
	while (cursor->next) {
		cursor = cursor->next;
	}

	cursor->next = malloc(sizeof(hash_list_item_t));

	if (cursor->next) {
		cursor = cursor->next;
		cursor->data = item;
		cursor->hash = hash;
		cursor->next = NULL;
		return EOK;
	}

	return EGENERIC;
}

hash_list_item_t *hash_list_get(hash_list_inst_t * list, hash_t hash)
{
	hash_list_item_t *cursor;

	if (!list) {
		return (NULL);
	}

	cursor = list->list + (hash % list->hash_size);

	while (cursor) {
		if (cursor->hash == hash) {
			return (cursor);
		}
		cursor = cursor->next;
	}

	return (NULL);
}

/*
 * Recursive copy and last item deletion
 */
static int hash_list_shrink_row(hash_list_item_t * item)
{
	if (item->next) {
		item->data = item->next->data;
		item->hash = item->next->hash;
		if (hash_list_shrink_row(item->next)) {
			item->next = NULL;
		}
		return (0);
	} else {
		free(item);
		return (1);
	}
}

int hash_list_del(hash_list_inst_t * list, hash_t hash)
{
	hash_list_item_t *cursor, *prev;

	if (!list) {
		return EINVAL;
	}

	cursor = list->list + (hash % list->hash_size);

	/*
	 * No folding is a special case
	 */
	if ((cursor->hash == hash) && !cursor->next) {
		cursor->data = NULL;
		cursor->hash = 0;
		return EOK;
	}

	prev = cursor;
	while (cursor) {
		if (cursor->hash == hash) {
			if (cursor->next) {
				hash_list_shrink_row(cursor);
			} else {
				free(cursor);
				prev->next = NULL;
			}
			return EOK;
		}
		prev = cursor;
		cursor = cursor->next;
	}

	return EGENERIC;
}

/*
 * Recursive deletion
 */
static void hash_list_clear_row(hash_list_item_t * item)
{
	if (item->next) {
		hash_list_clear_row(item->next);
	}
	free(item);
}

int hash_list_done(hash_list_inst_t * inst)
{
	hash_list_item_t *cursor;

	if (!inst) {
		return EINVAL;
	}

	cursor = inst->list;

	while (inst->hash_size--) {
		if (cursor->next) {
			hash_list_clear_row(cursor->next);
		}
		cursor++;
	}

	free(inst->list);
	inst->list = NULL;

	return (EOK);
}

void hash_list_dump(hash_list_inst_t * list)
{
	unsigned i = 0;
	hash_list_item_t *item, *row;

	if (!list) {
		printf("List is null!\n");
		return;
	}

	item = list->list;

	printf(" === Start of Dump ===\n");

	while (i < list->hash_size) {

		if (item->data) {
			printf("Item at row %-4u is %-40s, hash 0x%X\n", i, (char *)item->data,
			       item->hash);
			if (item->next) {
				printf("*** Sharing row with ");
				row = item->next;
				while (row) {
					printf("%s,", (char *)row->data);
					row = row->next;
				}
				printf("\n");
			}
		}
		++item;
		++i;
	}

	printf(" === End of Dump ===\n");
}
