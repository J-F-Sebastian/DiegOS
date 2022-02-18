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
#include <unistd.h>
#include <errno.h>

#include "loc_incl.h"

int readbuffer(FILE * stream)
{
	unsigned i;
	ssize_t retcode;
	char ch = '\0';

	if (fileno(stream) < 0) {
		errno = EBADF;
		return (EOF);
	}

	if (stream_testflags(stream, IOBUF_ERROR)) {
		errno = EIO;
		return (EOF);
	}

	if (stream_testflags(stream, IOBUF_EOF)) {
		errno = EOF;
		return (EOF);
	}

	if (!stream_r(stream) || stream_wrting(stream)) {
		stream_setflags(stream, IOBUF_ERROR);
		return (EOF);
	}

	stream_setflags(stream, IOBUF_READING);

	/*
	 * If this is the first time we call readbuffer, and the buffer
	 * is NULL, and buffering is in use, then try to allocate
	 * the buffer.
	 * If successful set IOBUF_RELBUF to release memory when closing the file.
	 */
	if (!stream_nbuf(stream) && !stream->buffer) {
		stream->buffer = (char *)malloc(BUFSIZ);
		if (!stream->buffer) {
			stream_setflags(stream, IOBUF_NBUF);
			stream->bufsize = 0;
		} else {
			stream_setflags(stream, IOBUF_RELBUF);
			stream->bufsize = BUFSIZ;
		}
	}

	/* flush line-buffered output when filling an input buffer */
	for (i = 0; i < FOPEN_MAX; i++) {
		if (iostreams[i] && stream_testflags(iostreams[i], IOBUF_LBUF)) {
			if (stream_wrting(iostreams[i])) {
				(void)fflush(iostreams[i]);
			}
		}
	}

	stream->count = 0;
	stream->bufptr = stream->buffer;

	if (stream_nbuf(stream)) {
		retcode = read(stream->fd, &ch, sizeof(ch));
	} else {
		retcode = read(stream->fd, stream->buffer, stream->bufsize);
	}

	if (retcode <= 0) {
		stream->validsize = 0;
		if (retcode < 0) {
			stream_setflags(stream, IOBUF_ERROR);
		} else {
			stream_setflags(stream, IOBUF_EOF);
		}
		return (EOF);
	}

	/*
	 * count is the total amount of bytes read minus the returned one,
	 * so if no buffer is allocated, count is always 0 and no read
	 * from memory can take place - all stdio functions are forced to
	 * call readbuffer one byte at a time.
	 */
	stream->count = stream->validsize = (unsigned)(retcode - 1);

	return (stream_nbuf(stream) ? (int)ch : (*stream->bufptr++));
}
