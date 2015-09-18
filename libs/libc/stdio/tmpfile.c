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
#include <string.h>
#include <diegos/kernel.h>
#include "loc_incl.h"

#define TMPNAME "/tmp/tmp."

static char name_buffer[TMP_NAMELEN];
static unsigned long fcount = 0;
static unsigned long count = 0;

FILE *tmpfile (void)
{
    FILE *file;
    char *name;

    strncpy(name_buffer, TMPNAME, sizeof(name_buffer));
    name = name_buffer + strlen(name_buffer);
    snprintf(name,
             sizeof(name_buffer) - strlen(name_buffer),
             "%d_pid_%.3u",
             fcount++,
             my_thread_id());

    file = fopen(name_buffer,"wb+");
    if (!file) return ((FILE *)NULL);
    (void) remove(name_buffer);
    return (file);
}

char *tmpnam(char *s)
{
    char *name;

    if (++count > TMP_MAX) count = 1;	/* wrap-around */
    strncpy(name_buffer, TMPNAME, sizeof(name_buffer));
    name = name_buffer + strlen(name_buffer);
    snprintf(name,
             sizeof(name_buffer) - strlen(name_buffer),
             "%d_pid_%.3u",
             count,
             my_thread_id());

    return ((s) ? (strcpy(s, name_buffer)) : (name_buffer));
}
