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

#include	<ctype.h>
#include	<errno.h>
#include	<limits.h>
#include	<stdlib.h>

#define between(a, c, z)  ((unsigned) ((c) - (a)) <= (unsigned) ((z) - (a)))

static unsigned long
string2long(const char *s, char **endp, int base, int is_signed)
{
    unsigned int v;
    unsigned long val = 0;
    int c;
    int ovfl = 0, sign = 1;
    const char *startnptr = s, *nrstart;

    if (endp) {
        *endp = (char *)s;
    }

    while (isspace(*s)) s++;
    c = *s;

    if (c == '-' || c == '+') {
        if (c == '-') sign = -1;
        s++;
    }
    nrstart = s;			/* start of the number */

    /* When base is 0, the syntax determines the actual base */
    if (base == 0) {
        if (*s == '0') {
            if (*++s == 'x' || *s == 'X') {
                base = 16;
                s++;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
    } else if ((base == 16) && (*s == '0') && (*(s+1) =='x' || *(s+1) =='X')) {
        s+=2;
    }

    for (;;) {
        c = *s;
        if (between('0', c, '9')) {
            v = c - '0';
        } else if (between('a', c, 'z')) {
            v = c - 'a' + 0xa;
        } else if (between('A', c, 'Z')) {
            v = c - 'A' + 0xA;
        } else {
            break;
        }
        if ((int)v >= base) break;
        if (val > (ULONG_MAX - v) / base) {
            ovfl++;
        }
        val = (val * base) + v;
        s++;
    }
    if (endp) {
        if (nrstart == s) *endp = (char *)startnptr;
        else *endp = (char *)s;
    }

    if (!ovfl) {
        /* Overflow is only possible when converting a signed long. */
        if (is_signed &&
                ((sign < 0 && val > -(unsigned long)LONG_MIN) ||
                 (sign > 0 && val > LONG_MAX))) {
            ovfl++;
        }
    }

    if (ovfl) {
        errno = ERANGE;
        if (is_signed) {
            if (sign < 0) {
                return (LONG_MIN);
            } else {
                return (LONG_MAX);
            }
        } else {
            return (ULONG_MAX);
        }
    }
    return ((long) sign * val);
}


long strtol(const char *s, char **endp, int base)
{
    return ((signed long)string2long(s, endp, base, 1));
}

unsigned long int strtoul(const char *s, char **endp, int base)
{
    return (string2long(s, endp, base, 0));
}


