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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <types_common.h>
#include <diegos/interrupts.h>

#include "ia32_private.h"
#include "ints_private.h"
#include "i8259.h"

/*
 * Memory layout on IA32
 *
 * 0x0600
 *
 * IDT TABLE (256 entries, 2048 bytes)
 *
 * ......
 * ......
 *
 * 0x0E00
 *
 * GDT TABLE (NULL, CODE, DATA, +5)
 *
 * 0xE40
 *
 * IDTR
 *
 * 0xE48
 *
 * GDTR
 */

#define MAX_INT (64)
#define MAX_EXC (20)

/*
 * Mapping of interrupt vectors
 *
 * R 	reserved to exceptions and processor interrupts
 * IRQ	allocated to i8259 interrupts, IRQ0 to IRQ15
 * SW	configurable for software-generated interrupts
 * HW	configurable for hardware-generated interrupts
 *
 * There are 32 reserved vectors, 16 vectors for i8259 hardware interrupts,
 * 16 vectors for software interrupts, 32 vectors for additional hardware interrupts,
 * totalling 96 addressed interrupts, 64 available for the OS.
 * All other vectors (160 remaining) are available but not configured.
 * To make use of them you need to increase MAX_INT defines.
 *
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B | C | D | E | F |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * | 0 | R | R | R | R | R | R | R | R | R | R | R | R | R | R | R | R |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * | 1 | R | R | R | R | R | R | R | R | R | R | R | R | R | R | R | R |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * | 2 |IRQ|IRQ|IRQ|IRQ|IRQ|IRQ|IRQ|IRQ|IRQ|IRQ|IRQ|IRQ|IRQ|IRQ|IRQ|IRQ|
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * | 3 |SW |SW |SW |SW |SW |SW |SW |SW |SW |SW |SW |SW |SW |SW |SW |SW |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * | 4 |HW |HW |HW |HW |HW |HW |HW |HW |SW |HW |HW |HW |HW |HW |HW |HW |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * | 5 |HW |HW |HW |HW |HW |HW |HW |HW |HW |HW |HW |HW |HW |HW |HW |HW |
 * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * | 6 | .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
 * +---+
 * | 7 | .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
 * +---+
 *  ...
 *  ...
 */

const uint16_t FLAGS = 0x8E00;

typedef struct _interrupt_gate {
	uint16_t offset_low;
	uint16_t segment;
	uint16_t flags;
	uint16_t offset_hi;
} interrupt_gate;

/*
 * IDT interrupt table handlers, see boot32.s for details
 */
static interrupt_gate *idt_table = (interrupt_gate *) (0x600);

/*
 * Interrupts callbacks, these are used by assembler files
 * sw_interrupts.s and hw_interrupts.s
 */
int_handler_t int_table[MAX_INT];

/*
 * Exceptions callbacks, these are used by assembler file
 * exceptions.s
 */
exc_handler_t exc_table[MAX_EXC];

static void default_exception_handler(void)
{
	fprintf(stderr, "### Default exception handler.\n");
}

static BOOL default_int_cb(void)
{
	fprintf(stderr, "### Default interrupt handler.\n");
	return (TRUE);
}

/*
 * Exception 0 - divide by 0
 */
static void exc0_handler(void)
{
	fprintf(stderr, "### Exception 0: division by 0.\n");
	abort();
}

/*
 * Exception 6 - Invalid opcode
 */
static void exc6_handler(void)
{
	fprintf(stderr, "### Exception 6: invalid opcode.\n");
	abort();
}

/*
 * Exception 8 - Double fault
 */
static void exc8_handler(void)
{
	fprintf(stderr, "### Exception 8: double fault.\n");
	abort();
}

/*
 * Exception 11 - Segment not present
 */
static void exc11_handler(void)
{
	fprintf(stderr, "### Exception 11: segment not present (segmentation fault).\n");
	abort();
}

/*
 * Exception 12 - Segment not present
 */
static void exc12_handler(void)
{
	fprintf(stderr, "### Exception 12: stack fault.\n");
	abort();
}

/*
 * Exception 13 - General protection exception
 */
static void exc13_handler(void)
{
	fprintf(stderr, "### Exception 13: general protection.\n");
	abort();
}

