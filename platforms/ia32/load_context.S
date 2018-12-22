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

.file	"load_context.S"

.text
.globl  load_context
.type	load_context, @function

.equ    P1,     4
/*
** on entry, stack looks like this:
**      4(esp)  ->              const void *to
**       (esp)  ->              return address
**
*/

load_context:
cli
movl    P1(%esp), %eax          # move context pointer into eax
movl    %eax, %esp              # we know the context is the stack pointer

#If support for FP/MMX/SSE is desired, set the Task Switched bit to trap
# FP and SIMD instructions
#if defined(ENABLE_FP) || defined(ENABLE_SIMD)
call    set_ts
#endif          

#recover all registers, ESP will be skipped but this is exactly what we want
popal    
#recover EFLAGS
popfl
sti

#returning will load the thread entry point from stack and jump into it
ret
