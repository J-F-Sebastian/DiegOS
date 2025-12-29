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

#include	<unistd.h>
#include	<stdio.h>

#include	"loc_incl.h"

int fflush(FILE *stream)
{
	unsigned i, count;
	int c1, retval = 0;
	off_t adjust = 0;

	if (!stream) {
		for (i = 0; i < FOPEN_MAX; i++)
			if (iostreams[i] && fflush(iostreams[i])) {
				retval = EOF;
			}
		return (retval);
	}

	/*
	 * Flushing makes sense if we buffer data.
	 */
	if (stream_nbuf(stream) || (!stream_dirty(stream))) {
		return (0);
	}

	if (stream_rding(stream)) {
		adjust = (off_t) (-stream->count);
		stream->count = 0;
		if (lseek(stream->fd, adjust, SEEK_CUR) == (off_t) - 1) {
			stream_setflags(stream, IOBUF_ERROR);
			return (EOF);
		}
		if (stream_w(stream)) {
			stream_clearflags(stream, IOBUF_READING | IOBUF_WRITING);
		}
		stream->bufptr = stream->buffer;
		return (0);
	}

	if (stream_r(stream)) {	/* "a" or "+" mode */
		stream_clearflags(stream, IOBUF_WRITING);
	}

	count = stream->validsize - stream->count;

	/*
	 * This can happen if: the buffer is in writing mode and no data
	 * has ever been written in the buffer OR the buffer is in reading mode.
	 */
	if (!count) {
		return (0);
	}

	/*
	 * When writing, the buffer size can be fully used for valid data.
	 * This is not true when reading - EOF may be encountered before
	 * having read bufsize bytes.
	 */
	stream->bufptr = stream->buffer;
	stream->count = stream->validsize = stream->bufsize;

	if (stream_testflags(stream, IOBUF_APPEND)) {
		if (lseek(stream->fd, 0L, SEEK_END) == (off_t) - 1) {
			stream_setflags(stream, IOBUF_ERROR);
			return (EOF);
		}
	}
	c1 = write(stream->fd, stream->buffer, count);

	if ((ssize_t) count == c1) {
		return (0);
	}

	stream_setflags(stream, IOBUF_ERROR);
	return (EOF);
}
