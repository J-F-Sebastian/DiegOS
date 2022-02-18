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
#include <stdlib.h>

void *bsearch(const void *key,
	      const void *base,
	      size_t n, size_t size, int (*cmp) (const void *keyval, const void *datum))
{
	const void *mid_point;
	int retval;

	while (n > 0) {
		mid_point = (char *)base + size * (n >> 1);
		if ((retval = cmp(key, mid_point)) == 0)
			return ((void *)mid_point);
		if (retval >= 0) {
			base = (char *)mid_point + size;
			n = (n - 1) >> 1;
		} else
			n >>= 1;
	}
	return (NULL);
}
