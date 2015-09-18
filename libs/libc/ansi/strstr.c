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

#include <string.h>
#include <stddef.h>

char *strstr(const char *cs, const char *ct)
{
    const size_t len = strlen(ct);

    if (!len || !cs) {
        return ((char *)cs);
    }

    while ((*cs != *ct) || (strncmp(cs, ct, len))) {
        if (*cs++ == '\0') {
            return ((char *)NULL);
        }
    }

    return ((char *)cs);
}
