/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2022 Diego Gallizioli
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

#ifndef _IOMALLOC_H_
#define _IOMALLOC_H_

#include <stddef.h>

/*
 * Allocate I/O memory, which is guaranteed to be uncached or to be provided
 * with an I/O coherency protection scheme.
 * The returned pointer is aligned to the required address - or to the natural
 * alignment of the platform.
 *
 * PARAMETERS IN
 * size_t size - the size of the requested memory area
 * size_t aln - the memory alignement, can be 0, in this case the natural alignement
 *              of the platform will be used.
 *
 * RETURNS
 * A valid pointer or NULL in case of failure.
 */

void *iomalloc(size_t size, size_t aln);

/*
 * Release I/O memory previously allocated by iomalloc.
 *
 * PARAMETERS IN
 *  void *ptr - a valid I/O memory pointer
 */
void iofree(void *ptr);

#endif
