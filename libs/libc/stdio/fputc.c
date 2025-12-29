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

#include <stdio.h>
#include <stdlib.h>
#include "loc_incl.h"

int fputc(int c, FILE *stream)
{
	/*
	 * First stage: check stream validity and update flags
	 * in case of error.
	 * If the stream is good switch it to IOBUF_WRITING.
	 */
	if (!stream) {
		return (EOF);
	}

	if (!stream_w(stream)) {
		return (EOF);
	}

	if (stream_rding(stream)) {
		if (!feof(stream)) {
			return (EOF);
		}
		stream_clearflags(stream, IOBUF_READING);
	}

	stream_setflags(stream, IOBUF_WRITING);

	/*
	 * Second stage: the stream is unbuffered, call writebuffern
	 */
	if (stream_nbuf(stream)) {
		return writebuffern(c, stream);
	}

	/*
	 * Third stage: the stream is buffered, allocate the buffer if
	 * necessary.
	 * In case of failure the stream is configured as unbuffered and
	 * processed as such.
	 */
	if (!stream->buffer) {
		if (!(stream->buffer = (char *)malloc(BUFSIZ))) {
			stream_setflags(stream, IOBUF_NBUF);
			stream->count = stream->bufsize = stream->validsize = 0;
			return writebuffern(c, stream);
		} else {
			stream_setflags(stream, IOBUF_RELBUF);
			stream->count = stream->bufsize = stream->validsize = BUFSIZ;
		}
		stream->bufptr = stream->buffer;
	}

	/*
	 * Fourth stage: process buffers
	 */
	if (stream_lbuf(stream)) {
		return writebufferl(c, stream);
	} else {
		return (writebuffer(c, stream));
	}
	return ((int)c);
}
