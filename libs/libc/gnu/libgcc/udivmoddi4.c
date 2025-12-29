/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2022 Diego Gallizioli
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

#include <stdint.h>

uint64_t __udivmoddi4(uint64_t a, uint64_t b, uint64_t *c)
{
	if (c)
		*c = a % b;

	/*
	 * This seems to be the behavior coded in libgcc GNU source code...
	 */
	if (b)
		return (a / b);
	else
		return 0;
}
