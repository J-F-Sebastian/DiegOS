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

#ifndef _ARM_H_
#define _ARM_H_

/*
 * CPSR values
 *
 * 31| 30| 29| 28| 27| 26 25| 24| 23 .. 20| 19 .. 16| 15 .. 10| 9| 8| 7| 6| 5| 4 .. 0
 * N | Z | C | V | Q |  RES | J | RESERVED| GE[3:0] | RESERVED| E| A| I| F| T| M[4:0]
 *
 */

#define CPSR_N_BIT (1 << 31)UL
#define CPSR_Z_BIT (1 << 30)UL
#define CPSR_C_BIT (1 << 29)UL
#define CPSR_V_BIT (1 << 28)UL
#define CPSR_Q_BIT (1 << 27)UL
#define CPSR_J_BIT (1 << 24)UL
/* GE are used on ARMv6 only */
/* E bit used on ARMv6 only */
/* A bit used on ARMv6 only */
#define CPSR_I_BIT (1 << 7)UL
#define CPSR_F_BIT (1 << 6)UL
#define CPSR_T_BIT (1 << 5)UL

#define CPSR_M_USR 0x10UL
#define CPSR_M_FIQ 0x11UL
#define CPSR_M_IRQ 0x12UL
#define CPSR_M_SVC 0x13UL
#define CPSR_M_ABT 0x17UL
#define CPSR_M_UND 0x1BUL
#define CPSR_M_SYS 0x1FUL

#define CPSR_GE_MSK (0xF << 16)UL
#define CPSR_M_MSK  (0x1 << 0)FUL

#endif
