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

#include "arm.h"
#include "arm_cp.h"

.file	"power_save.S"

.text
.globl power_save
.func  power_save

power_save:
sub	r0, r0
/* Read control register */
mrc	p15, 0, r1, c1, c0, 0
/* Data Synchronization Barrier (Drain write buffer) */
mcr	p15, 0, r0, c7, c10, 4
/* Clear I cache bit */
bic	r2, r1, #CP15_C0_I_BIT
/* Disable FIQs while I cache is disabled */
mrs	r3, cpsr
orr	ip, r3, #CPSR_I_BIT
msr	cpsr_c, ip
/* Disable I cache */
mcr	p15, 0, r2, c1, c0, 0
/* Wait for interrupt */
mcr	p15, 0, r0, c7, c0, 4
/* Restore I cache state */
mcr	p15, 0, r1, c1, c0, 0
/* Restore FIQ state */
msr	cpsr_c, r3
bx	lr

