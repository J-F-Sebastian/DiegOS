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

#include "loc_incl.h"

static unsigned char ch[FOPEN_MAX];

int readbuffer (FILE *stream)
{
    unsigned i;
    ssize_t retcode;

    stream->count = 0;

    if ((fileno(stream) < 0) ||
            stream_testflags(stream, IOBUF_EOF | IOBUF_ERROR)) {
            abort();
        return (EOF);
    }

    if (!stream_r(stream) || stream_wrting(stream)) {
        stream->flags |= IOBUF_ERROR;
        return (EOF);
    }

    if (!stream_rding(stream)) {
        stream->flags |= IOBUF_READING;
    }

    if (!stream_nbuf(stream) && !stream->buffer) {
        stream->buffer = (unsigned char *) malloc(BUFSIZ);
        if (!stream->buffer) {
            stream->flags |= IOBUF_NBUF;
        } else {
            stream->flags |= IOBUF_RELBUF;
            stream->bufsize = BUFSIZ;
        }
    }

    /* flush line-buffered output when filling an input buffer */
    for (i = 0; i < FOPEN_MAX; i++) {
        if (iostreams[i] && stream_testflags(iostreams[i], IOBUF_LBUF)) {
            if (stream_wrting(iostreams[i])) {
                (void) fflush(iostreams[i]);
            }
        }
    }

    if (stream_nbuf(stream)) {
        stream->buffer = &ch[fileno(stream)];
        stream->bufsize = 1;
    }
    stream->bufptr = stream->buffer;
    retcode = read(stream->fd, (char *)stream->buffer, stream->bufsize);

    if (retcode <= 0) {
        stream->count = 0;
        if (retcode < 0) {
            stream_setflags(stream, IOBUF_ERROR);
        } else {
            stream_setflags(stream, IOBUF_EOF);
        }
        return (EOF);
    }

    stream->count = (unsigned)retcode - 1;

    return (*stream->bufptr++);
}
