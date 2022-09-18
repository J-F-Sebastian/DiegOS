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

#include <string.h>
#include <stdio.h>
#include <types_common.h>
#include "apic.h"
#include "ia32_private.h"
#include "ia32.h"

static volatile unsigned *apic_base = NULL;

/*
 * NOT PUBLIC
 */
void init_apic(void)
{
	unsigned buffer[2];

	/*
	 * Read IA32_APIC_BASE
	 */
	read_msr(buffer, 0x1B);

	/*
	 * If not enabled, set to enabled
	 */
	buffer[0] |= (1UL << 11);
	write_msr(buffer, 0x1B);

	/*
	 * Store the memory address of the APIC
	 */
	apic_base = (unsigned *)(buffer[0] & (0xFFFFFUL << 12));

	/*
	 * Enable APIC from Software
	 */
	apic_base[0x0F0 / sizeof(*apic_base)] = (1 << 8);
}

BOOL is_apic_supported()
{
	unsigned buffer[2];

	/*
	 * Read IA32_APIC_BASE
	 */
	read_msr(buffer, 0x1B);

	return (buffer[0] & (1UL << 11)) ? TRUE : FALSE;
}

BOOL apic_configure(char oneshot, char enableint, char vector, char divisor)
{
	unsigned temp = 0;

	if (!apic_base)
		return (FALSE);

	temp = (unsigned)vector;
	if (!enableint)
		temp |= (1UL << 16);
	if (!oneshot)
		temp |= (1UL << 17);

	apic_base[0x320 / sizeof(*apic_base)] = temp;

	temp = 0;
	temp |= divisor;
	if (temp & ~APIC_DIV_1)
		return FALSE;

	apic_base[0x3E0 / sizeof(*apic_base)] = temp;

	return (TRUE);
}

BOOL apic_write_counter(unsigned count)
{
	if (!apic_base)
		return (FALSE);

	apic_base[0x380 / sizeof(*apic_base)] = count;

	return (TRUE);
}

unsigned apic_read_counter()
{
	if (!apic_base)
		return (0);

	return apic_base[0x390 / sizeof(*apic_base)];
}
