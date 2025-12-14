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

.file	"load_context.S"

.text
.globl  load_context
.type	load_context, #function

/*
** on entry, registers look like this:
**       r0  ->              const void *to
**       r14 ->              return address
**
*/

load_context:
mov    sp, r0               @ move context pointer into sp, we know the context is the stack pointer
ldr    r0, [sp], #4         @ retrieve the cspr register value (pop from stack)
msr    cpsr_fsxc, r0        @ restore the cspr
ldmfd  sp!, {r0 - r12, r14} @ restore all registers (pop from stack)
mov    pc, r14              @ return
