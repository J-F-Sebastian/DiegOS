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

#include <iomalloc.h>
#include <types_common.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
 * 8 bytes for 32-bit platforms
 * 12/16 bytes for 64-bit platforms
 */

struct mempart {
	uintptr_t pointer;
	size_t size;
};

/*
 * I/O locations
 */
static struct mempart storage[MAX_IO_ALLOCS];
/*
 * number of locations in use
 */
static unsigned stored = 0;
/*
 * Beginning of I/O memory
 */
static uintptr_t start_of_memory = 0;
/*
 * End of I/O memory
 */
static uintptr_t end_of_memory = 0;

/*
 * Yes this should definitively be some typecasted variable,
 * with well known size.
 * But, we rely on common sense.
 */
static unsigned long freebytes = 0;
static unsigned long allocbytes = 0;

/*
 * Sorting routine
 */
static int compare(const void *a, const void *b)
{
	struct mempart *aa = (struct mempart *)a;
	struct mempart *bb = (struct mempart *)b;

	if (aa->pointer > bb->pointer)
		return 1;
	if (aa->pointer < bb->pointer)
		return -1;
	return 0;
}

/*
 * This should not be called by platform specific code,
 * to avoid cross functional issues.
 */
STATUS iomalloc_init(const void *start, const void *end)
{
	if (start > end)
		return (EINVAL);

	if ((uintptr_t) (start) & (sizeof(long) - 1))
		return (EINVAL);

	if ((uintptr_t) (end) & (sizeof(long) - 1))
		return (EINVAL);

	start_of_memory = (uintptr_t) start;
	end_of_memory = (uintptr_t) end;
	allocbytes = 0;
	freebytes = end_of_memory - start_of_memory;
	memset(storage, 0, sizeof(storage));

	return (EOK);
}

static void *simple_alloc(struct mempart *location, uintptr_t cursor, size_t aln, size_t newsize)
{
	uintptr_t temp;

	temp = location->pointer = MULT(cursor, aln);
	location->size = newsize;
	/*
	 * This check is meaningful only when the first free storage slot
	 * is in use, but limiting the check would probably make things more
	 * complicated than they are.
	 * So we always check if we are overrunning the buffer, actually this
	 * can happen only when not allocating memory in a carved hole, but
	 * we do not care.
	 */
	if (location->pointer + location->size > end_of_memory) {
		temp = location->pointer = location->size = 0;
	} else {
		allocbytes += newsize;
		freebytes -= newsize;
		++stored;
	}

	return (void *)(temp);
}

static void *iomalloc_internal(size_t newsize, size_t aln)
{
	uintptr_t cursor;
	struct mempart *this = storage;
	struct mempart *dst = storage + stored;
	struct mempart *src = dst - 1;

	if (stored == MAX_IO_ALLOCS)
		return (NULL);

	if (stored == 0)
		return simple_alloc(this, start_of_memory, aln, newsize);

	/*
	 * Loop locations in use, there might be a hole
	 * to be filled.
	 * If no hole is found, 'this' will point to the first unused storage slot,
	 * and its content will be zeroed (i.e. pointer == 0).
	 * This condition is detected at the end of the loop.
	 * The value assigned to cursor is the first available location in memory, i. e.
	 * the memory starting at the end of the last allocated block of memory.
	 * NOTE: The code below is guaranteed to have at least one item in storage,
	 * and at least one item free.
	 */
	while (this->pointer) {
		/*
		 * Do we have a memory hole?
		 */
		if (this->pointer + this->size < (this + 1)->pointer) {
			/*
			 * Is it big enough?
			 */
			cursor = MULT(this->pointer + this->size, aln);
			if (cursor + newsize < (this + 1)->pointer) {
				/*
				 * Keep in mind that this code will not be invoked if
				 * 'stored' equals MAX_IO_ALLOCS; so 'stored' points to the
				 * first free item in storage.
				 * All slots following 'this' are moved one step down to the end
				 * of the storage.
				 * 'this' is now a hole to be used by simple_malloc, i.e. to be overwritten.
				 * NOTE: carving a hole keeps the set ordered!
				 */

				while (dst != this)
					*dst-- = *src--;

				/*
				 * Break! We carved a hole.
				 */

				break;
			}
		}
		++this;
	}

	/*
	 * This is guaranteed to point to a valid storage item, unused items
	 * are zeroed.
	 */
	if (this->pointer == 0) {
		cursor = MULT((this - 1)->pointer + (this - 1)->size, aln);
	}

	return simple_alloc(this, cursor, aln, newsize);
}

void *iomalloc(size_t size, size_t aln)
{
	size_t newsize;
	void *retval;

	if (!size)
		return (NULL);

	/*
	 * Make aln a nice multiple of.
	 */
	if (!aln)
		aln = sizeof(long);

	/*
	 * Make newsize be a multiple of long
	 * */
	newsize = ALN(size, sizeof(long));
	retval = iomalloc_internal(newsize, aln);

	/*
	 * Nope, we cannot accomodate this request...
	 */
	if (!retval)
		errno = ENOMEM;

	return (retval);
}

/*
 * NOTE: deletion keeps the set ordered!
 */
void iofree(void *ptr)
{
	struct mempart key = {
		.pointer = (uintptr_t) ptr,
		.size = 0
	};
	struct mempart *retval = NULL;

	if (!ptr)
		return;

	retval = bsearch(&key, storage, stored, sizeof(storage[0]), compare);

	//FIXME should assert ?
	if (!retval)
		return;

	freebytes += retval->size;
	allocbytes -= retval->size;

	while (retval < storage + stored - 1) {
		*retval = *(retval + 1);
		++retval;
	}

	retval->pointer = retval->size = 0;
	stored--;
}

/*
 * This is a custom call for DiegOS
 */
void diegos_iomalloc_stats(unsigned long bytes[2])
{
	bytes[0] = freebytes;
	bytes[1] = allocbytes;
}
