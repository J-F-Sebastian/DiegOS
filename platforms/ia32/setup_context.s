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

.file	"setup_context.s"
.include "context_equ.s"

.text
.globl setup_context
.type	setup_context, @function

.equ    P1,     4
.equ    P2,     8
.equ    P3,     12
.equ    P4,     16

/*
** on entry, stack looks like this:
**     16(esp)  ->		void **ctx
**     12(esp)  ->      void *entry_point
**      8(esp)  ->      void *fail_safe
**      4(esp)  ->      void *stack_ptr
**       (esp)  ->      return address
**
*/

setup_context:
movl	P4(%esp), %edx
movl	P3(%esp), %ecx
movl	P2(%esp), %ebx
movl	P1(%esp), %eax

#Step 1 update the stack pointer to the "pushed" value
subl    $(RETPTR+4), %eax

#Step 2 move fail_safe and entry_point to the thread's stack memory
movl    %ebx, RETPTR(%eax)
movl    %ecx, STRPTR(%eax)

#Step 3 init stored registers, init values to 0 except for stack pointer and
#flags
subl    %ebx, %ebx
movl    %ebx, EAX(%eax)
movl	%ebx, ECX(%eax)
movl	%ebx, EDX(%eax)
movl	%ebx, EBX(%eax)
movl    %eax, ESP(%eax) 
movl	%ebx, EBP(%eax)
movl	%ebx, ESI(%eax)
movl	%ebx, EDI(%eax)
#Reset default value of EFLAGS
movl    $0x2, EFLAGS(%eax)

#Step 4 update the context by storing the stack pointer, that's all we need
movl    %eax, (%edx)
ret
