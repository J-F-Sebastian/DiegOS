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
 *      .head = 0,
 *      .tail = 0,
 *      .bufsize = 128,
 *      .bufstep = 1
 * };
 *
 * char bytes[128];
 *
 * while (cbuffer_space(&mybuf) && !timeout()) {
 *    bytes[mybuf.tail] = read();
 *    cbuffer_add(&mybuf);
 * }
 *
 * while (!cbuffer_is_empty(&mybuf)) {
 *    printf("%c",bytes[mybuf.head]);
 *    cbuffer_sub(&mybuf);
 * }
 *
 *
 *  While reading INTO THE BUFFER
 * head ------- tail ---> ...
 *
 * While writing FROM THE BUFFER
 *
 *  ... head ---> tail
 */

struct cbuffer {
    /*
     * The first element of the buffer, or the fist element to be removed.
     * Reading from the buffer (removing items from the buffer) means moving
     * the head to the tail.
     */
    unsigned head;
    /*
     * The last element of the buffer, or the last element added.
     * Writing to the buffer (adding items to the buffer) means moving
     * the tail away from the head.
     */
    unsigned tail;
    /*
     * The total size items in the buffer; this is the maximum difference
     * from tail to head, equalling the maximum storage capability minus bufstep.
     * When the buffer is full, the last item cannot be stored in the buffer,
     * so that the total amount of items in the buffer is bufsize - bufstep.
     */
    unsigned bufsize;
    /*
     * The value to be added to head or tail when advancing their values.
     * the number of advancing steps equals bufsize/bufstep.
     */
    unsigned bufstep;
};

#define CBUFFER_STATIC_INIT(cbuffername, buffersize, step) \
struct cbuffer listname = {     \
    .head = 0,                  \
    .tail = 0,                  \
    .bufsize = buffersize,      \
    .bufstep = step             \
}

extern inline void cbuffer_init (struct cbuffer *cbuf,
                                 unsigned buffersize,
                                 unsigned bufferstep)
{
    cbuf->head = cbuf->tail = 0;
    cbuf->bufsize = buffersize;
    cbuf->bufstep = bufferstep;
    if (buffersize < bufferstep) {
        cbuf->bufstep = cbuf->bufsize;
    }
}

extern inline void cbuffer_add (struct cbuffer *cbuf)
{
    cbuf->tail += cbuf->bufstep;
    cbuf->tail %= cbuf->bufsize;
}

extern inline void cbuffer_remove (struct cbuffer *cbuf)
{
    cbuf->head += cbuf->bufstep;
    cbuf->head %= cbuf->bufsize;
}

extern inline void cbuffer_add_n (struct cbuffer *cbuf, unsigned n)
{
    cbuf->tail += cbuf->bufstep*n;
    cbuf->tail %= cbuf->bufsize;
}

extern inline void cbuffer_remove_n (struct cbuffer *cbuf, unsigned n)
{
    cbuf->head += cbuf->bufstep*n;
    cbuf->head %= cbuf->bufsize;
}

extern inline unsigned cbuffer_free_space (struct cbuffer *cbuf)
{
    if (cbuf->head == cbuf->tail) return (cbuf->bufsize - cbuf->bufstep);
    if (cbuf->head < cbuf->tail) {
        return (cbuf->head + cbuf->bufsize - cbuf->tail - cbuf->bufstep);
    } else {
        return (cbuf->head - cbuf->tail - cbuf->bufstep);
    }
}

extern inline unsigned cbuffer_in_use (struct cbuffer *cbuf)
{
    if (cbuf->head == cbuf->tail) return (0);
    if (cbuf->head < cbuf->tail) {
        return (cbuf->tail - cbuf->head);
    } else {
        return (cbuf->bufsize - cbuf->head + cbuf->tail);
    }
}

extern inline BOOL cbuffer_is_empty (struct cbuffer *cbuf)
{
    return (cbuf->head == cbuf->tail) ? TRUE : FALSE;
}

#endif // CBUFFERS_H_INCLUDED
