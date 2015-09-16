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

.file	"switch_context.s"
.include "context_equ.s"

.text
.globl switch_context
.type	switch_context, @function

.equ    P1,     4
.equ    P2,     8
/*
** on entry, stack looks like this:
**      8(esp)  ->              thread *to
**      4(esp)  ->              thread *from
**       (esp)  ->              return address
**
** we push the current eax on the stack so that we can use it as
** a pointer to t1, this decrements esp by 4, so when we use it
** to reference stuff on the stack, we add 4 to the offset.
** REMEMBER: DiegOS requires stack pointer to be in first place,
**           return address (Program Counter) in the second place.
*/

.align 16
switch_context:
cli
pushl   %ebp			        # save the value of ebp and use it to reference
                                # the context
pushfl                          # save the flags
movl    8+P1(%esp), %ebp        # move pointer to (from) into ebp
movl    %eax, EAX(%ebp)         # save registers
movl    %ebx, EBX(%ebp)
movl    %ecx, ECX(%ebp)
movl    %edx, EDX(%ebp)

movl    %ebp, %ebx              # ebx now references the context
movl    8+0(%esp), %ecx           # get return address from stack into ecx
popl    %eax                    # eax now saves the flags
popl    %ebp                    # restores the original ebp

movl    %esi, ESI(%ebx)
movl    %edi, EDI(%ebx)
movl    %ebp, EBP(%ebx)
movl    %esp, ESP(%ebx)         # save stack pointer
movl    %eax, EFLAGS(%ebx)      # store EFLAGS
movl    %ecx, PC(%ebx)          # save EIP into the pc storage

movl    P2(%esp), %eax          # move pointer to (to) into eax

movl    EFLAGS(%eax), %ecx      # restore EFLAGS so we do not mess with the stack
pushl   %ecx                    # anymore
popf
movl    EBX(%eax), %ebx         # restore old registers
movl    ECX(%eax), %ecx
movl    EDX(%eax), %edx
movl    ESI(%eax), %esi
movl    EDI(%eax), %edi
movl    EBP(%eax), %ebp
movl    ESP(%eax), %esp         # restore stack pointer
movl    PC(%eax), %eax          # restore return address into eax
movl    %eax, 0(%esp)           # copy over the ret address on the stack
movl    EAX(%eax), %eax		    # restore eax
sti
ret

.size	switch_context, .-switch_context
