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

#ifndef _ARM_H_
#define _ARM_H_

/*
 * CPSR values
 *
 * 31| 30| 29| 28| 27| 26 25| 24| 23 .. 20| 19 .. 16| 15 .. 10| 9| 8| 7| 6| 5| 4 .. 0
 * N | Z | C | V | Q |  RES | J | RESERVED| GE[3:0] | RESERVED| E| A| I| F| T| M[4:0]
 *
 */

/* Negative flag */
#define CPSR_N_BIT (1UL << 31)
/* Zero flag */
#define CPSR_Z_BIT (1UL << 30)
/* Carry flag */
#define CPSR_C_BIT (1UL << 29)
/* oVerflow flag */
#define CPSR_V_BIT (1UL << 28)
/* 
 * Q bit used on ARMv5TE and beyond.
 * Overflow on saturation.
 */
#define CPSR_Q_BIT (1UL << 27)
/* 
 * J bit used on ARMv5TE and beyond.
 * Jazelle instruction set selection.
 */
#define CPSR_J_BIT (1UL << 24)
/*
 * GE[3:0] are used on ARMv6 and beyond.
 * Greater than or equal flags for vector instructions.
 */
#define CPSR_GE_MSK (0xFUL << 16)
/*
 * E bit used on ARMv6 and beyond.
 * Set endianness of load/store operations.
 * If set the memory is BE.
 */
#define CPSR_E_BIT (1UL << 9)
/*
 * A bit used on ARMv6 and beyond.
 * Set imprecise data aborts.
 * If set disables imprecise data aborts
 */
#define CPSR_A_BIT (1UL << 8)
/* 
 * I bit disables IRQ if set.
 */
#define CPSR_I_BIT (1UL << 7)
/* 
 * F bit disables FIQ if set.
 */
#define CPSR_F_BIT (1UL << 6)
/* 
 * T bit used on ARMv4 and beyond.
 * Thumb instruction set selection.
 */
#define CPSR_T_BIT (1UL << 5)

#define CPSR_M_MSK  (0x1FUL << 0)

#define CPSR_M_USR 0x10UL
#define CPSR_M_FIQ 0x11UL
#define CPSR_M_IRQ 0x12UL
#define CPSR_M_SVC 0x13UL
#define CPSR_M_ABT 0x17UL
#define CPSR_M_UND 0x1BUL
#define CPSR_M_SYS 0x1FUL

#endif
