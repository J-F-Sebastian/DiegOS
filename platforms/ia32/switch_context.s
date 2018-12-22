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
**      8(esp)  ->              const void *to
**      4(esp)  ->              void **from
**       (esp)  ->              return address
**
*/

.align 16
switch_context:
cli
movl    P1(%esp), %eax
movl    P2(%esp), %ebx
cmpl    %ebx, (%eax)
je      switch_exit
    
# Store context to the stack
pushfl
pushal

#store the new stack pointer to the actual context pointer.
#Note: all registers now are scratch registers...
movl    %esp, (%eax)
#CONTEXT SWITCH !!!!   
movl    %ebx, %esp

#Recover context
popal
popfl

switch_exit:
sti
ret
