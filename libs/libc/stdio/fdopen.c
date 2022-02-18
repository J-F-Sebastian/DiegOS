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
#include <fcntl.h>
#include <errno.h>

#include "loc_incl.h"

#define PMODE       0666

FILE *fdopen(int fildes, const char *mode)
{
	int rwmode = 0;
	int fdflags;

	if ((fildes < 0) || (fildes >= FOPEN_MAX)) {
		errno = EBADF;
		return ((FILE *) NULL);
	}

	fdflags = fcntl(fildes, F_GETFL) & O_ACCMODE;

	switch (*mode++) {
	case 'r':
		rwmode = O_RDONLY;
		break;
	case 'w':
		rwmode = O_WRONLY;
		break;
	case 'a':
		rwmode = O_WRONLY;
		break;
	default:
		errno = EINVAL;
		return ((FILE *) NULL);
	}

	while (*mode) {
		switch (*mode++) {
		case 'b':
			continue;
		case '+':
			rwmode = O_RDWR;
			continue;
		default:
			break;
		}
		break;
	}

	if (rwmode != fdflags) {
		errno = EINVAL;
		return ((FILE *) NULL);
	}

	return (iostreams[fildes]);
}
