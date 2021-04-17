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

#define MAGIC_ALLOC_NUMBER (0x1A2B3C4DL)
#define MAGIC_FREE_NUMBER  (0x5E6F8A9BL)

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

static inline int is_free (list_next *p)
{
    return (MAGIC_FREE_NUMBER == p->magic) ? 1 : 0;
}

static void defrag_mem (void)
{
    list_next *this = heap_start;
    list_next *merge = NULL;
 
    while (this != this->next) {
        if (is_free(this)) {
            merge = this;
            this = this->next;
            while (is_free(this)) {
                this = this->next;
            }
            if (merge->next != this) {               
               merge->next = this;
            }            
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

    temp = (list_next *)heap_start;
    temp->next = heap_end;
    temp->magic = MAGIC_FREE_NUMBER;

    temp = (list_next *)heap_end;
    temp->next = temp;
    temp->magic = MAGIC_ALLOC_NUMBER;

    return (EOK);
}

static void *malloc_internal (size_t newsize)
{
	list_next *this = heap_start;
    list_next *split;    
    uintptr_t avail;
    
    while (this != this->next) 
	{
		while ((this != this->next) && !is_free(this)) this = this->next;
		if (this == this->next) break;
        avail = (uintptr_t)this->next - (uintptr_t)(this + 1);
        /* 
         * Available space cannot fit the requested size
         */
        if (avail < newsize)
        {
			this = this->next;
			continue;
		}
		/*
		 * Available space fits the requested size
		 */
        if (avail < newsize + sizeof(list_next))
        {
			this->magic = MAGIC_ALLOC_NUMBER;
			return (void *)(this + 1);
		}
        /*
		 * Available space fits the requested size and new free space 
		 * can be carved
		 */
		split = (void *)(this + 1) + newsize;
        split->next = this->next;
        split->magic = MAGIC_FREE_NUMBER;
        this->next = split;        
        this->magic = MAGIC_ALLOC_NUMBER;
        return (void *)(this + 1);        
    }
    
    return NULL;
}

void *malloc (size_t size)
{    
    size_t newsize; 
    void *retval;

    if (!size) {
        return (NULL);
    }

    /*
     * Make size accomodate pointers, magic number and the user data,
     * ending on a (void *) boundary.
     */
    newsize = MULT(size, sizeof(list_next));
	retval = malloc_internal(newsize);
	if (!retval) 
	{
		defrag_mem();
		retval = malloc_internal(newsize);
	}

    if (!retval) errno = ENOMEM;
    return (retval);
}

void *realloc(void *p, size_t size)
{
    list_next *this = (list_next *)p;
    void *temp;
    unsigned oldsize;

    if (!p) return malloc(size);
    if (!size) {
        free(p);
        return (NULL);
    }
        
    this--;
    if ((MAGIC_ALLOC_NUMBER != this->magic) && 
        (MAGIC_FREE_NUMBER != this->magic)) 
    {
        errno = ENOMEM;
        return (NULL);
    }

    /* Now pointer is valid */
    oldsize = (uintptr_t)this->next - (uintptr_t)(this + 1);
    if (oldsize >= size) 
    {
		this->magic = MAGIC_ALLOC_NUMBER;
		return (p);
	}

    /* Need more space, alloc */
    temp = malloc(size);
    if (temp) {
        memcpy(temp, p, oldsize); 
        free(p);
    }
    return (temp);        
}

void free (void *p)
{
    list_next *this = (list_next *)p;

    if (!p) return;
   
    this--;
    assert(MAGIC_ALLOC_NUMBER == this->magic);
    this->magic = MAGIC_FREE_NUMBER;    
}


