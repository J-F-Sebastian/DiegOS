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

#include <unistd.h>
#include <errno.h>
#include "fdescr_private.h"

ssize_t read (int fd, void *buf, size_t n)
{
    unsigned retries = 10;
    int retcode;

    if ((fd < 0) || (fd >= (int)NELEMENTS(fdarray))) {
        errno = EBADF;
	return (-1);
    }

    if (!buf || !(fdarray[fd].flags & FD_DATA_IS_INUSE)) {
        errno = EINVAL;
        return (-1);
    }

    if (fdarray[fd].flags & FD_DATA_IS_RAW) {
        do {
            retcode = device_io_rx(fdarray[fd].rawdev, (char *)buf, n);
        } while (retries-- && (retcode == EAGAIN));
        if (retcode < EOK) {
	    errno = EIO;
            return (-1);
        }
    } else {
	errno = EINVAL;
        return (-1);
    }

    return ((ssize_t)retcode);
}
