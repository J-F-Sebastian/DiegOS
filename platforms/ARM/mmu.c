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

#include "../../kernel/platform_include.h"
#include <types_common.h>
#include <errno.h>

static unsigned translation_table[4096] __attribute__((aligned(16 * KBYTE)));

void arm_mmu_enable()
{
	int reg0 = 0;
	uintptr_t reg1 = (uintptr_t) (translation_table);

	__asm volatile ("MCR     p15, #0, %0, c7, c7, #0  \n"	// Invalidate both I and D caches
			"MCR     p15, #0, %0, c7, c14, #0 \n"	// Clean and invalidate entire data cache
			"MCR     p15, #0, %1, c2, c0, #0  \n"	// write translation table base register c2
			"MRC     p15, #0, %0, c1, c0, #0  \n"	// read control register c1
			"ORR     %0, %0, #0x1000          \n"	// enable I cache bit
			"ORR     %0, %0, #0x5             \n"	// enable D cache and MMU bits
			"MCR     p15, #0, %0, c1, c0, #0  \n"	// write control register c1
			::"r" (reg0), "r"(reg1)
	    );
}

void arm_mmu_disable()
{
	int reg = 0;
	__asm volatile ("MCR     p15, #0, %0, c7, c5,  #0 \n"	// Flush I cache
			"MCR     p15, #0, %0, c7, c6,  #0 \n"	// Flush D cache
			"MCR     p15, #0, %0, c7, c10, #4 \n"	// Drain write buffer
			"MRC     p15, #0, %0, c1, c0,  #0 \n"	// read Control register c1
			"BIC     %0, %0, #1               \n"	// Disable MMU
			"MCR     p15, #0, %0, c1, c0,  #0 \n"	// write Control register c1
			::"r" (reg)
	    );
}

int arm_mmu_config(unsigned mbstart, unsigned mbsize, unsigned phy, unsigned caching)
{
	unsigned i;

	if (mbstart >= NELEMENTS(translation_table))
		return EINVAL;
	if (mbstart + mbsize >= NELEMENTS(translation_table))
		return EINVAL;
	if (phy >= NELEMENTS(translation_table))
		return EINVAL;
	if (caching & ~3)
		return EINVAL;

	/*
	 * Setup domains, all Client mode (accesses checked with transation_table)
	 */
	__asm volatile ("MCR     p15, #0, %0, c3, c0, #0"::"r" (0x55555555));

	/*
	 * We recycle variables and registers...
	 */

	phy <<= 20;
	/*
	 * AP set to 3 (read/write), 16th domain, section descriptor
	 */
	phy |= 0xDF2U;
	phy |= caching << 2;

	/*
	 * Every entry in the table is formatted according to this schema
	 *
	 * Bits         Description
	 * [31:20]      Form the corresponding bits of the physical address for a section
	 * [19:12]      Always written as 0
	 * [11:10]      The AP bits specify the access permissions for this section
	 * [9]          Always written as 0
	 * [8:5]        Specify one of the 16 possible domains,
	 *              held in the domain access control register,
	 *              that contain the primary access controls
	 * [4]          Should be written as 1, for backwards compatibility
	 * [3:2]        These bits (C and B) indicate if the area of memory mapped by this section
	 *              is treated as write-back cacheable, write-through cacheable, noncached buffered,
	 *              or noncached nonbuffered
	 * [1:0]        These bits must be 10 to indicate a section descriptor
	 *
	 */
	for (i = mbstart; i < mbsize; i++, phy += MBYTE) {
		translation_table[i] = phy;
	}

	return EOK;
}
