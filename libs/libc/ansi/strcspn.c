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

size_t strcspn(const char *cs, const char *ct)
{
	const char *s1, *s2;

	if (cs && ct) {
		for (s1 = cs; *s1; s1++) {
			for (s2 = ct; (*s2 != *s1) && *s2; s2++) {
			};
			if (*s2) {
				break;
			}
		}
		return ((size_t) (s1 - cs));
	} else {
		return (0);
	}
}
