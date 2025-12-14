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

#include "../../../kernel/platform_include.h"
#include <types_common.h>
#include <platform/N9H26_sdram.h>
#include <platform/N9H26_mm_registers.h>

/*
 * The RAM memory map is
 *
 * +-----------------------+
 * |                       |
 * |       vectors         |  32 bytes
 * |                       |
 * +-----------------------+
 * |                       |
 * |    text and data      | 1 MByte - 32 bytes
 * |                       |
 * +-----------------------+
 * |                       |
 * |        heap           | RAM size - I/O memory
 * |                       |
 * +-----------------------+
 * |                       |
 * |        I/O            | I/O memory
 * |                       |
 * +-----------------------+
 *
 */

void cacheable_memory(void **base, unsigned long *size)
{
	unsigned long ram;

	total_memory(&ram);

	*base = (void *)(MBYTE);
	*size = ram - IOMEMORY_SIZE - MBYTE;
}

void io_memory(void **base, unsigned long *size)
{
	unsigned long ram;

	total_memory(&ram);
	/*
	 * I/O is allocated in the upper end of
	 * the RAM address space.
	 */
	*base = (void *)(ram - IOMEMORY_SIZE);
	*size = IOMEMORY_SIZE;
}

void total_memory(unsigned long *size)
{
	uint32_t bank0, bank1;

	bank0 = N9H26r(N9H26SDRAM_BASE, SDSIZE0) & DRAMSIZE;
	bank1 = N9H26r(N9H26SDRAM_BASE, SDSIZE1) & DRAMSIZE;

	if (bank0)
		bank0 = MBYTE << bank0;
	if (bank1)
		bank1 = MBYTE << bank1;

	*size = bank0 + bank1;
}
