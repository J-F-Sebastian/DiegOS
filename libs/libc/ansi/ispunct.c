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

#include <ctype.h>

int ispunct (int c)
{
    if ((c >= 33) && (c <= 47)) {
        return (1);
    } else if ((c >= 58) && (c <= 64)) {
        return (1);
    } else if ((c >= 91) && (c <= 96)) {
        return (1);
    } else if ((c >= 123) && (c <= 126)) {
        return (1);
    }
    return (0);
}
