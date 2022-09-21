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
#define MAGIC_LAST_NUMBER  (0xDEADBEEFL)

/*
 * Structure of the list
 *
 * (1)			<-- heap_start, allocated
 * MAGIC_ALLOC_NUMBER
 * ....
 * ....
 * ....
 * (2)
 * MAGIC_ALLOC_NUMBER	<-- next item, allocated
 * ....
 * ....
 * ....
 * ....
 * (3)
 * MAGIC_FREE_NUMBER	<-- next item, free'd
 * ....
 * ....
 * ....
 * (heap_end)
 * MAGIC_LAST_NUMBER	<-- last item, unusable
 */

/*
 * 8 bytes for 32-bit platforms
 * 16 bytes for 64-bit platforms
 */

struct mempart {
	struct mempart *next;
	uintptr_t magic;
};

static struct mempart *heap_start = NULL;
static struct mempart *heap_end = NULL;
/*
 * Yes this should definitively be some typecasted variable,
 * with well known size.
 * But, we rely on common sense.
 */
static unsigned long freebytes = 0;
static unsigned long allocbytes = 0;

static inline int is_alloc(struct mempart *p)
{
	return (MAGIC_ALLOC_NUMBER == p->magic) ? 1 : 0;
}

static inline int is_free(struct mempart *p)
{
	return (MAGIC_FREE_NUMBER == p->magic) ? 1 : 0;
}

static inline int is_last(struct mempart *p)
{
	return (MAGIC_LAST_NUMBER == p->magic) ? 1 : 0;
}

/*
 * Available space is equal to the difference between
 * the next descriptor pointer and the actual (this)
 * descriptor pointer, less the size of "this".
 * As it turns out, advancing the "this" pointer by 1 makes
 * the trick.
 */
static inline unsigned long get_size(struct mempart *p)
{
	return ((uintptr_t) p->next - (uintptr_t) (p + 1));
}

static void defrag_mem(void)
{
	struct mempart *this = heap_start;
	struct mempart *merge = NULL;
	unsigned long bytes = 0;

	while (!is_last(this)) {
		if (is_free(this)) {
			merge = this;
			this = this->next;
			while (is_free(this)) {
				this = this->next;
				/*
				 * Account for mempart structures
				 * becoming part of the free space
				 */
				bytes += sizeof(struct mempart);
			}
			/*
			 * We fall here even if this points to the last
			 * item, as it fails the is_free() test just like
			 * an allocated item.
			 */
			if (merge->next != this) {
				merge->next = this;
			}
		}
		/*
		 * This works on the last item as it points to itself.
		 */
		this = this->next;
	}
	freebytes += bytes;
	allocbytes -= bytes;
}

/*
 * This should not be called by platform specific code,
 * to avoid cross functional issues.
 * Try passing heap values to the kernel
 */
STATUS malloc_init(const void *heapstart, const void *heapend)
{
	if (heapstart > heapend)
		return (EINVAL);

	if (((uintptr_t) (heapstart) ^ (uintptr_t) (heapend)) & (sizeof(struct mempart) - 1))
		return (EINVAL);

	heap_start = (struct mempart *)heapstart;
	heap_end = (struct mempart *)heapend;
	/*
	 * Step back one structure, as the end of the heap IS
	 * the end of the heap !!!
	 */
	heap_end -= sizeof(*heap_end);

	/*
	 * Link start and end of the heap
	 */
	heap_start->next = heap_end;
	heap_start->magic = MAGIC_FREE_NUMBER;

	heap_end->next = heap_end;
	heap_end->magic = MAGIC_LAST_NUMBER;

	/*
	 * Account for free and allocated space
	 */
	allocbytes = 2 * sizeof(struct mempart);
	freebytes = (uintptr_t) (heap_end) - (uintptr_t) (heap_start) - allocbytes;

	return (EOK);
}

static void *malloc_internal(size_t newsize)
{
	struct mempart *this = heap_start;
	struct mempart *middle;
	void *split;
	unsigned long available;

	while (!is_last(this)) {
		while (is_alloc(this))
			this = this->next;
		if (is_last(this))
			break;

		available = get_size(this);
		/* 
		 * Available space cannot fit the requested size, keep on searching
		 */
		if (available < newsize) {
			this = this->next;
			continue;
		}
		/*
		 * This slot can be allocated.
		 */
		this->magic = MAGIC_ALLOC_NUMBER;
		/*
		 * Available space fits the requested size; the slot is considered
		 * to be a good fit if it offers no more than sizeof(struct mempart)
		 * bytes in addition to newsize.
		 * This check is required to understand if we could split the slot
		 * leaving free space.
		 * In other words, if we have residual space for a struct mempart and more,
		 * we can fragment the memory leaving some free space.
		 */
		if (available < newsize + sizeof(struct mempart)) {
			allocbytes += available;
			freebytes -= available;
			return (void *)(this + 1);
		}
		/*
		 * Available space fits the requested size and new free space 
		 * can be carved from this slot.
		 */
		split = (void *)(this + 1) + newsize;
		middle = (struct mempart *)split;
		middle->magic = MAGIC_FREE_NUMBER;
		middle->next = this->next;
		this->next = middle;
		allocbytes += newsize + sizeof(struct mempart);
		freebytes -= newsize + sizeof(struct mempart);
		return (void *)(this + 1);
	}

	return NULL;
}

void *malloc(size_t size)
{
	size_t newsize;
	void *retval;

	if (!size) {
		return (NULL);
	}

	/*
	 * Make size a nice multiple of.
	 */
	newsize = MULT(size, sizeof(void *));
	retval = malloc_internal(newsize);
	/*
	 * First run was unsuccessful, try defragmenting the list
	 */
	if (!retval) {
		defrag_mem();
		retval = malloc_internal(newsize);
	}

	/*
	 * Nope, we cannot accomodate this request...
	 */
	if (!retval)
		errno = ENOMEM;
	return (retval);
}

void *realloc(void *p, size_t size)
{
	struct mempart *this = (struct mempart *)p;
	void *temp;
	unsigned long oldsize;

	if (!p)
		return malloc(size);

	if (!size) {
		free(p);
		return (NULL);
	}

	this--;
	if (!is_alloc(this)) {
		errno = ENOMEM;
		return (NULL);
	}

	/* Now pointer is valid */
	oldsize = get_size(this);
	if (oldsize >= size) {
		return (p);
	}

	/* Need more space, alloc */
	temp = malloc(size);
	if (temp) {
		memcpy(temp, p, oldsize);
		free(p);
	} else {
		errno = ENOMEM;
	}
	return (temp);
}

void free(void *p)
{
	struct mempart *this = (struct mempart *)p;

	if (!p)
		return;

	this--;
	assert(is_alloc(this) == 1);
	this->magic = MAGIC_FREE_NUMBER;
	allocbytes -= get_size(this);
	freebytes += get_size(this);
}

/*
 * This is a custom call for DiegOS
 */
void diegos_malloc_stats(unsigned long bytes[2])
{
	bytes[0] = freebytes;
	bytes[1] = allocbytes;
}
