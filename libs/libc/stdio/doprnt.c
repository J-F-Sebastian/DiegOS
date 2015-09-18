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

#define NOFLOAT

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "loc_incl.h"

static const char X2C_tab[] = "0123456789ABCDEF";
static const char x2c_tab[] = "0123456789abcdef";

enum {
    LEFT = 1,
    RIGHT = 2,
    LONGLONG = 4,
    LONG = 8,
    INT = 16,
    PREPEND = 32
};

#define TEMP_SIZE (8 * sizeof(long long) / 3 + 2)

static inline int fputc_internal(int c,
                                 int skipeof,
                                 FILE *stream,
                                 unsigned *bcount)
{
    if (EOF == fputc(c, stream)) {
        if (!skipeof) {
            return (-1);
        }
    }
    ++(*bcount);
    return (0);
}

int formatted_printf(FILE *stream, const char *fmt, const int skipeof, va_list ap)
{
    int c;
    int fill;
    int width, max, len, base;
    const char *x2c;
    char *p;
    long long i;
    unsigned long long u;
    char temp[TEMP_SIZE];
    unsigned bcount = 0;
    unsigned flags;

    while ((c = *fmt++) != 0) {
        if (c != '%') {
            /* Ordinary character. */
            if (fputc_internal(c, skipeof, stream, &bcount) < 0)
                return (-1);
            continue;
        }

        /* Format specifier of the form:
         *	%[adjust][fill][width][.max]keys
         */
        c = *fmt++;

        flags = RIGHT;
        if (c == '-') {
            flags = LEFT;
            c = *fmt++;
        }
        if (c == '#') {
            flags |= PREPEND;
            c = *fmt++;
        }

        fill = ' ';
        if (c == '0') {
            fill = '0';
            c = *fmt++;
        }

        width = 0;
        if (c == '*') {
            /* Width is specified as an argument, e.g. %*d. */
            width = va_arg(ap, int);
            c = *fmt++;
        } else if (isdigit(c)) {
            /* A number tells the width, e.g. %10d. */
            do {
                width = width * 10 + (c - '0');
            } while (isdigit(c = *fmt++));
        }

        max = INT_MAX;
        if (c == '.') {
            /* Max field length coming up. */
            if ((c = *fmt++) == '*') {
                max = va_arg(ap, int);
                c = *fmt++;
            } else if (isdigit(c)) {
                max = 0;
                do {
                    max = max * 10 + (c - '0');
                } while (isdigit(c = *fmt++));
            }
        }

        /* Set a few flags to the default. */
        x2c = x2c_tab;
        i = 0;
        base = 10;
        flags |= INT;
        if (c == 'l' || c == 'L') {
            /* "Long" key, e.g. %ld. */
            flags &= ~INT;
            flags |= LONG;
            c = *fmt++;
            if (c == 'l' || c == 'L') {
                /* "Longlong" key, e.g. %lld. */
                flags &= ~LONG;
                flags |= LONGLONG;
                c = *fmt++;
            }
        }
        if (c == 0)
            break;

        switch (c) {
        /* Decimal. */
        case 'd':
            switch (flags & (INT | LONG | LONGLONG)) {
            case INT:
                i = va_arg(ap, int);
                break;
            case LONG:
                i = va_arg(ap, long);
                break;
            case LONGLONG:
                i = va_arg(ap, long long);
                break;
            default:
                i = va_arg(ap, int);
                break;
            }
            u = i < 0 ? -i : i;
            goto int2ascii;

            /* Octal. */
        case 'o':
            base = 010;
            goto getint;

            /* Pointer, interpret as %X or %lX. */
        case 'p':
            if (sizeof(char *) > sizeof(int)) {
                flags &= ~INT;
                flags |= LONG;
            }
            /* Hexadecimal.  %X prints upper case A-F, not %lx. */
            /* FALLTHRU */
            /* no break */
        case 'X':
            x2c = X2C_tab;
            /* FALLTHRU */
            /* no break */
        case 'x':
            base = 0x10;
            goto getint;

            /* Unsigned decimal. */
        case 'u':
            getint:
            switch (flags & (INT | LONG | LONGLONG)) {
            case INT:
                u = va_arg(ap, unsigned int);
                break;
            case LONG:
                u = va_arg(ap, unsigned long);
                break;
            case LONGLONG:
                u = va_arg(ap, unsigned long long);
                break;
            default:
                u = va_arg(ap, unsigned int);
                break;
            }

            int2ascii:
            p = temp + sizeof(temp) - 1;
            len = *p = 0;
            do {
                *--p = x2c[(u % base)];
                ++len;
            } while ((u /= base) > 0);
            if (flags & PREPEND) {
                switch (base) {
                case 010:
                    *--p = '0';
                    ++len;
                    break;
                case 0x10:
                    *--p = 'x';
                    *--p = '0';
                    len += 2;
                    break;
                }
            }
            goto string_print;

            /* A character. */
        case 'c':
            p = temp;
            *p = va_arg(ap, int);
            len = 1;
            goto string_print;

            /* Simply a percent. */
        case '%':
            p = temp;
            *p = '%';
            len = 1;
            goto string_print;

            /* A string.  The other cases will join in here. */
        case 's':
            p = va_arg(ap, char *);
            if (!p)
                p = "(null)";
            for (len = 0; p[len] != 0 && len < max; len++) {
            }

            string_print:
            width -= len;
            if (i < 0)
                width--;
            if (fill == '0' && i < 0) {
                if (fputc_internal('-', skipeof, stream, &bcount) < 0)
                    return (-1);
            }
            if (flags & RIGHT) {
                while (width > 0) {
                    if (fputc_internal(fill, skipeof, stream, &bcount) < 0)
                        return (-1);
                    width--;
                }
            }
            if (fill == ' ' && i < 0) {
                if (fputc_internal('-', skipeof, stream, &bcount) < 0)
                    return (-1);
            }
            while (len > 0) {
                if (fputc_internal(*p++, skipeof, stream, &bcount) < 0)
                    return (-1);
                len--;
            }
            while (width > 0) {
                if (fputc_internal(fill, skipeof, stream, &bcount) < 0)
                    return (-1);
                width--;
            }
            break;

            /* Unrecognized format key, echo it back. */
        default:
            if (fputc_internal('%', skipeof, stream, &bcount) < 0)
                return (-1);
            if (fputc_internal(c, skipeof, stream, &bcount) < 0)
                return (-1);
        }
    }
    return ((int) bcount);
}
