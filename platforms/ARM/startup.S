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

/*
 * Startup code for N9H26
 */

#include "arm.h"

.text
.global ARM_start
.func ARM_start

ARM_start:

/* 
 * Vector table
 * NOTE: every platform MUST define the function calls
 * in the table, except for ARM_reset.
 */
 
/* Reset: relative branch allows remap */
B 	ARM_reset
/* Undefined Instruction */
B 	ARM_undefined_instruction
/* Software Interrupt */
B 	ARM_swi
/* Prefetch Abort */
B 	ARM_prefetch_abort
/* Data Abort */
B 	ARM_data_abort
/* Reserved */
B	.
/* IRQ */
B 	ARM_irq
/* FIQ */
B 	ARM_fiq

ARM_reset:

/* Initialize stack pointers for all ARM modes */
MSR	CPSR_c, #(CPSR_M_IRQ | CPSR_I_BIT | CPSR_F_BIT)
LDR	sp,=__irq_stack_start__ /* set the IRQ stack pointer */
MSR	CPSR_c, #(CPSR_M_FIQ | CPSR_I_BIT | CPSR_F_BIT)
LDR	sp,=__fiq_stack_start__ /* set the FIQ stack pointer */
MSR	CPSR_c, #(CPSR_M_SVC | CPSR_I_BIT | CPSR_F_BIT)
LDR	sp,=__svc_stack_start__ /* set the SVC stack pointer */
MSR	CPSR_c, #(CPSR_M_ABT | CPSR_I_BIT | CPSR_F_BIT)
LDR	sp,=__abt_stack_start__ /* set the ABORT stack pointer */
MSR	CPSR_c, #(CPSR_M_UND | CPSR_I_BIT | CPSR_F_BIT)
LDR	sp,=__und_stack_start__ /* set the UNDEFINED stack pointer */
MSR	CPSR_c, #(CPSR_M_SYS | CPSR_I_BIT | CPSR_F_BIT)
LDR	sp,=__sys_stack_start__ /* set the SYSTEM stack pointer */

/* Continue is supervisor mode */
MSR	CPSR_c, #CPSR_M_SVC

/* Interrupts are disabled in all modes at this point, set all 
 * general purpose registers to 0
 */
MOV	r0, #0
MOV	r1, #0
MOV	r2, #0
MOV	r3, #0
MOV	r4, #0
MOV	r5, #0
MOV	r6, #0
MOV	r7, #0
MOV	r8, #0
MOV	r9, #0
MOV	r10, #0
MOV	r11, #0
MOV	r12, #0
MOV	lr, #0

B	ARM_main

ARM_main:
B	DiegOS
