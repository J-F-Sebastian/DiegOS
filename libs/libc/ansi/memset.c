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
#include <stdint.h>

void *memset(void *s, int c, size_t n)
{
    char *s1 = s;
    int *s2 = s;

    if (s1 && (n > 0)) {
        /*
         * Byte aligned
         */
        if ((intptr_t)s & 3) {
            while (n--) {
                *s1++ = (char)c;
            }
        } else {
           /*
            * Word aligned
            */
            c &= 0xff;
            c |= (c << 8);
            c |= (c << 16);
            while (n > 4) {
                *s2++ = c;
                n -= 4;
            }
            s1 = (char *)s2;
            while (n > 0) {
                --n;
                *s1++ = (char)c;
            }
        }
    }
    return (s);
}
