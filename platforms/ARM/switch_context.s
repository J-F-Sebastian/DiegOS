/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2024 Diego Gallizioli
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

.file	"switch_context.s"
.include "context_equ.s"

.text
.globl switch_context
.type	switch_context, #function

/*
** on entry, registers look like this:
**      r0  ->              const void *to
**      r1  ->              void **from
**      r14 ->              return address
**
*/

switch_context:
ldr     r2, [r1]
cmp     r0, r2
beq     switch_exit
    
@ Store context to the stack using the full stack model
stmfd   sp!, {r0 - r12, r14}
@Note: all registers now are scratch registers...
mrs     r0, cpsr
str     r0, [sp], #4
@store the new stack pointer to the actual context pointer.
str     sp, [r1]
@CONTEXT SWITCH !!!!   
mov     sp, r0

@Recover context
ldr    r0, [sp], #4         @ retrieve the cspr register value (pop from stack)
msr    cpsr_fsxc, r0        @ restore the cspr
ldmfd  sp!, {r0 - r12, r14} @ restore all registers (pop from stack)

switch_exit:
mov    pc, r14              @ return
