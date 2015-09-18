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
#include "loc_incl.h"

static FILE locstdin = {0, IOBUF_READ | IOBUF_LBUF, 0, 0, 0, 0};
static FILE locstdout = {1, IOBUF_WRITE | IOBUF_LBUF, 0, 0, 0, 0};
static FILE locstderr = {2, IOBUF_WRITE | IOBUF_NBUF, 0, 0, 0, 0};

FILE *stdin = &locstdin;
FILE *stdout = &locstdout;
FILE *stderr = &locstderr;

FILE *iostreams[FOPEN_MAX] = {
    &locstdin,
    &locstdout,
    &locstderr,
    NULL
};
