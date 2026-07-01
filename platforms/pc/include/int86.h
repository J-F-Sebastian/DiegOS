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

/*
 *	Bit # 	Mask 	Abbreviation 	Description 	        Category 	SET 	                CLEAR
 *	0 	0x0001 	CF 	        Carry flag 	        Status 	        CY (Carry) 	        NC (No Carry)
 *	2 	0x0004 	PF 	        Parity flag 	        Status 	        PE (Parity Even) 	PO (Parity Odd)
 *	4 	0x0010 	AF 	        Auxiliary Carry flag 	Status 	        AC (Auxiliary Carry) 	NA (No Auxiliary Carry)
 *	6 	0x0040 	ZF 	        Zero flag 	        Status   	ZR (Zero) 	        NZ (Not Zero)
 *	7 	0x0080 	SF 	        Sign flag 	        Status 	        NG (Negative)    	PL (Positive)
 *	8 	0x0100 	TF 	        Trap flag (single step)	Control
 *	9 	0x0200 	IF 	        Interrupt enable flag 	Control 	EI (Enable Interrupt) 	DI (Disable Interrupt)
 *	10 	0x0400 	DF 	        Direction flag 	        Control 	DN (Down) 	        UP (Up)
 *	11 	0x0800 	OF 	        Overflow flag 	        Status  	OV (Overflow)   	NV (Not Overflow)
 *	12–13 	0x3000 	IOPL 	        I/O privilege level
 *        	                        (286+ only), always
 *					all-1s on 8086 and 186 	System
 * 	14 	0x4000 	NT 	        Nested task flag (286+ only),
 *        	                        always 1 on 8086 and 186System
*/

typedef struct __attribute__((packed)) {
	uint16_t di, si, bp, sp, bx, dx, cx, ax;
	uint16_t gs, fs, es, ds, flags;
} regs16_t;

inline void prot_to_seg_ofs(void *prot, uint16_t *seg, uint16_t *ofs)
{
	*seg = ((uintptr_t) prot >> 4) & 0xFFFFUL;
	*ofs = ((uintptr_t) prot & 0xFUL);
}

inline void *seg_ofs_to_prot(uint16_t seg, uint16_t ofs)
{
	uintptr_t retval = seg;
	retval *= 16;
	retval += ofs;
	return (void *)retval;
}

inline void *prot_to_real(const void *prot)
{
	uintptr_t retval = ((uintptr_t) prot & 0xFFFF0UL) << 12;
	retval |= ((uintptr_t) prot & 0xFUL);
	return (void *)retval;
}

inline void *real_to_prot(const void *real)
{
	uintptr_t retval = (uintptr_t) real >> 16;
	retval *= 16;
	retval += (uintptr_t) real & 0xFFFFUL;
	return (void *)retval;
}

extern void int86(unsigned char intnum, regs16_t * regs);

extern void *int86_real_buffer(void);

extern unsigned int86_real_buffer_size(void);

extern void *ISA_DMA_real_buffer(unsigned idx);

extern unsigned ISA_DMA_real_buffer_size(unsigned idx);

#endif
