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

#ifndef _FDESCR_PRIVATE_H_
#define _FDESCR_PRIVATE_H_

#include <diegos/devices.h>
#include <limits.h>

#define FD_MAX (OPEN_MAX)

enum {
    FD_DATA_IS_RAW      = (1 << 0),
    FD_DATA_IS_FILE     = (1 << 1),
    FD_DATA_IS_SOCK     = (1 << 2),
    FD_DATA_IS_INUSE    = (1 << 3),
    FD_DATA_IS_R        = (1 << 4),
    FD_DATA_IS_W        = (1 << 5)
};

typedef struct fd_data {
    /*
     * File descriptor flags, see enum
     */
    unsigned flags;
    /*
     * Names are malloc'd on purpose, limits in unistd.h
     */
    char *absfname;
    /*
     * If a file is opened/redirected into a raw device,
     * this pointer must not be null
     */
    device_t *rawdev;
} fd_data_t;

/*
 * Array for direct access to file descriptors
 */
extern fd_data_t fdarray[FD_MAX];

/*
 * Extern inline function to check boundaries before accessing fdarray.
 * DO NOT access fdarray directly, cal fdget instead.
 */
extern inline fd_data_t *fdget (int fd)
{
    return (((fd >=0) && (fd < (int)NELEMENTS(fdarray))) ? (fdarray+fd) : (NULL));
}

#endif
