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
#include "loc_incl.h"

#define	PMODE		0666

FILE *freopen(const char *filename, const char *mode, FILE * stream)
{
	unsigned i;
	unsigned flags = stream->flags & (_IONBF | _IOFBF | _IOLBF);
	int rwmode = 0, rwflags = 0;
	int fd;

	(void)fflush(stream);	/* ignore errors */
	(void)close(fileno(stream));

	switch (*mode++) {
	case 'r':
		flags |= (IOBUF_READ);
		rwmode = O_RDONLY;
		break;
	case 'w':
		flags |= (IOBUF_WRITE);
		rwmode = O_WRONLY;
		rwflags = (O_CREAT | O_TRUNC);
		break;
	case 'a':
		flags |= (IOBUF_WRITE | IOBUF_APPEND);
		rwmode = O_WRONLY;
		rwflags |= (O_APPEND | O_CREAT);
		break;
	default:
		return ((FILE *) NULL);
	}

	while (*mode) {
		switch (*mode++) {
		case 'b':
			continue;
		case '+':
			rwmode = O_RDWR;
			flags |= (IOBUF_READ | IOBUF_WRITE);
			continue;
		default:
			break;
		}
		break;
	}

	if ((rwflags & O_TRUNC)
	    || (((fd = open(filename, rwmode, 0)) < 0)
		&& (rwflags & O_CREAT))) {
		if (((fd = creat(filename, PMODE)) < 0) && (flags | IOBUF_READ)) {
			(void)close(fd);
			fd = open(filename, rwmode, 0);
		}
	}

	if (fd < 0) {
		for (i = 0; i < FOPEN_MAX; i++) {
			if (stream == iostreams[i]) {
				iostreams[i] = NULL;
				break;
			}
		}
		if (stream != stdin && stream != stdout && stream != stderr)
			free((void *)stream);
		return ((FILE *) NULL);
	}

	stream->count = 0;
	stream->fd = fd;
	stream->flags = flags;
	return (stream);
}
