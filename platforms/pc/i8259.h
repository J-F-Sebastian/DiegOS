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

#ifndef _I8259_H_
#define _I8259_H_

/* Suitable irq bases for hardware interrupts.  Reprogram the 8259(s) from
 * the PC BIOS defaults since the BIOS doesn't respect all the processor's
 * reserved vectors (0 to 31).
 */
#define IRQ0_VECTOR     0x20	/* nice vectors to relocate IRQ0-7 to */
#define IRQ8_VECTOR     0x28	/* nice vectors to relocate  IRQ8-15 to */

#define NR_IRQ_VECTORS    16

/* Hardware interrupt numbers. */
#define CLOCK_IRQ          0
#define KEYBOARD_IRQ       1
#define CASCADE_IRQ        2	/* cascade enable for 2nd AT controller */
#define SECONDARY_IRQ      3	/* RS232 interrupt vector for port 2 */
#define RS232_IRQ          4	/* RS232 interrupt vector for port 1 */
#define XT_WINI_IRQ        5	/* xt winchester */
#define FLOPPY_IRQ         6	/* floppy disk */
#define PRINTER_IRQ        7
#define RTC_IRQ		       8    /*  Cascaded IRQ 2 interrupt from master 8259 */
#define PS2MOUSE_IRQ	  12
#define CP_IRQ		      13
#define AT_WINI_0_IRQ     14	/* at winchester */
#define AT_WINI_1_IRQ     15	/* at winchester */

/* 8259A interrupt controller ports. */
#define INT_CTL         0x20	/* I/O port for interrupt controller */
#define INT_CTLMASK     0x21	/* setting bits in this port disables ints */
#define INT2_CTL        0xA0	/* I/O port for second interrupt controller */
#define INT2_CTLMASK    0xA1	/* setting bits in this port disables ints */

/* Magic numbers for interrupt controller. */
#define SPEC_EOI        0x60	/* Specific End Of Interrupt, re-enable an
                                 * interrupt after servicing it
                                 */

void i8259_init(void);

#endif

