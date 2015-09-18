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

#include	<stdio.h>
#include	<stdlib.h>
#include	"loc_incl.h"

int setvbuf (FILE *stream, char *buf, int mode, size_t size)
{
    int retval = 0;

    if ((mode != _IOFBF) && (mode != _IOLBF) && (mode != _IONBF)) {
        return EOF;
    }

    if (stream->buffer && stream_testflags(stream, IOBUF_RELBUF)) {
        free(stream->buffer);
    }

    stream_setflags(stream, ~(IOBUF_NBUF | IOBUF_LBUF));

    if (buf && size <= 0) {
        retval = EOF;
    }
    if (!buf && (mode != _IONBF)) {
        if ((size <= 0) || (buf = (char *) malloc(size)) == NULL) {
            retval = EOF;
        }
        stream_setflags(stream, IOBUF_RELBUF);
    }

    stream->buffer = (unsigned char *) buf;

    stream->count = 0;
    stream->flags |= mode;
    stream->bufptr = stream->buffer;
    stream->bufsize = size;

    return (retval);
}
