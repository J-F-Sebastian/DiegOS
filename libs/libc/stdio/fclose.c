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

int fclose (FILE *stream)
{
    unsigned i;
    int retval = 0;

    for (i = 0; (i < FOPEN_MAX) && (stream != iostreams[i]); i++) {};

    if (FOPEN_MAX == i) {
        return (EOF);
    } else if (stream == iostreams[i]) {
        iostreams[i] = NULL;
    }

    if (fflush(stream)) {
        retval = EOF;
    }
    if (close(fileno(stream))) {
        retval = EOF;
    }
    if (stream->buffer) {
        free(stream->buffer);
    }
    if ((stream != stdin) && (stream != stdout) && (stream != stderr)) {
        free(stream);
    }
    return (retval);
}