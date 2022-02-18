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
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "loc_incl.h"

#define	PMODE		0666

FILE *fopen(const char *filename, const char *mode)
{
	unsigned i;
	unsigned flags;
	int rwmode, rwflags;
	FILE *stream = NULL;
	int fd;

	for (i = 0; (i < FOPEN_MAX) && iostreams[i]; i++) {
	};

	if (i == FOPEN_MAX) {
		errno = EBADF;
		return (stream);
	}

	switch (*mode++) {
	case 'r':
		flags = IOBUF_READ;
		rwmode = O_RDONLY;
		rwflags = 0;
		break;
	case 'w':
		flags = IOBUF_WRITE;
		rwmode = O_WRONLY;
		rwflags = (O_CREAT | O_TRUNC);
		break;
	case 'a':
		flags = (IOBUF_WRITE | IOBUF_APPEND);
		rwmode = O_WRONLY;
		rwflags = (O_APPEND | O_CREAT);
		break;
	default:
		errno = EINVAL;
		return (stream);
	}

	while (*mode) {
		switch (*mode++) {
		case 'b':
			flags |= IOBUF_BIN;
			break;
		case '+':
			flags |= (IOBUF_READ | IOBUF_WRITE);
			rwmode = O_RDWR;
			break;
		default:
			errno = EINVAL;
			return (stream);
		}
	}

	/* Perform a creat() when the file should be truncated or when
	 * the file is opened for writing and the open() failed.
	 */
	if (rwflags & O_TRUNC) {
		fd = creat(filename, PMODE);
		if ((fd > 0) && (flags & IOBUF_READ)) {
			if (close(fd)) {
				fd = -1;
			} else {
				fd = open(filename, rwmode, 0);
			}
		}
	} else {
		fd = open(filename, rwmode, 0);
		if ((fd < 0) && (rwflags & O_CREAT)) {
			fd = open(filename, rwmode, 0);
			if (fd < 0) {
				fd = creat(filename, PMODE);
				if ((fd > 0) && (flags & IOBUF_READ)) {
					if (close(fd)) {
						fd = -1;
					} else {
						fd = open(filename, rwmode, 0);
					}
				}
			}
		}
	}

	if (fd < 0)
		return ((FILE *) NULL);

	stream = (FILE *) malloc(sizeof(FILE));
	if (!stream) {
		(void)close(fd);
		return ((FILE *) NULL);
	}

	if ((flags & (IOBUF_READ | IOBUF_WRITE)) == (IOBUF_READ | IOBUF_WRITE)) {
		flags &= ~(IOBUF_READING | IOBUF_WRITING);
	}

	/*
	 * NO BUFFERING BY DEFAULTS RIGHT NOW !!!
	 */
	stream_init(stream, fd, NULL, 0, flags);
	iostreams[i] = stream;

	return (stream);
}
