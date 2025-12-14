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

#ifndef _ARM_CP_H_
#define _ARM_CP_H_

/*
 * Coprocessor CP15 C0 values
 *
 * 31 .. 27| 26| 25| 24| 23| 22| 21| 20 .. 16| 15| 14| 13| 12| 11| 10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0
 * UNP/SBZP| L2| EE| VE| XP| U | FI|         | L4| RR| V | I | Z | F | R| S| B| L| D| P| W| C| A| M
 *
 */

/* L2 Unified Cache enable */
#define CP15_C0_L2_BIT (1UL << 26)
/* Mixed Endian exception entry */
#define CP15_C0_EE_BIT (1UL << 25)
/* Configure vectored interrupts */
#define CP15_C0_VE_BIT (1UL << 24)
/* Configure Extended page table */
#define CP15_C0_XP_BIT (1UL << 23)
/* Enable unaligned data access */
#define CP15_C0_U_BIT (1UL << 22)
/* Configure Fast Interrupt configuration */
#define CP15_C0_FI_BIT (1UL << 21)
/* Inhibits ARMv5T Thumb interwork */
#define CP15_C0_L4_BIT (1UL << 15)
/* Cache replacement strategy (random or Round Robin) */
#define CP15_C0_RR_BIT (1UL << 14)
/* Exception vectors location (0x0000 or 0xFFFF based) */
#define CP15_C0_V_BIT (1UL << 13)
/* Enable L1 instruction cache (if not unified) */
#define CP15_C0_I_BIT (1UL << 12)
/* Enable branch prediction */
#define CP15_C0_Z_BIT (1UL << 11)
/* Implementation defined */
#define CP15_C0_F_BIT (1UL << 10)
/* ROM Protection bit */
#define CP15_C0_R_BIT (1UL << 9)
/* System Protection bit */
#define CP15_C0_S_BIT (1UL << 8)
/* Endianness, LE or BE */
#define CP15_C0_B_BIT (1UL << 7)
/* Enable Write buffer */
#define CP15_C0_W_BIT (1UL << 3)
/* Enable L1 Unified cache */
#define CP15_C0_C_BIT (1UL << 2)
/* Alignment, relaxed or strict */
#define CP15_C0_A_BIT (1UL << 1)
/* Enable MMU */
#define CP15_C0_M_BIT (1UL << 0)

#endif
