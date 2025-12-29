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

/*
 * loc_incl.h - local include file for stdio library
 */

#ifndef _LOC_INCL_H_
#define _LOC_INCL_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

enum {
	LEFT = (1 << 0),
	RIGHT = (1 << 1),
	LONGLONG = (1 << 2),
	LONG = (1 << 3),
	INT = (1 << 4),
	SHORT = (1 << 5),
	CHAR = (1 << 6),
	PREPEND = (1 << 7),
	FORCE_SIGN = (1 << 8),
	FORCE_NOSIGN = (1 << 9),
	NO_ASSIGN = (1 << 10),
	LONGDOUBLE = (1 << 11),
	WIDTHSPEC = (1 << 12),
	FILLZERO = (1 << 13)
};

enum {
	/*
	 * stream capabilities: can read, can write, can append
	 */
	IOBUF_READ = (1 << 0),
	IOBUF_WRITE = (1 << 1),
	IOBUF_APPEND = (1 << 2),
	/*
	 * stream actions: is reading, is writing
	 */
	IOBUF_READING = (1 << 4),
	IOBUF_WRITING = (1 << 5),
	/*
	 * stream conditions: buffer handling, is in error,
	 * has reached the end-of-file, is binary
	 */
	IOBUF_NBUF = _IONBF,
	IOBUF_LBUF = _IOLBF,
	IOBUF_FBUF = _IOFBF,
	IOBUF_ERROR = (1 << 16),
	IOBUF_EOF = (1 << 17),
	IOBUF_RELBUF = (1 << 18),
	IOBUF_BIN = (1 << 19)
};

struct iobuf {
	/* file descriptor */
	int fd;
	/* stream flags */
	unsigned flags;
	/* buffer size in bytes */
	unsigned bufsize;
	/* actual buffer size in use */
	unsigned validsize;
	/* buffer to be read or written */
	unsigned count;
	/* the stream buffer */
	char *buffer;
	/* pointer to the actual buffer position for I/O */
	char *bufptr;
};

extern FILE *iostreams[FOPEN_MAX];

inline int stream_r(FILE *stream)
{
	return ((stream->flags & IOBUF_READ) ? (1) : (0));
}

inline int stream_w(FILE *stream)
{
	return ((stream->flags & IOBUF_WRITE) ? (1) : (0));
}

inline int stream_rw(FILE *stream)
{
	return (((stream->flags & (IOBUF_READ | IOBUF_WRITE)) ==
		 (IOBUF_READ | IOBUF_WRITE)) ? (1) : (0));
}

inline int stream_dirty(FILE *stream)
{
	return ((stream->flags & (IOBUF_WRITING | IOBUF_READING)) ? (1) : (0));
}

inline int stream_wrting(FILE *stream)
{
	return ((stream->flags & IOBUF_WRITING) ? (1) : (0));
}

inline int stream_rding(FILE *stream)
{
	return ((stream->flags & IOBUF_READING) ? (1) : (0));
}

inline int stream_nbuf(FILE *stream)
{
	return ((stream->flags & IOBUF_NBUF) ? (1) : (0));
}

inline int stream_lbuf(FILE *stream)
{
	return ((stream->flags & IOBUF_LBUF) ? (1) : (0));
}

inline int stream_fbuf(FILE *stream)
{
	return ((stream->flags & IOBUF_FBUF) ? (1) : (0));
}

inline int stream_isb(FILE *stream)
{
	return ((stream->flags & IOBUF_BIN) ? (1) : (0));
}

inline int stream_testflags(FILE *stream, unsigned flags)
{
	return ((stream->flags & flags) ? (1) : (0));
}

inline void stream_setflags(FILE *stream, unsigned flags)
{
	stream->flags |= flags;
}

inline void stream_clearflags(FILE *stream, unsigned flags)
{
	stream->flags &= ~flags;
}

inline void stream_init(FILE *stream, int fd, char *buffer, unsigned bufsize, unsigned flags)
{
	if (stream) {
		stream->fd = fd;
		stream->flags = flags;
		stream->buffer = stream->bufptr = buffer;
		stream->bufsize = stream->count = bufsize;
	}
}

int formatted_printf(FILE * stream, const char *fmt, const int skipeof, va_list ap);
int formatted_scan(FILE * stream, const char *format, va_list ap);
char *i_compute(uint64_t val, int base, char *s, unsigned nrdigits);
char *_f_print(va_list * ap, int flags, char *s, char c, int precision);
int writebuffern(int c, FILE * stream);
int writebufferl(int c, FILE * stream);
int writebuffer(int c, FILE * stream);
int readbuffer(FILE * stream);

//FILE *popen(const char *command, const char *type);
//FILE *fdopen(int fd, const char *mode);

char *_ecvt(long double value, int ndigit, int *decpt, int *sign);
char *_fcvt(long double value, int ndigit, int *decpt, int *sign);

#endif
