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

#ifndef _BITMAPS_H_
#define _BITMAPS_H_

#include <types_common.h>

/*
 * bits per long
 */
#define BINL (sizeof(long)*8)

/*
 * size of bitmap in longs, x is the number of positions required.
 */
#define BITMAPLEN(x) ((x+BINL-1)/BINL)

/*
 * size of bitmap in bytes, x is the number of positions required.
 */
#define BITMAPBYTES(x) (BITMAPLEN(x)*sizeof(long))

/*
 * Callback function, to be used with bitmap_for_each_set,
 * bitmap_for_each_clear, bitmap_if_any_set
 */
typedef void (*bitmapcb)(long *bitmap, unsigned bitpos, void *);

/*
 * For any function, bitpos is 0 BASED
 */
inline void bitmap_set(long *bitmap, unsigned bitpos)
{
	bitmap[bitpos / BINL] |= (long)1 << (bitpos % BINL);
}

inline void bitmap_clear(long *bitmap, unsigned bitpos)
{
	bitmap[bitpos / BINL] &= ~((long)1 << (bitpos % BINL));
}

inline BOOL bitmap_is_set(long *bitmap, unsigned bitpos)
{
	if (bitmap[bitpos / BINL] & ((long)1 << (bitpos % BINL))) {
		return (TRUE);
	}

	return (FALSE);
}

inline unsigned bitmap_first_is_set(long *bitmap, unsigned leninlongs)
{
	unsigned i = 0, j, k;

	while (i < leninlongs) {
		if (bitmap[i]) {
			for (j = 1, k = 0; j; j += j, k++) {
				if (bitmap[i] & j) {
					return (i * BINL + k);
				}
			}
		}
		++i;
	}

	return (leninlongs * sizeof(long) * 8);
}

inline unsigned bitmap_first_is_clear(long *bitmap, unsigned leninlongs)
{
	unsigned i = 0, j, k;

	while (i < leninlongs) {
		if (!bitmap[i]) {
			return (i * BINL);
		}
		for (j = 1, k = 0; j; j += j, k++) {
			if (!(bitmap[i] & j)) {
				return (i * BINL + k);
			}
		}
		++i;
	}

	return (leninlongs * BINL);
}

inline void bitmap_for_each_set(long *bitmap, unsigned leninlongs, bitmapcb cbfn, void *param)
{
	unsigned i = 0, j, k;

	while (i < leninlongs) {
		if (bitmap[i]) {
			for (j = 1, k = 0; j; j += j, k++) {
				if (bitmap[i] & j) {
					cbfn(bitmap, i * BINL + k, param);
				}
			}
		}
		++i;
	}
}

inline void bitmap_for_each_clear(long *bitmap, unsigned leninlongs, bitmapcb cbfn, void *param)
{
	unsigned i = 0, j, k;

	while (i < leninlongs) {
		if (-1 != bitmap[i]) {
			for (j = 1, k = 0; j; j += j, k++) {
				if (!(bitmap[i] & j)) {
					cbfn(bitmap, i * BINL + k, param);
				}
			}
			++i;
		}
	}
}

#endif