void idt_init()
{
	unsigned i;

	for (i = 0; i < NELEMENTS(int_table); i++) {
		int_table[i] = default_int_cb;
	}
	for (i = 0; i < NELEMENTS(exc_table); i++) {
		exc_table[i] = default_exception_handler;
	}

	/*
	 * Assign some exception handlers
	 */
	exc_table[0] = exc0_handler;
	exc_table[6] = exc6_handler;
	exc_table[8] = exc8_handler;
	exc_table[11] = exc11_handler;
	exc_table[12] = exc12_handler;
	exc_table[13] = exc13_handler;

	i = 0;

	/*
	 * First 32 vectors are reserved for exceptions and
	 * processors interrupts (0 to 31).
	 * Vectors from 32 to 47 are in use for hardware interrupts,
	 * coming from i8259 or equivalent.
	 * Vectors from 48 to 63 are in use for software interrupts.
	 * Vectors from 64 to 95 are in use for hardware interrupts
	 * from other sources.
	 */

	/* Exceptions from 0 to 19 */
	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc00);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc00 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc01);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc01 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc02);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc02 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc03);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc03 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc04);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc04 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc05);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc05 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc06);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc06 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc07);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc07 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc08);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc08 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc09);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc09 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc10);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc10 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc11);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc11 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc12);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc12 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc13);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc13 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc14);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc14 >> 16);

	/* Exceptions from 16 to 19 */
	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc16);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc16 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc17);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc17 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc18);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc18 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) exc19);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) exc19 >> 16);

	while (i < 32) {
		idt_table[i++].flags &= ~0x8000;
	}

	/*
	 * IRQ from 0 to 15 matches vectors from 32 to 47 and are dedicated
	 * to i8259 hardware
	 */
	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint00);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) hwint00 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint01);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) hwint01 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint02);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) hwint02 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint03);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) hwint03 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint04);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) hwint04 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint05);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) hwint05 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint06);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) hwint06 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint07);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) hwint07 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint08);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) hwint08 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint09);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) hwint09 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint10);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) hwint10 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint11);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) hwint11 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint12);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) hwint12 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint13);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) hwint13 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint14);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) hwint14 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) hwint15);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i].offset_hi = (uint16_t) ((intptr_t) hwint15 >> 16);

	/* Software interrupts 16 to 31 matches vectors from 48 to 63 */
	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint00);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint00 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint01);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint01 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint02);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint02 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint03);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint03 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint04);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint04 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint05);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint05 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint06);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint06 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint07);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint07 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint08);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint08 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint09);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint09 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint10);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint10 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint11);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint11 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint12);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint12 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint13);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint13 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint14);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint14 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint15);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint15 >> 16);

	/* Hardware interrupts 32 to 63 matches vectors from 64 to 95 */
	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint16);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint16 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint17);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint17 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint18);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint18 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint19);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint19 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint20);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint20 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint21);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint21 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint22);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint22 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint23);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint23 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint24);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint24 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint25);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint25 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint26);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint26 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint27);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint27 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint28);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint28 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint29);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint29 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint30);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint30 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint31);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint31 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint32);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint32 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint33);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint33 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint34);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint34 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint35);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint35 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint36);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint36 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint37);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint37 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint38);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint38 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint39);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint39 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint40);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint40 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint41);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint41 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint42);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint42 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint43);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint43 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint44);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint44 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint45);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint45 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint46);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint46 >> 16);

	idt_table[i].offset_low = (uint16_t) ((intptr_t) swint47);
	idt_table[i].segment = 8;
	idt_table[i].flags = FLAGS;
	idt_table[i++].offset_hi = (uint16_t) ((intptr_t) swint47 >> 16);

	/*
	 * set remaining vectors as unused
	 */
	for (; i < 256; i++) {
		idt_table[i].flags &= ~0x8000;
	}
}

void enable_int(unsigned intno)
{
	if (intno < NR_IRQ_VECTORS) {
		enable_irq(intno);
	}
}

void disable_int(unsigned intno)
{
	if (intno < NR_IRQ_VECTORS) {
		disable_irq(intno);
	}
}

int add_int_cb(int_handler_t cb, unsigned intno)
{
	if (intno >= MAX_INT)
		return (EINVAL);

	if (int_table[intno] && (int_table[intno] != default_int_cb)) {
		return EINVAL;
	}
	int_table[intno] = cb;

	return (EOK);
}

int del_int_cb(unsigned intno)
{
	if (intno >= MAX_INT)
		return EINVAL;

	if (int_table[intno]) {
		int_table[intno] = default_int_cb;
	}

	return (EOK);
}

int add_exc_cb(exc_handler_t cb, unsigned exc)
{
	if (exc >= MAX_EXC)
		return (EINVAL);

	if (exc_table[exc] && (exc_table[exc] != default_exception_handler)) {
		return EINVAL;
	}
	exc_table[exc] = cb;

	return (EOK);
}

int del_exc_cb(unsigned exc)
{
	if (exc >= MAX_INT)
		return EINVAL;

	if (exc_table[exc]) {
		exc_table[exc] = default_exception_handler;
	}

	return (EOK);
}
