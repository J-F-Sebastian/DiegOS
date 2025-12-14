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

#ifndef _N9H26_MM_REGISTERS_H_
#define _N9H26_MM_REGISTERS_H_

#include <diegos/mm_registers.h>

/*
 * Write data to a memory mapped register
 */
inline void N9H26w(uintptr_t base, unsigned offset, uint32_t data)
{
	uintptr_t temp = regaddr32(base, offset);
	writereg32(temp, data);
}

/*
 * Read data from a memory mapped register
 */
inline uint32_t N9H26r(uintptr_t base, unsigned offset)
{
	uintptr_t temp = regaddr32(base, offset);
	return readreg32(temp);
}

/*
 * Update data in a memory mapped register
 * Operations for updating are
 * Read -> set using OR with data -> Write
 */
inline void N9H26u(uintptr_t base, unsigned offset, uint32_t data)
{
	N9H26w(base, offset, N9H26r(base, offset) | data);
}

/*
 * Clear data in a memory mapped register
 * Operations for clearing are
 * Read -> clear using AND NOT with data -> Write
 */
inline void N9H26c(uintptr_t base, unsigned offset, uint32_t data)
{
	N9H26w(base, offset, N9H26r(base, offset) & ~data);
}

/*
 * Clear and update data in a memory mapped register
 * Operations for clearing and updating are
 * Read offset 1 -> clear using AND NOT with data -> set using OR with data -> Write offset 2
 * Source and destination registers can be different
 */
inline void N9H26cu(uintptr_t base, unsigned ofs1, unsigned ofs2, uint32_t cdata, uint32_t udata)
{
	N9H26w(base, ofs1, (N9H26r(base, ofs2) & ~cdata) | udata);
}

#endif
