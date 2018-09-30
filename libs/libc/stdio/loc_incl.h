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

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

enum {
    /*
     * stream capabilities: can read, can write, can append
     */
    IOBUF_READ      = (1 << 0),
    IOBUF_WRITE     = (1 << 1),
    IOBUF_APPEND    = (1 << 2),
    /*
     * stream actions: is reading, is writing
     */
    IOBUF_READING   = (1 << 4),
    IOBUF_WRITING   = (1 << 5),
    /*
     * stream conditions: buffer handling, is in error,
     * has reached the end-of-file
     */
    IOBUF_NBUF      = _IONBF,
    IOBUF_LBUF      = _IOLBF,
    IOBUF_FBUF      = _IOFBF,
    IOBUF_ERROR     = (1 << 16),
    IOBUF_EOF       = (1 << 17),
    IOBUF_RELBUF    = (1 << 18)
};

struct iobuf {
    /* file descriptor */
    int             fd;
    /* stream flags */
    unsigned        flags;
    /* buffer size in bytes */
    unsigned        bufsize;
    /* buffer to be read or written */
    unsigned        count;
    /* the stream buffer */
    unsigned char  *buffer;
    /* pointer to the actual buffer position for I/O */
    unsigned char  *bufptr;
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

inline int stream_busy(FILE *stream)
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

inline void stream_init (FILE *stream,
                                int fd,
                                unsigned char *buffer,
                                unsigned bufsize,
                                unsigned flags)
{
    if (stream) {
        stream->fd = fd;
        stream->flags = flags;
        stream->buffer = stream->bufptr = buffer;
        stream->bufsize = stream->count = bufsize;
    }
}

typedef long long printval_s_t;

int formatted_printf (FILE *stream, const char *fmt, const int skipeof, va_list ap);
int formatted_scan (FILE * stream, const char *format, va_list ap);
char *i_compute (uint64_t val, int base, char *s, unsigned nrdigits);
char *_f_print(va_list *ap, int flags, char *s, char c, int precision);
int writebuffer (int c, FILE *stream);
int readbuffer (FILE *stream);


//FILE *popen(const char *command, const char *type);
//FILE *fdopen(int fd, const char *mode);

#ifndef	NOFLOAT
char *_ecvt(long double value, int ndigit, int *decpt, int *sign);
char *_fcvt(long double value, int ndigit, int *decpt, int *sign);
#endif	/* NOFLOAT */

#define	FL_LJUST	0x0001		/* left-justify field */
#define	FL_SIGN		0x0002		/* sign in signed conversions */
#define	FL_SPACE	0x0004		/* space in signed conversions */
#define	FL_ALT		0x0008		/* alternate form */
#define	FL_ZEROFILL	0x0010		/* fill with zero's */
#define	FL_SHORT	0x0020		/* optional h */
#define	FL_LONG		0x0040		/* optional l */
#define	FL_LONGDOUBLE	0x0080		/* optional L */
#define	FL_WIDTHSPEC	0x0100		/* field width is specified */
#define	FL_PRECSPEC	0x0200		/* precision is specified */
#define FL_SIGNEDCONV	0x0400		/* may contain a sign */
#define	FL_NOASSIGN	0x0800		/* do not assign (in scanf) */
#define	FL_NOMORE	0x1000		/* all flags collected */
#define	FL_LONGLONG	0x2000		/* 64-bit for ints */
