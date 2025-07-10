/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2025 Diego Gallizioli
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
#include <stdint.h>
#include <processor/ia32.h>
#include <processor/mtrr.h>
#include "ia32_private.h"

/*
 * Defaults to mask for 32 physical address bits
 * Mask is right-shifted by 12 bits.
 */
static unsigned phys_len_mask = 0xFFFFFUL;
static unsigned mtrr_count = 0;
static unsigned mtrr_pos = 0;

/*
 * flags
 *
 * 1 WC is supported
 */
static unsigned flags = 0;

void init_mtrr()
{
	struct cpuid_data info;
	uint32_t buffer[2];
	unsigned phy;

	/*
	 * IA32_MTRRCAP (MTRRcap), MTRRCapability (RO)
	 */
	read_msr(buffer, 0xFE);
	if ((buffer[0] >> 10) & 1) {
		flags |= 1;
	}
	mtrr_count = (buffer[0] & 0xFFUL);

	/*
	 * If 0x80000008 is not supported the physical
	 * address bits are 36.
	 * The mask is 32 bit long.
	 * Physical address bits minus the mask shift
	 * (12 bits) as used by MTRR gives the number
	 * of bits to shift right a 0xFFFFFFFF mask
	 */
	execute_cpuid(&info, 0x80000000UL);
	if (info.word[EAX] > 0x80000007UL) {
		execute_cpuid(&info, 0x80000008UL);
		phy = info.word[EAX] & 0xFFUL;
		phy -= 12;
		phys_len_mask = UINT_MAX >> (32 - phy);
	} else {
		phys_len_mask = UINT_MAX >> (32 - (36 - 12));
	}
}

BOOL mtrr_configure(uintptr_t base, uintptr_t size, int cache_type)
{
	uintptr_t upperaddr;
	uint32_t buffer[2];
	unsigned reg = 0x200;

	if (mtrr_pos == mtrr_count)
		return FALSE;
	if (base & 0xFFFUL)
		return FALSE;
	if (size & 0xFFFUL)
		return FALSE;
	if (cache_type == MTRR_TYPE_WC) {
		if ((flags & 1) == 0)
			return FALSE;
	} else {
		if ((cache_type != MTRR_TYPE_UC) &&
		    (cache_type != MTRR_TYPE_WT) &&
		    (cache_type != MTRR_TYPE_WP) && (cache_type != MTRR_TYPE_WB))
			return FALSE;
	}

	upperaddr = size - 1;
	upperaddr = ~upperaddr;
	upperaddr >>= 12;
	upperaddr &= phys_len_mask;
	base >>= 12;

	reg += 2 * mtrr_pos;

	/*
	 * IA32_MTRR_PHYSBASE0
	 */
	read_msr(buffer, reg);

	buffer[0] &= ~0xFUL;
	buffer[0] |= cache_type & 0xFUL;
	buffer[0] &= ~(phys_len_mask << 12);
	buffer[0] |= (base << 12);

	/*
	 * We are not using 36 or longer linear addresses
	 * and we are not addressing more than 4 GBytes
	 */
	if (phys_len_mask >> 20) {
		buffer[1] &= ~(phys_len_mask >> 20);
	}

	write_msr(buffer, reg++);

	/*
	 *  IA32_MTRR_PHYSMASK0
	 */
	read_msr(buffer, reg);

	buffer[0] &= ~(phys_len_mask << 12);
	buffer[0] |= (upperaddr << 12);
	buffer[0] |= (1UL << 11);
	/*
	 * We are not using 36 or longer linear addresses
	 * and we are not addressing more than 4 GBytes
	 */
	if (phys_len_mask >> 20) {
		buffer[1] &= ~(phys_len_mask >> 20);
		buffer[1] |= (phys_len_mask >> 20);
	}

	write_msr(buffer, reg);

	mtrr_pos++;
	return TRUE;
}

void mtrr_dump()
{
	uint32_t buffer[2];
	unsigned count;
	const unsigned freg[] =
	    { 0x250, 0x258, 0x259, 0x268, 0x269, 0x26A, 0x26B, 0x26C, 0x26D, 0x26E, 0x26F, 0 };
	unsigned reg = 0x200;

	printf("=== MTRR Registers\n");

	printf("PHY mask %X\n", phys_len_mask);

	/*
	 * Check IA32_MTRR_DEF_TYPE
	 */
	read_msr(buffer, 0x2FF);
	printf("Default type %d\n", buffer[0] & 7UL);

	/*
	 * IA32_MTRRCAP (MTRRcap), MTRRCapability (RO)
	 */
	read_msr(buffer, 0xFE);
	printf("Write Combine (%c) Fixed Ranges (%c)\n", ((buffer[0] >> 10) & 1) ? 'Y' : 'N',
	       ((buffer[0] >> 8) & 1) ? 'Y' : 'N');

	/*
	 * IA32_MTRR_FIX
	 */
	if ((buffer[0] >> 8) & 1) {
		count = 0;
		while (freg[count]) {
			read_msr(&buffer, freg[count]);
			printf("Fixed range %X -> %X (%d)\n", freg[count], buffer, count);
			count++;
		}
	}

	if (mtrr_count) {
		count = 0;
		while (count < mtrr_count) {
			/*
			 * IA32_MTRR_PHYSBASE0
			 */
			read_msr(buffer, reg++);
			printf("(%d)\ttype %d\t%X:%X ", count, buffer[0] & 0xFFUL, buffer[1],
			       buffer[0]);

			/*
			 *  IA32_MTRR_PHYSMASK0
			 */
			read_msr(buffer, reg++);
			/*
			 * If bit V (11) is set the entry is Valid
			 */
			printf("-> %X:%X V %c\n", buffer[1], buffer[0],
			       buffer[0] & (1UL << 11) ? 'Y' : 'N');
			++count;
		}
	}
	printf("===\n");
}
