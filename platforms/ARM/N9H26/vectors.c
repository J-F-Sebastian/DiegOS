/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2023 Diego Gallizioli
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
#include <stdlib.h>

/* Undefined instruction */
void ARM_undefined_instruction()
{
	fprintf(stderr, "### Undefined instruction.\n");
	abort();
}

/* Software Interrupt */
void ARM_swi()
{
	fprintf(stderr, "### SWI.\n");
}

/* Prefetch Abort */
void ARM_prefetch_abort()
{
	fprintf(stderr, "### Prefetch abort.\n");
	abort();
}

/* Data Abort */
void ARM_data_abort()
{
	fprintf(stderr, "### Data abort.\n");
	abort();
}
