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
#include <unistd.h>

#include "loc_incl.h"

int fseek(FILE *stream, long offset, int origin)
{
	int adjust = 0;
	long pos;

	/* Clear both the end of file and error flags */
	clearerr(stream);

	if (stream_rding(stream)) {
		if ((origin == SEEK_CUR) && (stream->buffer) && !stream_nbuf(stream)) {
			adjust = stream->count;
		}
		stream->count = 0;
	} else if (stream_wrting(stream)) {
		fflush(stream);
	}

	pos = lseek(fileno(stream), (off_t) (offset - adjust), origin);

	if (stream_rw(stream)) {
		stream_clearflags(stream, IOBUF_READING | IOBUF_WRITING);
	}

	stream->bufptr = stream->buffer;

	return ((pos == -1) ? (-1) : (0));
}
