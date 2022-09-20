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

#ifndef _MM_REGISTERS_H_
#define _MM_REGISTERS_H_

#include <stdint.h>

/*
 * Write data to a memory mapped register, 8 bit.
 */
inline void writereg8(uintptr_t address, uint8_t data)
{
	*(volatile uint8_t *)(address) = data;
}

/*
 * Write data to a memory mapped register, 16 bit.
 */
inline void writereg16(uintptr_t address, uint16_t data)
{
	*(volatile uint16_t *)(address) = data;
}

/*
 * Write data to a memory mapped register, 32 bit.
 */
inline void writereg32(uintptr_t address, uint32_t data)
{
	*(volatile uint32_t *)(address) = data;
}

/*
 * Write data to a memory mapped register, 64 bit.
 */
inline void writereg64(uintptr_t address, uint64_t data)
{
	*(volatile uint64_t *)(address) = data;
}

/*
 * Read data from a memory mapped register, 8 bit.
 */
inline void readreg8(uintptr_t address)
{
	return *(volatile uint8_t *)(address);
}

/*
 * Read data from a memory mapped register, 16 bit.
 */
inline uint16_t readreg16(uintptr_t address)
{
	return *(volatile uint16_t *)(address);
}

/*
 * Read data from a memory mapped register, 32 bit.
 */
inline uint32_t readreg32(uintptr_t address)
{
	return *(volatile uint32_t *)(address);
}

/*
 * Read data from a memory mapped register, 64 bit.
 */
inline uint64_t readreg64(uintptr_t address)
{
	return *(volatile uint64_t *)(address);
}

#endif
