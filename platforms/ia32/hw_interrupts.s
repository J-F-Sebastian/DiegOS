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

.file    "hw_interrupts.s"
.include "ints_equ.s"

.text
.globl hwint00
.globl hwint01
.globl hwint02
.globl hwint03
.globl hwint04
.globl hwint05
.globl hwint06
.globl hwint07
.globl hwint08
.globl hwint09
.globl hwint10
.globl hwint11
.globl hwint12
.globl hwint13
.globl hwint14
.globl hwint15
.type  hwint00, @function
.type  hwint01, @function
.type  hwint02, @function
.type  hwint03, @function
.type  hwint04, @function
.type  hwint05, @function
.type  hwint06, @function
.type  hwint07, @function
.type  hwint08, @function
.type  hwint09, @function
.type  hwint10, @function
.type  hwint11, @function
.type  hwint12, @function
.type  hwint13, @function
.type  hwint14, @function
.type  hwint15, @function

/*
 * Interrupts are serviced with interrupt gates, running at CPL 0, the same
 * privilege of the running code.
 * Since there is no change in the privilege of the running task, servicing
 * an interrupt is definitevely easy.
 * When the processor receives an interrupt signal the EFLAGS, CS, EIP and
 * an error code are pushed on the active stack - using the actual ESP value.
 * The effect of this action is to push 16 bytes onto the stack of any running
 * OS task, depending on which task was active when the interrupt was raised.
 * The ISRs are written to re-enable interrupts as fast as possible, letting
 * other interrupts to be serviced in a nested fashion.
 * Nesting ISRs might cause excessive data pushes onto a single stack - if multiple
 * interrupts happen simultaneously the processor will push 16 bytes for each
 * interruption.
 * If the ISR need to push additional data then a corruption might take place:
 * OS threads can configure the size of the stack and this size is not dynamic.
 * If the stack has not enough space to push all required data it will overwrite
 * other data locations.
 */

/*
 *===========================================================================*
 *                               interrupt handlers                          *
 *              interrupt handlers for 386 32-bit protected mode             *
 *===========================================================================*
 */
/*
 *===========================================================================*
 *                               hwint00 - 07                                *
 *===========================================================================*
 * Note this is a macro, it looks like a subroutine.
 */
.macro hwint_master irq
cld
pusha   /* stack pointer is not changed */
/* Mask this interrupt */
inb     $INT_CTLMASK
orb     $(1<<\irq), %al
outb    $INT_CTLMASK
movb    $(SPEC_EOI|\irq), %al
/* enable master 8259 int */
outb    $INT_CTL
/* enable interrupts */
sti
/*  Service the interrupt now */
/* eax = (*int_table[irq])()  */
call    *int_table+4*\irq(,1)
/* disable interrupts */
cli
/* Unmask this interrupt */
inb     $INT_CTLMASK
andb    $(~1<<\irq), %al
outb    $INT_CTLMASK
popa
iretl
.endm

/* Each of these entry points is an expansion of the hwint_master macro */
.align 64
hwint00: /* Interrupt routine for irq 0 (the clock).*/
hwint_master 0

.align 16
hwint01: /* Interrupt routine for irq 1 (keyboard) */
hwint_master 1

.align 16
hwint02: /* Interrupt routine for irq 2 (cascade!) */
hwint_master 2

.align 16
hwint03: /* Interrupt routine for irq 3 (second serial) */
hwint_master 3

.align 16
hwint04: /* Interrupt routine for irq 4 (first serial) */
hwint_master 4

.align 16
hwint05: /* Interrupt routine for irq 5 (XT winchester) */
hwint_master 5

.align 16
hwint06: /* Interrupt routine for irq 6 (floppy) */
hwint_master 6

.align 16
hwint07: /* Interrupt routine for irq 7 (printer) */
hwint_master 7

/*
 *===========================================================================*
 *                            hwint08 - 15                                   *
 *===========================================================================*
 *
 *Note this is a macro, it looks like a subroutine.
 */
.macro hwint_slave irq
cld
pusha
/* Mask this interrupt */
inb     $INT2_CTLMASK
orb     $(1<<(\irq-8)), %al
outb    $INT2_CTLMASK
/* enable slave 8259  */
movb    $(SPEC_EOI|(\irq-8)), %al
outb    $INT2_CTL
/* enable master 8259 */
movb    $(SPEC_EOI|CASCADE_IRQ), %al
outb    $INT_CTL
/* enable interrupts */
sti
/*  Service the interrupt now */
/* eax = (*int_table[irq])()  */
call    *int_table+4*\irq(,1)
/* disable interrupts */
cli
/* Mask this interrupt */
inb     $INT2_CTLMASK
andb    $(~1<<(\irq-8)), %al
outb    $INT2_CTLMASK
popa
iretl
.endm

/* Each of these entry points is an expansion of the hwint_slave macro */
.align  16
hwint08: /* Interrupt routine for irq 8 (realtime clock) */
hwint_slave 8

.align  16
hwint09: /* Interrupt routine for irq 9 (irq 2 redirected) */
hwint_slave 9

.align  16
hwint10: /* Interrupt routine for irq 10 */
hwint_slave 10

.align  16
hwint11: /* Interrupt routine for irq 11 */
hwint_slave 11

.align  16
hwint12: /* Interrupt routine for irq 12 */
hwint_slave 12

.align  16
hwint13: /* Interrupt routine for irq 13 (FPU exception) */
hwint_slave 13

.align  16
hwint14: /* Interrupt routine for irq 14 (AT winchester) */
hwint_slave 14

.align  16
hwint15: /* Interrupt routine for irq 15 */
hwint_slave 15

