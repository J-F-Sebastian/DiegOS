#include <stddef.h>
#include <limits.h>
#include <ctype.h>

#include "kputb.h"
#include "kprintf.h"

enum {
    LEFT = 1,
    RIGHT = 2,
    LONGLONG = 4,
    LONG = 8,
    INT = 16,
    PREPEND = 32
};

int kprintf(const char *fmt, ...)
{
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = kvprintf("[KRN]: ", ap);
    n = kvprintf(fmt, ap);
    va_end(ap);

    return (n);
}

int kmsgprintf(const char *fmt, ...)
{
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = kvprintf("[INF]: ", ap);
    n = kvprintf(fmt, ap);
    va_end(ap);

    return (n);
}

int kerrprintf(const char *fmt, ...)
{
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = kvprintf("[ERR]: ", ap);
    n = kvprintf(fmt, ap);
    va_end(ap);

    return (n);
}

#define VPRINTF_BUFSIZE (80)

static inline void count_kputc(int c,
                               int *count,
                               char *buffer,
                               unsigned *bcount)
{
    (*count)++;
    if (*bcount == VPRINTF_BUFSIZE) {
        kputb(buffer, VPRINTF_BUFSIZE);
        (*bcount) = 0;
    }
    buffer[(*bcount)++] = (char) c;
}

int kvprintf(const char *fmt, va_list ap)
{
    int c, charcount = 0;
    unsigned flags;
    int fill;
    int width, max, len, base;
    static const char X2C_tab[] = "0123456789ABCDEF";
    static const char x2c_tab[] = "0123456789abcdef";
    const char *x2c;
    char *p;
    long long i;
    unsigned long long u;
    char temp[8 * sizeof(long long) / 3 + 2];
    char buffer[VPRINTF_BUFSIZE] = { 0 };
    unsigned bcount = 0;

    while ((c = *fmt++) != 0) {
        if (c != '%') {
            /* Ordinary character. */
            count_kputc(c, &charcount, buffer, &bcount);
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
            if (max != INT_MAX) {
                switch (flags & (INT | LONG | LONGLONG)) {
                case INT:
                    case LONG:
                    max = (max > 11) ? 11 : max;
                    break;
                case LONGLONG:
                    max = (max > 22) ? 22 : max;
                    break;
                }
            }
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
            if (max != INT_MAX) {
                switch (flags & (INT | LONG | LONGLONG)) {
                case INT:
                    case LONG:
                    max = (max > 8) ? 8 : max;
                    break;
                case LONGLONG:
                    max = (max > 16) ? 16 : max;
                    break;
                }
            }
            /* Unsigned decimal. */

            /* FALLTHRU */
            /* no break */
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
            /*
             * Fill in the number from RIGHT to LEFT, null-terminated
             * i.e. start with null, then numbers from LSB to MSB.
             */
            do {
                *--p = x2c[(u % base)];
                ++len;
            } while ((u /= base) > 0);
            /*
             * Fill in zeroes if the max field is bigger than the actual
             * precision
             */
            if (max != INT_MAX) {
                while ((p != temp) && (len < max)) {
                    *--p = x2c[0];
                    ++len;
                }
            }
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
            if (fill == '0' && i < 0)
                count_kputc('-', &charcount, buffer, &bcount);
            if (flags & RIGHT) {
                while (width > 0) {
                    count_kputc(fill, &charcount, buffer, &bcount);
                    width--;
                }
            }
            if (fill == ' ' && i < 0)
                count_kputc('-', &charcount, buffer, &bcount);
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
