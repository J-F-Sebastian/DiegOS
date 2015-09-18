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

#include	<types_common.h>
#include	<stdlib.h>
#include	<errno.h>

static const uintptr_t MAGIC_ALLOC_NUMBER = 0x1A2B3C4D;
static const uintptr_t MAGIC_FREE_NUMBER = 0x5E6F8A9B;
static size_t free_memory = 0;
static void *heap_start = NULL;
static void *heap_end = NULL;

/*
 * Structure of the list
 *
 * next == (2)
 * ....
 * ....
 * MAGIC_NUMBER
 * (ptr value returned)
 *
 *
 *
 * (2) next == (3)
 * ....
 * ....
 * MAGIC_NUMBER
 * (ptr value returned)
 * ....
 * ....
 */

typedef struct _list_next {
    struct _list_next *next;
    uintptr_t magic;
} list_next;

/*
 * This should not be called by platform specific code,
 * to avoid cross functional issues.
 * Try passing heap values to the kernel
 */
STATUS malloc_init(const void *heapstart, const void *heapend)
{
    list_next *temp;

    if (heapstart > heapend) return (EINVAL);

    heap_start = (void *)heapstart;
    heap_end = (void *)heapend;

    free_memory = (uintptr_t)heap_end - (uintptr_t)heap_start;

    temp = (list_next *)heap_start;
    temp->next = heap_end;
    temp->magic = MAGIC_FREE_NUMBER;

    return (EOK);
}

void *malloc (size_t size)
{
    list_next *this = heap_start;
    list_next *split;
    size_t newsize;

    if (!size) {
        return (NULL);
    }

    /*
     * Make size accomodate pointers, magic number and the user data,
     * ending on a (void *) boundary.
     */
    newsize = MULT(size, sizeof(void *));

    if (newsize > free_memory) {
        errno = ENOMEM;
        return (NULL);
    }

    while (this != (list_next *)heap_end) {
        if (MAGIC_FREE_NUMBER == this->magic) {
            if ((uintptr_t)this->next - (uintptr_t)this >
                    (newsize + 2*sizeof(list_next))) {
                split = (void *)this + newsize + sizeof(*this);
                split->next = this->next;
                split->magic = MAGIC_FREE_NUMBER;

                this->next = split;
                this->magic = MAGIC_ALLOC_NUMBER;

                free_memory -= newsize + 2*sizeof(list_next);

                return ((void *)this+sizeof(*this));

            } else if ((uintptr_t)this->next - (uintptr_t)this >=
                       (newsize + sizeof(list_next))) {

                this->magic = MAGIC_ALLOC_NUMBER;

                free_memory -= (uintptr_t)this->next - (uintptr_t)this;

                return ((void *)this+sizeof(*this));

            }
        }

        this = this->next;
    }

    errno = ENOMEM;
    return (NULL);
}

void *realloc(void *p, size_t size)
{
    return (NULL);
}

void free (void *p)
{
    list_next *this = heap_start;
    list_next *merge = NULL;

    if (!p) return;

    while (this != (list_next *)heap_end) {
        if ((list_next *)p > this->next) {
            if (MAGIC_FREE_NUMBER == this->magic) {
                merge = this;
            }
            this = this->next;
        } else {
            /* ptr is inside this chunk */
            /* we need asserts !!! */
            if (MAGIC_ALLOC_NUMBER != this->magic) {
                /* Double free !!! */
                return;
            }

            this->magic = MAGIC_FREE_NUMBER;
            free_memory += (uintptr_t)this->next - (uintptr_t)this;

            if (merge && (merge->next == this)) {
                /* CASE 1: the previous chunk is free,
                 * merge & set free
                 */
                merge->next = this->next;
            } else if ((this->next != heap_end) &&
                       (MAGIC_FREE_NUMBER == this->next->magic)) {
                /* CASE 2: the next chunk is free,
                 * merge & set free
                 */
                this->next = this->next->next;
            }

            return;
        }
    }
}

