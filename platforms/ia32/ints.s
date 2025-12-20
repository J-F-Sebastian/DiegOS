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

.file "ints.s"
.include "ints_equ.s"

.data
locked:
.int 0

.text

.globl lock		/* disable interrupts */
.type lock,@function

/*
 * void lock()
 * Disable CPU interrupts.
 */
lock:
cli
incl    locked
ret


.globl unlock		/* enable interrupts */
.type unlock,@function

/*
 * void unlock()
 * Enable CPU interrupts.
 */
unlock:
cmp    $0, locked
je     sk1
decl   locked
jnz    sk1
sti
sk1:
ret

.globl enable_irq	/* enable an irq at the 8259 controller */
.type enable_irq,@function

/*
 * void enable_irq(unsigned irq)
 * Enable an interrupt request line by clearing an 8259 bit.
 */
enable_irq:
cli
mov	4(%esp), %ecx		/* irq*/
movb	$~1, %ah
rolb	%cl, %ah		/* ah = ~(1 << (irq % 8))*/
cmpb	$8, %cl
jae	enable_8		/* enable irq >= 8 at the slave 8259*/

enable_0:
inb	    $INT_CTLMASK
andb	%ah, %al
outb	$INT_CTLMASK		/* clear bit at master 8259*/
sti
ret

enable_8:
inb	    $INT2_CTLMASK
andb	%ah, %al
outb	$INT2_CTLMASK		/* clear bit at slave 8259*/
sti
ret

.globl disable_irq
.type disable_irq,@function

/*
 * int disable_irq(unsigned irq)
 * Disable an interrupt request line by setting an 8259 bit.
 * Returns true if the interrupt was not already disabled.
 */
disable_irq:
cli
mov	4(%esp), %ecx		/* irq*/

movb	$1, %ah
rolb	%cl, %ah		/* ah = (1 << (irq % 8))*/
cmpb	$8, %cl
jae	disable_8		/* disable irq >= 8 at the slave 8259*/

disable_0:
inb	    $INT_CTLMASK
testb	%ah, %al
jnz	    dis_already		/* already disabled?*/
orb	    %ah, %al
outb	$INT_CTLMASK		/* set bit at master 8259*/
mov	    $1, %eax		/* disabled by this function*/
sti
ret

disable_8:
inb	$INT2_CTLMASK
testb	%ah, %al
jnz	    dis_already		/* already disabled?*/
orb	    %ah, %al
outb	$INT2_CTLMASK		/* set bit at slave 8259*/
mov	    $1, %eax		/* disabled by this function*/
sti
ret

dis_already:
xor	%eax, %eax		 /*already disabled*/
sti
ret

/*
 * This should not be here, but I did not want to add another
 * file to the codebase.
 */
.globl	power_save
.type   power_save,@function

/*
 * This function invoke the halt instruction to suspend the cpu
 * sti is invoked first, it should not be needed, but might it
 * happen the hlt instruction is invoked with interrupts disabled,
 * the system would freeze!
 */
power_save:
sti
hlt
ret

 .globl	enable_master_int
.type   enable_master_int,@function
