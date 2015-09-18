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

#include "loc_incl.h"

char *i_compute (uint64_t val, int base, char *s, unsigned nrdigits)
{
    int c;

    c= (int)(val % base);
    val /= base;
    if (val || (nrdigits > 1)) {
        s = i_compute(val, base, s, nrdigits - 1);
    }
    *s++ = (c > 9) ? ('a' + c - 10) : ('0' + c);
    return (s);
}
