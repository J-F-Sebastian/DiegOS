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

#include <stddef.h>
#include <ctype.h>
#include <limits.h>

#include "kputb.h"

#define VPRINTF_BUFSIZE (80)

static inline void count_kputc(int c, int *count, char *buffer, unsigned *bcount)
{
    (*count)++;
    if (*bcount == VPRINTF_BUFSIZE) {
        kputb(buffer, VPRINTF_BUFSIZE);
        (*bcount) = 0;
    }
    buffer[(*bcount)++] = (char)c;
}

int kvprintf(const char *fmt, va_list argp)
{
    int c, charcount = 0;
    enum { LEFT, RIGHT } adjust;
    enum { LONG, INT } intsize;
    int fill;
    int width, max, len, base;
    static char X2C_tab[]= "0123456789ABCDEF";
    static char x2c_tab[]= "0123456789abcdef";
    char *x2c;
    char *p;
    long i;
    unsigned long u;
    char temp[8 * sizeof(long) / 3 + 2];
    char buffer[VPRINTF_BUFSIZE] = {0};
    unsigned bcount = 0;

    while ((c= *fmt++) != 0) {
        if (c != '%') {
            /* Ordinary character. */
            count_kputc(c, &charcount, buffer, &bcount);
            continue;
        }

        /* Format specifier of the form:
         *	%[adjust][fill][width][.max]keys
         */
        c= *fmt++;

        adjust= RIGHT;
        if (c == '-') {
            adjust= LEFT;
            c= *fmt++;
        }

        fill= ' ';
        if (c == '0') {
            fill= '0';
            c= *fmt++;
        }

        width= 0;
        if (c == '*') {
            /* Width is specified as an argument, e.g. %*d. */
            width = va_arg(argp, int);
            c= *fmt++;
        } else if (isdigit(c)) {
            /* A number tells the width, e.g. %10d. */
            do {
                width= width * 10 + (c - '0');
            } while (isdigit(c= *fmt++));
        }

        max= INT_MAX;
        if (c == '.') {
            /* Max field length coming up. */
            if ((c= *fmt++) == '*') {
                max= va_arg(argp, int);
                c= *fmt++;
            } else if (isdigit(c)) {
                max= 0;
                do {
                    max= max * 10 + (c - '0');
                } while (isdigit(c= *fmt++));
            }
        }

        /* Set a few flags to the default. */
        x2c= x2c_tab;
        i= 0;
        base= 10;
        intsize= INT;
        if (c == 'l' || c == 'L') {
            /* "Long" key, e.g. %ld. */
            intsize= LONG;
            c= *fmt++;
        }
        if (c == 0) break;

        switch (c) {
            /* Decimal. */
        case 'd':
            i= (intsize == LONG) ? (va_arg(argp, long)) : (va_arg(argp, int));
            u= i < 0 ? -i : i;
            goto int2ascii;

            /* Octal. */
        case 'o':
            base= 010;
            goto getint;

            /* Pointer, interpret as %X or %lX. */
        case 'p':
            if (sizeof(char *) > sizeof(int)) intsize= LONG;

            /* Hexadecimal.  %X prints upper case A-F, not %lx. */
            /* FALLTHRU */
            /* no break */
        case 'X':
            x2c= X2C_tab;
            /* FALLTHRU */
            /* no break */
        case 'x':
            base= 0x10;
            goto getint;

            /* Unsigned decimal. */
        case 'u':
getint:
            u= (intsize == LONG) ? (va_arg(argp, unsigned long))
               : (va_arg(argp, unsigned int));
int2ascii:
            p= temp + sizeof(temp)-1;
            *p= 0;
            do {
                *--p= x2c[(ptrdiff_t) (u % base)];
            } while ((u /= base) > 0);
            goto string_length;

            /* A character. */
        case 'c':
            p= temp;
            *p= va_arg(argp, int);
            len= 1;
            goto string_print;

            /* Simply a percent. */
        case '%':
            p= temp;
            *p= '%';
            len= 1;
            goto string_print;

            /* A string.  The other cases will join in here. */
        case 's':
            p= va_arg(argp, char *);
            if (!p) p = "(null)";

string_length:
            for (len= 0; p[len] != 0 && len < max; len++) {}

string_print:
            width -= len;
            if (i < 0) width--;
            if (fill == '0' && i < 0) count_kputc('-', &charcount, buffer, &bcount);
            if (adjust == RIGHT) {
                while (width > 0) {
                    count_kputc(fill, &charcount, buffer, &bcount);
                    width--;
                }
            }
            if (fill == ' ' && i < 0) count_kputc('-', &charcount, buffer, &bcount);
            while (len > 0) {
                count_kputc((unsigned char) *p++, &charcount, buffer, &bcount);
                len--;
            }
            while (width > 0) {
                count_kputc(fill, &charcount, buffer, &bcount);
                width--;
            }
            break;

            /* Unrecognized format key, echo it back. */
        default:
            count_kputc('%', &charcount, buffer, &bcount);
            count_kputc(c, &charcount, buffer, &bcount);
        }
    }

    kputb(buffer, bcount);
    return (charcount);
}
