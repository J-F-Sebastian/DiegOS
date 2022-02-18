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

#include <string.h>

int strncmp(const char *cs, const char *ct, size_t n)
{
	if (cs && ct && (n > 0)) {
		do {
			if (*cs != *ct++)
				break;
			if (*cs++ == '\0')
				return (0);
		} while (--n > 0);
		if (n > 0) {
			if (*cs == '\0')
				return (-1);
			if (*--ct == '\0')
				return (1);
			return ((unsigned char)*cs - (unsigned char)*ct);
		}
	}
	return (0);
}
