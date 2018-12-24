/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2015 Diego Gallizioli
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

#ifndef _INT_86_H_
#define _INT_86_H_

#include <stdint.h>

typedef struct __attribute__ ((packed)) {
uint16_t di, si, bp, sp, bx, dx, cx, ax;
uint16_t gs, fs, es, ds, eflags;
} regs16_t;

inline void prot_to_seg_ofs (void *prot, uint16_t *seg, uint16_t *ofs)
{
	*seg = ((uintptr_t)prot >> 4) & 0xFFFFUL;
	*ofs = ((uintptr_t)prot & 0xFUL);
}

inline void *seg_ofs_to_prot (uint16_t seg, uint16_t ofs)
{
	uintptr_t retval = seg;
	retval *=16;
	retval += ofs;
	return (void *)retval;
}

inline void *prot_to_real (const void *prot)
{
	uintptr_t retval = ((uintptr_t)prot & 0xFFFF0UL) << 12;
	retval |= ((uintptr_t)prot & 0xFUL);
	return (void *)retval;
}

inline void *real_to_prot (const void *real)
{
	uintptr_t retval = (uintptr_t)real >> 16;
	retval *= 16;
	retval += (uintptr_t)real & 0xFFFFUL;
	return (void *)retval;
}

extern void int86(unsigned char intnum, regs16_t *regs);

extern void *real_buffer (void);

extern unsigned real_buffer_size (void);

#endif
