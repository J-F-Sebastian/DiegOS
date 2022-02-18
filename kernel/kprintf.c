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
#include <limits.h>
#include <ctype.h>

#include "kputb.h"
#include "kprintf.h"

enum {
	LEFT = (1 << 0),
	RIGHT = (1 << 1),
	LONGLONG = (1 << 2),
	LONG = (1 << 3),
	INT = (1 << 4),
	SHORT = (1 << 5),
	CHAR = (1 << 6),
	PREPEND = (1 << 7),
	FORCE_SIGN = (1 << 8),
	FORCE_NOSIGN = (1 << 9),
	NO_ASSIGN = (1 << 10),
	LONGDOUBLE = (1 << 11),
	WIDTHSPEC = (1 << 12),
	FILLZERO = (1 << 13)
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

static inline void count_kputc(int c, int *count, char *buffer, unsigned *bcount)
{
	(*count)++;
	if (*bcount == VPRINTF_BUFSIZE) {
		kputb(buffer, VPRINTF_BUFSIZE);
		(*bcount) = 0;
	}
	buffer[(*bcount)++] = (char)c;
}

int kvprintf(const char *fmt, va_list ap)
{
	int c, charcount = 0;
	int fill;
	int width, max, len, base;
	static const char X2C_tab[] = "0123456789ABCDEF";
	static const char x2c_tab[] = "0123456789abcdef";
	const char *x2c;
	char *p;
	long long i;
	unsigned long long u;
	char temp[8 * sizeof(long long) / 3 + 2];
	unsigned bcount = 0;
	unsigned flags;
	unsigned loopend;
	char buffer[VPRINTF_BUFSIZE] = { 0 };

	while ((c = *fmt++) != 0) {
		if (c != '%') {
			/* Ordinary character. */
			count_kputc(c, &charcount, buffer, &bcount);
			continue;
		}

		/* Format specifier of the form:
		 *      %[flags][width][.precision][length]specifier
		 */
		c = *fmt++;

		/*
		 * Flags first. RIGHT justification is default, as well as padding with
		 * spaces.
		 */
		flags = RIGHT;
		fill = ' ';
		loopend = 1;
		do {
			switch (c) {
			case '-':
				flags = LEFT;
				c = *fmt++;
				break;

			case '+':
				flags |= FORCE_SIGN;
				flags &= ~FORCE_NOSIGN;
				c = *fmt++;
				break;

			case ' ':
				flags &= ~FORCE_SIGN;
				flags |= FORCE_NOSIGN;
				c = *fmt++;
				break;

			case '#':
				flags |= PREPEND;
				c = *fmt++;
				break;

			case '0':
				flags |= FILLZERO;
				fill = '0';
				c = *fmt++;
				break;

			default:
				loopend = 0;
				break;
			}

		} while (loopend);

		/*
		 * Width next, must be either a star or a number
		 */
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

		/*
		 * Then precision...
		 */
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

		/*
		 * And length ...
		 */

		/* Set a few flags to the default. */
		x2c = x2c_tab;
		i = 0;
		base = 10;
		flags |= INT;
		if (c == 'l') {
			/* "Long" key, e.g. %ld. */
			flags &= ~INT;
			flags |= LONG;
			c = *fmt++;
			if (c == 'l') {
				/* "Longlong" key, e.g. %lld. */
				flags &= ~LONG;
				flags |= LONGLONG;
				c = *fmt++;
			}
		} else if (c == 'h') {
			flags &= ~INT;
			flags |= SHORT;
			c = *fmt++;
			if (c == 'h') {
				flags &= ~SHORT;
				flags |= CHAR;
				c = *fmt++;
			}
		} else if (c == 'L') {
			/* "Long" key, e.g. %ld. */
			flags &= ~INT;
			flags |= LONG;
			c = *fmt++;
		}

		if (c == 0)
			break;

		/*
		 * Then finally the specifiers
		 */
		switch (c) {
			/* Decimal. */
		case 'd':
		case 'i':
			switch (flags & (CHAR | SHORT | INT | LONG | LONGLONG)) {
			case CHAR:
				/* FALLTHRU */
			case SHORT:
				/* FALLTHRU */
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
			/*
			 * Now adjust max
			 */
			if (max != INT_MAX) {
				if (flags & CHAR)
					max = (max > 3) ? 3 : max;
				if (flags & SHORT)
					max = (max > 5) ? 5 : max;
				if (flags & INT)
					max = (max > 10) ? 10 : max;
				if (flags & LONG)
					max = (max > 10) ? 10 : max;
				if (flags & LONGLONG)
					max = (max > (int)(sizeof(temp) - 1)) ?
					    (int)(sizeof(temp) - 1) : max;
			}
			u = i < 0 ? -i : i;
			goto int2ascii;

			/* Octal. */
		case 'o':
			base = 010;
			/*
			 * Now adjust max
			 */
			if (max != INT_MAX) {
				if (flags & CHAR)
					max = (max > 3) ? 3 : max;
				if (flags & SHORT)
					max = (max > 6) ? 6 : max;
				if (flags & INT)
					max = (max > 11) ? 11 : max;
				if (flags & LONG)
					max = (max > 11) ? 11 : max;
				if (flags & LONGLONG)
					max = (max > 22) ? 22 : max;
			}
			goto getint;

			/* Pointer, interpret as %X or %lX. */
		case 'p':
			flags |= PREPEND;
			if (sizeof(void *) > sizeof(int)) {
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
			/*
			 * Now adjust max
			 */
			if (max != INT_MAX) {
				if (flags & CHAR)
					max = (max > 2) ? 2 : max;
				if (flags & SHORT)
					max = (max > 4) ? 4 : max;
				if (flags & INT)
					max = (max > 8) ? 8 : max;
				if (flags & LONG)
					max = (max > 8) ? 8 : max;
				if (flags & LONGLONG)
					max = (max > 16) ? 16 : max;
			}
			/* FALLTHRU */

			/* Unsigned decimal. */
		case 'u':
 getint:
			switch (flags & (CHAR | SHORT | INT | LONG | LONGLONG)) {
			case CHAR:
				/* FALLTHRU */
			case SHORT:
				/* FALLTHRU */
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
				if (width && !(len < width))
					u = 0;
			} while ((u /= base) > 0);

			if (max != INT_MAX) {
				max = (max < width) ? max : width;
				while (len < max) {
					*--p = fill;
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
			if ((i < 0) || (flags & FORCE_SIGN)) {
				width--;
				if (i < 0) {
					count_kputc('-', &charcount, buffer, &bcount);
				} else {
					count_kputc('+', &charcount, buffer, &bcount);
				}
			}
			if (flags & RIGHT) {
				while (width > 0) {
					count_kputc(fill, &charcount, buffer, &bcount);
					width--;
				}
			}
			while (len > 0) {
				count_kputc(*p++, &charcount, buffer, &bcount);
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
