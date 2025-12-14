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

.file	"setup_context.s"
.include "context_equ.s"

.text
.globl setup_context
.type	setup_context, #function

/*
** on entry, registers look like this:
**     r0  ->      void **ctx
**     r1  ->      void *entry_point
**     r2  ->      void *fail_safe
**     r3  ->      void *stack_ptr
**     r14 ->      return address
**
*/

setup_context:
#step 1 update link register for first frame, which is the fail safe frame.
#this is required to let any thread NOT calling the proper thread_terminate()
#return to a default terminating procedure.
#move fail_safe to the thread's stack memory at lr position
str	r2, [r3, #-TLR]

#step 2 set r2 to 0, init stored registers by pushing values to the frame
sub	r2, r2
#r0
str	r2, [r3, #-4]!
#r1
str	r2, [r3, #-4]!
#r2
str	r2, [r3, #-4]!
#r3
str	r2, [r3, #-4]!
#r4
str	r2, [r3, #-4]!
#r5
str	r2, [r3, #-4]!
#r6
str	r2, [r3, #-4]!
#r7
str	r2, [r3, #-4]!
#r8
str	r2, [r3, #-4]!
#r9
str	r2, [r3, #-4]!
#r10
str	r2, [r3, #-4]!
#r11
str	r2, [r3, #-4]!
#r12
str	r2, [r3, #-4]!

#step 3 init default cpsr to system mode and skip link register position
orr	r2, r2, #0x1f
str	r2, [r3, #-8]!

#step 4 update the context by storing the updated stack pointer, that's all we need
str	r3, [r0]

mov pc, r14
