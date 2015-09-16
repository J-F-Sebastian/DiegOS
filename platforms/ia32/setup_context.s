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
**     16(esp)  ->		void *ctx
**     12(esp)  ->      void *entry_point
**      8(esp)  ->      void *fail_safe
**      4(esp)  ->      void *stack_ptr
**       (esp)  ->      return address
**
*/

setup_context:
movl	P4(%esp), %edx
movl	P3(%esp), %ecx
# store entry point into the context
movl    %ecx, PC(%edx)
movl    P1(%esp), %ecx
# store stack pointer, minus the pushed values!
subl	$8, %ecx
movl	%ecx, ESP(%edx)
xorl	%eax, %eax
# start the thread with all other registers set to 0,
# except for EFLAGS
movl	%eax, EAX(%edx)
movl	%eax, EBX(%edx)
movl	%eax, ECX(%edx)
movl	%eax, EDX(%edx)
movl	%eax, ESI(%edx)
movl	%eax, EDI(%edx)
movl	%eax, EBP(%edx)
# push eflags, store it into eax, and then to
# memory
pushf
popl    %eax
movl    %eax, EFLAGS(%edx)
movl	P2(%esp), %eax
movl	%eax, 4(%ecx)		# setup fail safe, ecx
# has the stack pointer
# as "pushed"
movl    P3(%esp), %eax
movl    %eax, 0(%ecx)       # setup entry point,
# this is not required
# as switching context will
# overwrite the value
ret

.size	setup_context, .-setup_context
