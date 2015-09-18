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

#include <stdlib.h>
#include <limits.h>

div_t div(int num, int denom)
{
    div_t r;

    if (num == 0) {
        r.quot = num / denom; /* might trap if denom == 0 */
        r.rem = num % denom;
    } else if ((num < 0) != (denom < 0)) {
        r.quot = (num / denom) + 1;
        r.rem = num - (num / denom + 1) * denom;
    } else {
        r.quot = num / denom;
        r.rem = num % denom;
    }

    return (r);
}
