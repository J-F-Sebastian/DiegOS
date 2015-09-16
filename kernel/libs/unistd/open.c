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

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "fdescr_private.h"

#define DEV_PATH "/dev"

int open (const char *filename, int flags, int perms)
{
    unsigned fd, flg;

    /*
     * Only devices right now!
     */
    if (strncmp(filename, DEV_PATH, sizeof(DEV_PATH)-1)) {
        return (-1);
    }

    /*
     * To be added: exclusive access, multiple same-name opens
     */

    for (fd = 0;
            (fd < NELEMENTS(fdarray)) && (fdarray[fd].flags & FD_DATA_IS_INUSE);
            fd++) {};

    if (NELEMENTS(fdarray) == fd) {
        return (-1);
    }

    switch (flags & 0xFF) {
    case O_WRONLY:
        flg = FD_DATA_IS_W;
        break;
    case O_RDONLY:
        flg = FD_DATA_IS_R;
        break;
    case O_RDWR:
        flg = (FD_DATA_IS_W | FD_DATA_IS_R);
        break;
    default:
        return (-1);
    }

    fdarray[fd].absfname = malloc(strlen(filename));
    if (!fdarray[fd].absfname) {
        return (-1);
    }

    fdarray[fd].rawdev = device_lookup_name(filename+sizeof(DEV_PATH));

    if (!fdarray[fd].rawdev) {
        free(fdarray[fd].absfname);
        fdarray[fd].absfname = NULL;
        return (-1);
    }

    flg |= FD_DATA_IS_RAW;

    strcpy(fdarray[fd].absfname, filename);
    fdarray[fd].flags = flg;
    fdarray[fd].flags |= FD_DATA_IS_INUSE;

    return (fd);
}
