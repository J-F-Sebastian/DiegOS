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

#include <diegos/interrupts.h>

static int reentrant = 0;

void lock()
{
	/*
	 * Read the CPSR register, set the I and F bits,
	 * write back value.
	 * 0xC0 equals (CPSR_I_BIT | CPSR_F_BIT).
	 */
	__asm__ volatile ("MRS	R0, CPSR\n"
			 "ORR	R0, R0, #0xC0\n"
			 "MSR	CPSR_c, R0");

	reentrant++;
}

void unlock()
{
	if (reentrant > 0) --reentrant;
	if (reentrant == 0) {

		/*
		 * Read the CPSR register, clear the I and F bits,
		 * write back value.
		 * 0xC0 equals (CPSR_I_BIT | CPSR_F_BIT).
		 */
		__asm__ volatile ("MRS	R0, CPSR\n"
				 "BIC	R0, R0, #0xC0\n"
				 "MSR	CPSR_c, R0");
	}
}
