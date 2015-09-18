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

#ifndef _CHUNKS_H_
#define _CHUNKS_H_

#include <types_common.h>

typedef struct chunks_pool chunks_pool_t;

/*
 * Create a new memory chunks pool.
 *
 * const char *name - the name of the pool
 * unsigned aln     - alignment of the chunks, if null
 *                    the chunks will be long aligned.
 * unsigned size    - size of a single chunk
 * unsigned numitems    - number of initial chunks in the pool
 * unsigned delta   - number of chunks added to the pool when
 *                    a chunk_pool_malloc() request runs out of memory
 *
 * return NULL on error, a valid pointer on success.
 */
chunks_pool_t *chunks_pool_create(const char *name,
                                  unsigned aln,
                                  unsigned size,
                                  unsigned numitems,
                                  unsigned delta);

/*
 * Get a chunk from the pool
 */
void *chunks_pool_malloc(chunks_pool_t *pool);   

/*
 * Return a chunk to the pool
 */
void chunks_pool_free(chunks_pool_t *pool, void *ptr);

/*
 * Destroys the pool - no destructor is invoked for the chunks,
 * memory will be deallocated only.
 */
void chunks_pool_done(chunks_pool_t *pool);

/*
 * Print pool stats
 */
void chunks_pool_dump(chunks_pool_t *pool);

#endif
