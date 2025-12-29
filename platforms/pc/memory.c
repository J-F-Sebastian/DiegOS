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

#include "../../kernel/platform_include.h"
#include <platform/int86.h>
#include <types_common.h>

extern long _bss_end;

void cacheable_memory(void **base, unsigned long *size)
{
	regs16_t regs;
	unsigned long mem;

	regs.ax = 0xE801;
	int86(0x15, &regs);
	//assert on error ?
	/*
	 * is carry bit set?
	 */
	if (regs.eflags & 1) {
		*base = NULL;
		*size = 0;
		return;
	}

	if (regs.ax == 0) {
		regs.ax = regs.cx;
		regs.bx = regs.dx;
	}

	/*
	 * The first MByte is a legacy of shadow memories,
	 * mapped VGA devices, etc.
	 * Executable code is running above 1 Mbyte (plus 64KByte).
	 * Don't mess with this.
	 * NOTE: interrupt services report available memory
	 * above 1Mbyte.
	 */
	mem = (regs.ax << 10) + (regs.bx << 16);
	*base = (void *)(1UL * MBYTE + 64UL * KBYTE + (uintptr_t) (&_bss_end));
	*size = mem - IOMEMORY_SIZE - (uintptr_t) (*base);
}

void io_memory(void **base, unsigned long *size)
{
	regs16_t regs;
	unsigned long mem;

	regs.ax = 0xE801;
	int86(0x15, &regs);
	//assert on error ?
	/*
	 * is carry bit set?
	 */
	if (regs.eflags & 1) {
		*base = NULL;
		*size = 0;
		return;
	}

	if (regs.ax == 0) {
		regs.ax = regs.cx;
		regs.bx = regs.dx;
	}

	/*
	 * The first MByte is a legacy of shadow memories,
	 * mapped VGA devices, etc.
	 * Executable code is running above 1 Mbyte (plus 64KByte).
	 * Don't mess with this.
	 * NOTE: interrupt services report available memory
	 * above 1Mbyte.
	 */
	mem = (regs.ax << 10) + (regs.bx << 16);
	mem = (regs.ax << 10) + (regs.bx << 16);
	*base = (void *)(mem - IOMEMORY_SIZE);
	*size = IOMEMORY_SIZE;
}

void total_memory(unsigned long *size)
{
	regs16_t regs;
	unsigned long mem;

	regs.ax = 0xE801;
	int86(0x15, &regs);
	//assert on error ?
	/*
	 * is carry bit set?
	 */
	if (regs.eflags & 1) {
		*size = 0;
		return;
	}

	if (regs.ax == 0) {
		regs.ax = regs.cx;
		regs.bx = regs.dx;
	}

	/*
	 * NOTE: interrupt services report available memory
	 * above 1Mbyte.
	 */
	mem = (regs.ax << 10) + (regs.bx << 16);
	mem = (regs.ax << 10) + (regs.bx << 16);
	*size = mem + 1UL * MBYTE;
}
