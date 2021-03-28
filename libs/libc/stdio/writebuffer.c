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

int writebuffer(int c, FILE *stream)
{
    unsigned count;

    if (fileno(stream) < 0) {
        stream_setflags(stream, IOBUF_ERROR);
        return (EOF);
    }

    if (!stream_w(stream) || (stream_rding(stream) && !feof(stream))) {
        return (EOF);
    }

    stream_clearflags(stream, IOBUF_READING);
    stream_setflags(stream, IOBUF_WRITING);

    if (!stream_testflags(stream, IOBUF_NBUF) && !stream->buffer) {
        if (!(stream->buffer = (char *) malloc(BUFSIZ))) {
            stream_setflags(stream, IOBUF_NBUF);
            stream->count = stream->bufsize = 0;
        } else {
            stream_setflags(stream, IOBUF_RELBUF);
            stream->count = stream->bufsize = BUFSIZ;
        }
        stream->bufptr = stream->buffer;
    }

    if (stream_testflags(stream, IOBUF_NBUF)) {
        char c1 = (char)c;

        if (stream_testflags(stream, IOBUF_APPEND)) {
            if (lseek(fileno(stream), 0L, SEEK_END) == -1) {
                stream_setflags(stream, IOBUF_ERROR);
                return (EOF);
            }
        }
        if (write(fileno(stream), &c1, 1) != 1) {
            stream_setflags(stream, IOBUF_ERROR);
            return (EOF);
        }
    } else if (stream_testflags(stream, IOBUF_LBUF)) {
        if (stream->count) {
            --stream->count;
            *stream->bufptr++ = (char)c;
        }

        if ((c == '\n') || !stream->count) {

            if (stream_testflags(stream, IOBUF_APPEND)) {
                if (lseek(fileno(stream), 0L, SEEK_END) == -1) {
                    stream_setflags(stream, IOBUF_ERROR);
                    return (EOF);
                }
            }
            count = stream->bufsize - stream->count;
            stream->bufptr = stream->buffer;
            stream->count = stream->bufsize;

            if (write(fileno(stream), stream->buffer, count) < 0) {
                stream_setflags(stream, IOBUF_ERROR);
                return (EOF);
            }
            /*
             * Only TTYs make use of LBUF !!!
             */
            if (c == '\n') {
                if (write(fileno(stream), "\r", 1) < 0) {
                    stream_setflags(stream, IOBUF_ERROR);
                    return (EOF);
                }
            }
        }
    } else {

        count = stream->bufsize - stream->count;
        stream->count = stream->bufsize - 1;
        stream->bufptr = stream->buffer;

        if (count > 0) {
            if (stream_testflags(stream, IOBUF_APPEND)) {
                if (lseek(fileno(stream), 0L, SEEK_END) == -1) {
                    stream_setflags(stream, IOBUF_ERROR);
                    return (EOF);
                }
            }
            if (write(fileno(stream), stream->buffer, count) < 0) {
                *stream->bufptr++ = (char) c;
                stream_setflags(stream, IOBUF_ERROR);
                return (EOF);
            }
        }
        *stream->bufptr++ = (char) c;
    }

    return (c);
}
