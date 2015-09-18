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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "chunks.h"
#include "bitmaps.h"

typedef struct chunks_pool {
    unsigned aln;
    unsigned size;
    unsigned delta;
    list_inst pools;
    char name[16];
} chunks_pool_t;

typedef struct pool_array {
    list_node header;
    void *buffer;
    void *bufstart;
    unsigned numitems;
    unsigned free;
    long *bitmap;
} pool_array_t;


static STATUS create_pool(chunks_pool_t *ptr, unsigned numitems)
{
    unsigned total;
    pool_array_t *tmp;

    total = ptr->size*numitems+ptr->aln;

    tmp = malloc(sizeof(pool_array_t));

    if (!tmp) {
        return (ENOMEM);
    }

    tmp->buffer = malloc(total);

    if (!tmp->buffer) {
        free(tmp);
        return (ENOMEM);
    }

    tmp->bufstart = (void *)MULT((uintptr_t)tmp->buffer, ptr->aln);

    tmp->numitems = numitems;
    tmp->free = numitems;

    tmp->bitmap = malloc(BITMAPBYTES(numitems));

    if (!tmp->bitmap) {
        free(tmp->buffer);
        free(tmp);
        return (ENOMEM);
    }

    memset(tmp->bitmap,0,BITMAPBYTES(numitems));

    if (EOK != list_add(&ptr->pools, NULL, &tmp->header)) {
        free(tmp->bitmap);
        free(tmp->buffer);
        free(tmp);
        return (EGENERIC);
    }

    return (EOK);
}

static void delete_pool(chunks_pool_t *ptr, pool_array_t *dlt)
{
    /*
     * Never remove the main pool!
     */
    if (list_tail(&ptr->pools) == dlt) {
        return;
    }

    if (EOK != list_remove(&ptr->pools, &dlt->header)) {
        return;
    }

    free(dlt->bitmap);
    free(dlt->buffer);
    free(dlt);
}

chunks_pool_t *chunks_pool_create(const char *name,
                                  unsigned aln,
                                  unsigned size,
                                  unsigned numitems,
                                  unsigned delta)
{
    chunks_pool_t *ptr;

    if (!size || !numitems) {
        return (NULL);
    }

    if (!aln) {
        aln = sizeof(long);
    }

    ptr = malloc(sizeof(chunks_pool_t));

    if (!ptr) {
        return (NULL);
    }

    if (name) {
        snprintf(ptr->name, sizeof(ptr->name), "%s", name);
    } else {
        snprintf(ptr->name, sizeof(ptr->name), "Chunk_%p",ptr);
    }

    ptr->aln = aln;
    ptr->size = MULT(size, aln);
    ptr->delta = delta;

    if (EOK != list_init(&ptr->pools)) {
        free(ptr);
        return (NULL);
    }

    if (EOK != create_pool(ptr, numitems)) {
        free(ptr);
        return (NULL);
    }

    return (ptr);
}

void *chunks_pool_malloc(chunks_pool_t *pool)
{
    pool_array_t *ptr;
    unsigned pos;
    STATUS retcode;

    if (!pool) {
        return (NULL);
    }

    ptr = list_head(&pool->pools);

    while (ptr) {
        if (ptr->free) {
            pos = bitmap_first_is_clear(ptr->bitmap, BITMAPLEN(ptr->numitems));
            if (pos < ptr->numitems) {
                --ptr->free;
                bitmap_set(ptr->bitmap, pos);
                return (ptr->bufstart + (pos*pool->size));
            }
        }
        ptr = (pool_array_t *)ptr->header.next;
    }

    /*
     * add another pool to the list, if delta is not 0
     */
    if (pool->delta) {
        retcode = create_pool(pool, pool->delta);
    } else {
        retcode = ENOMEM;
    }
    if (EOK != retcode) {
        return (NULL);
    }

    ptr = list_head(&pool->pools);
    --ptr->free;
    bitmap_set(ptr->bitmap, 0);

    return (ptr->bufstart);
}

void chunks_pool_free(chunks_pool_t *pool, void *ptr)
{
    pool_array_t *cur;
    uintptr_t val;

    if (!pool || !ptr) {
        return;
    }

    cur = list_head(&pool->pools);

    while (cur) {
        if ((ptr >= cur->bufstart) &&
                (ptr <= cur->bufstart + (pool->size*(cur->numitems - 1)))) {
            val = (uintptr_t)ptr;
            val -= (uintptr_t)cur->bufstart;
            val /= (uintptr_t)pool->size;
            if (!bitmap_is_set(cur->bitmap, val)) {
                return;
            }
            bitmap_clear(cur->bitmap, val);
            ++cur->free;
            if (cur->free == cur->numitems) {
                delete_pool(pool, cur);
            }
            return;
        }

        cur = (pool_array_t *)cur->header.next;
    }
}

void chunks_pool_done(chunks_pool_t *pool)
{
    pool_array_t *cur;

    if (!pool) {
        return;
    }

    while (list_count(&pool->pools)) {

        cur = list_head(&pool->pools);

        if (EOK == list_remove(&pool->pools, &cur->header)) {
            free(cur->bitmap);
            free(cur->buffer);
            free(cur);
        } else {
            break;
        }
    }

    free(pool);
}

void chunks_pool_dump(chunks_pool_t *pool)
{
    pool_array_t *cur;
    unsigned i = 0;

    if (!pool) {
        return;
    }

    printf("\nChunk name: %s\n ============\n Alignment..... %u\n"
           " Element size.. %u\n Delta......... %u\n",
           pool->name, pool->aln, pool->size, pool->delta);

    cur = list_head(&pool->pools);

    printf(" ------------\n");

    while (cur) {
        printf("Pool array %u\n", i++);

        printf(" Buffer start 0x%p\n Items... %u\n Free.... %u\n",
               cur->bufstart, cur->numitems, cur->free);

        printf(" ------------\n");

        cur = (pool_array_t *)cur->header.next;
    }
}
