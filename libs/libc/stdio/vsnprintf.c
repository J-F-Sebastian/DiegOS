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
#include <stdarg.h>
#include <limits.h>
#include <errno.h>

#include "loc_incl.h"

int vsnprintf(char *s, size_t n, const char *format, va_list arg)
{
	int retval = 0;
	FILE tmpstream;

	if (!s || !format) {
		return (EINVAL);
	}

	if (n > 0) {
		stream_init(&tmpstream, -1, s, n, (IOBUF_WRITE | IOBUF_FBUF));
		retval = formatted_printf(&tmpstream, format, TRUE, arg);
		if (retval >= 0) {
			if (retval < (int)n) {
				s[retval] = '\0';
			} else {
				s[n - 1] = '\0';
			}
		}
	}

	return (retval);
}
