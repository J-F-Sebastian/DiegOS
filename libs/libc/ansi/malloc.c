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

#include <types_common.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#define MAGIC_ALLOC_NUMBER (0x1A2B3C4D)
#define MAGIC_FREE_NUMBER  (0x5E6F8A9B)
#define MAGIC_LAST_NUMBER  (0xDEADBEEF)

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

/*
 * 16 bytes for 32-bit platforms
 * 32 bytes for 64-bit platforms
 */
  
typedef struct _list_next {
    struct _list_next *next;
    uintptr_t magic;
} list_next;


static inline unsigned 
free_user_mem (list_next *start, list_next *end)
{
     return ((uintptr_t)end - (uintptr_t)start);
}

static inline unsigned 
free_priv_mem (list_next *start, list_next *end)
{
     return ((uintptr_t)end - (uintptr_t)start - sizeof(*start));
}

static inline int
is_free (list_next *p)
{
    return (MAGIC_FREE_NUMBER == p->magic) ? 1 : 0;
}

static inline int
is_last (list_next *p)
{
    return (p->next) ? 0 : 1;
}

static void defrag_mem (void)
{
    list_next *this = heap_start;
    list_next *merge = NULL;
 
    while (!is_last(this)) {
        if (is_free(this)) {
            merge = this;
            while (!is_last(this) && is_free(this)) {
                this = this->next;
            }
            if (merge->next != this) {
               free_memory += free_priv_mem(merge, this);
               merge->next = this;
            }
            if (is_last(this)) return;
        }
        this = this->next;
   }
}

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
    heap_end -= sizeof(list_next);

    free_memory = free_priv_mem(heap_start, heap_end);

    temp = (list_next *)heap_start;
    temp->next = heap_end;
    temp->magic = MAGIC_FREE_NUMBER;

    temp = (list_next *)heap_end;
    temp->next = 0;
    temp->magic = MAGIC_LAST_NUMBER;

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
    newsize = MULT(size, sizeof(*this));

    if (newsize > free_memory) {
        defrag_mem();
        if (newsize > free_memory) {
            errno = ENOMEM;
            return (NULL);
        }
    }

    while (!is_last(this)) {
        if (is_free(this)) {
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
    list_next *this = (list_next *)p;

    if (!p) return;
   
    this--;
    assert(MAGIC_ALLOC_NUMBER == this->magic);
    this->magic = MAGIC_FREE_NUMBER;
    free_memory += (uintptr_t)this->next - (uintptr_t)this;
}

