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

#ifndef CBUFFERS_H_INCLUDED
#define CBUFFERS_H_INCLUDED

/*
 * Some practical examples to use these inline library:
 *
 * (1) Storing bytes in a buffer (i.e. serial line, keyboard strokes)
 *
 * struct cbuffer mybuf = {
 *      .read = 0,
 *      .write = 0,
 *      .bufsize = 128,
 * };
 *
 * char bytes[128];
 *
 * while (cbuffer_space(&mybuf) && !timeout()) {
 *    bytes[mybuf.write] = read();
 *    cbuffer_add(&mybuf);
 * }
 *
 * while (!cbuffer_is_empty(&mybuf)) {
 *    printf("%c",bytes[mybuf.read]);
 *    cbuffer_remove(&mybuf);
 * }
 *
 *     +-----------------+
 *     |                 |
 *     +-----------------+
 *     |        O - - - -|- - - > READ    removing items moves READ to WRITE
 *     +-----------------+
 *     |                 |         |
 *     +-----------------+         |
 *     |                 |         V
 *     +-----------------+
 *     |        O - - - -|- - - > WRITE    adding items move WRITE to the back of READ
 *     +-----------------+
 *     |                 |
 *     +-----------------+
 *     |                 |
 *     +-----------------+
 *     |                 |
 *     +-----------------+
 *     |                 |
 *     +-----------------+
 *
 * 
 * 
 *     +-----------------+
 *     |                 |
 *     +-----------------+
 *     |        O - - - -|- - - > READ, WRITE equal means the buffer is empty
 *     +-----------------+
 *     |                 |
 *     +-----------------+
 *     |                 |
 *     +-----------------+
 *
 * 
 * 
 *     +-----------------+
 *     |                 |
 *     +-----------------+
 *     |        O - - - -|- - - > WRITE next to READ means the buffer is full. This position is free!
 *     +-----------------+
 *     |        O - - - -|- - - > READ
 *     +-----------------+
 *     |                 |
 *     +-----------------+
 *
 */

struct cbuffer {
	/*
	 * The first element in the buffer, or the fist element to be removed.
	 * Reading from the buffer (removing items from the buffer) means moving
	 * the read pointer to the write pointer.
	 * read points to the next item to be read.
	 */
	unsigned read;
	/*
	 * The last element in the buffer, or the first free element.
	 * Writing to the buffer (adding items to the buffer) means moving
	 * the write pointer away from the read pointer.
	 * write points to the next free item to be written.
	 */
	unsigned write;
	/*
	 * The total size items in the buffer; this is the maximum difference
	 * from tail to head, equalling the maximum storage capability minus 1.
	 * When the buffer is full, the last item cannot be stored in the buffer,
	 * so that the total amount of items in the buffer is bufsize - 1.
	 */
	unsigned bufsize;
};

#define CBUFFER_STATIC_INIT(cbuffername, buffersize) \
struct cbuffer listname = {     \
    .read = 0,                  \
    .write = 0,                  \
    .bufsize = buffersize       \
}

inline void cbuffer_init(struct cbuffer *cbuf, unsigned buffersize)
{
	cbuf->read = cbuf->write = 0;
	cbuf->bufsize = buffersize;
}

inline void cbuffer_reset(struct cbuffer *cbuf)
{
	cbuf->read = cbuf->write = 0;
}

inline void cbuffer_add(struct cbuffer *cbuf)
{
	cbuf->write++;
	if (cbuf->write == cbuf->bufsize)
		cbuf->write = 0;
}

inline void cbuffer_remove(struct cbuffer *cbuf)
{
	cbuf->read++;
	if (cbuf->read == cbuf->bufsize)
		cbuf->read = 0;
}

inline void cbuffer_add_n(struct cbuffer *cbuf, unsigned n)
{
	cbuf->write += n;
	if (cbuf->write >= cbuf->bufsize)
		cbuf->write -= cbuf->bufsize;
}

inline void cbuffer_remove_n(struct cbuffer *cbuf, unsigned n)
{
	cbuf->read += n;
	if (cbuf->read >= cbuf->bufsize)
		cbuf->read -= cbuf->bufsize;
}

inline unsigned cbuffer_free_space(struct cbuffer *cbuf)
{
	if (cbuf->read <= cbuf->write) {
		/*return (cbuf->bufsize - (cbuf->write - cbuf->read) - 1); */
		return (cbuf->bufsize - cbuf->write + cbuf->read - 1);
	} else {
		return (cbuf->read - cbuf->write - 1);
	}
}

/*
 * This function is used to trim the free space to be contiguous,
 * so that the buffer can be filled with a single memcpy() call.
 */
inline unsigned cbuffer_free_space_trim(struct cbuffer *cbuf)
{
	unsigned retval;

	if (cbuf->read <= cbuf->write) {
		retval = cbuf->bufsize - cbuf->write;
		if (cbuf->read == 0)
			retval--;
	} else {
		retval = cbuf->read - cbuf->write - 1;
	}

	return retval;
}

/*
 * Cannot be more than cbuf->bufsize - 1
 */
inline unsigned cbuffer_in_use(struct cbuffer *cbuf)
{
	if (cbuf->read <= cbuf->write) {
		return (cbuf->write - cbuf->read);
	} else {
		return (cbuf->bufsize - cbuf->read + cbuf->write);
	}
}

/*
 * This function is used to trim the used space to be contiguous,
 * so that the buffer can be (partially) emptied with a single memcpy() call.
 */
inline unsigned cbuffer_in_use_trim(struct cbuffer *cbuf)
{
	if (cbuf->read <= cbuf->write) {
		return (cbuf->write - cbuf->read);
	} else {
		return (cbuf->bufsize - cbuf->read);
	}
}

/*
 * the buffer is full when tail + 1 equals head (wrapped)
 */
inline BOOL cbuffer_is_full(struct cbuffer *cbuf)
{
	return (cbuffer_free_space(cbuf) == 0) ? TRUE : FALSE;
}

/*
 * The buffer is empty when head and tail are equal
 */
inline BOOL cbuffer_is_empty(struct cbuffer *cbuf)
{
	return (cbuf->read == cbuf->write) ? TRUE : FALSE;
}

#endif				// CBUFFERS_H_INCLUDED
