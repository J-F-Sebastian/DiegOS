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
#include <stddef.h>

char *strtok_r(char *s, const char *ct, char **saveptr)
{
	const char *s1;
	char *s2;

	if (!saveptr)
		return ((char *)NULL);

	if (s == NULL) {
		s = *saveptr;
		if (s == NULL) {
			return ((char *)NULL);
		}
	}

	s1 = s + strspn(s, ct);
	if (*s1 == '\0') {
		*saveptr = NULL;
		return ((char *)NULL);
	}

	s2 = strpbrk(s1, ct);
	if (s2) {
		*s2++ = '\0';
	}
	*saveptr = s2;
	return ((char *)s1);
}
