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
#include <platform/i8259.h>

#include "ia32_private.h"
#include "ints_private.h"

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
 * To make use of them you need to increase MAX_INT defines and you need
 * to initialize the IDT handlers in idt_init().
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

/*
 * Flags are
 *
 * F E D C B A 9 8 | 7 6 5 4 3 2 1 0
 *-----------------+----------------
 *   D D           |
 * P P P 0 D 1 1 0 | 0 0 0 R R R R R
 *   L L           |
 *-----------------+----------------
 *
 * 1 0 0 0 1 1 1 0 | 0 0 0 0 0 0 0 0
 * |  |    |
 *  \  \    \
 *   \  \    \
 *    \  \    +- 32 bit segment
 *     \  +- Priority 0
 *      +- Present
 */
#define PRESENT (0x8E00)
#define MISSING (0x0E00)

struct interrupt_gate {
	uint16_t offset_low;
	uint16_t segment;
	uint16_t flags;
	uint16_t offset_hi;
};

/*
 * IDT interrupt table handlers, see boot32.s for details
 */
static struct interrupt_gate *idt_table = (struct interrupt_gate *)(0x600UL);

/*
 * Interrupts callbacks, these are used by assembler files
 * sw_interrupts.s and hw_interrupts.s
 * The serviced vectors start at number 32.
 */
int_handler_t int_table[MAX_INT];

/*
 * Exceptions callbacks, these are used by assembler file
 * exceptions.s
 * The services exception vectors start at number 0.
 */
exc_handler_t exc_table[MAX_EXC];

/*
 * Helper functions used to set a handler in the IDT.
 */
static inline void set_idt_table(unsigned idx, intptr_t hdlr)
{
	idt_table[idx].offset_low = (uint16_t) (hdlr);
	idt_table[idx].segment = 8;
	idt_table[idx].flags = PRESENT;
	idt_table[idx].offset_hi = (uint16_t) (hdlr >> 16);
}

static inline void set_idt_table_void(unsigned idx)
{
	idt_table[idx].flags = MISSING;
}

/*
 * Default handlers, these are invoked by the IDT handlers
 * in case an interrupt is active, triggered, but no handler
 * was set to do some work.
 * These avoid checking for NULL.
 */
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

	/*
	 * Initialize all handlers to defaults.
	 */
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

	/*
	 * First 32 vectors are reserved for exceptions and
	 * processors interrupts (0 to 31).
	 * Vectors from 32 to 47 are in use for hardware interrupts,
	 * coming from i8259 or equivalent.
	 * Vectors from 48 to 95 are in use for user-defined interrupts.
	 */

	/*
	 * Exceptions from 0 to 19
	 */
	set_idt_table(0, (intptr_t) exc00);
	set_idt_table(1, (intptr_t) exc01);
	set_idt_table(2, (intptr_t) exc02);
	set_idt_table(3, (intptr_t) exc03);
	set_idt_table(4, (intptr_t) exc04);
	set_idt_table(5, (intptr_t) exc05);
	set_idt_table(6, (intptr_t) exc06);
	set_idt_table(7, (intptr_t) exc07);
	set_idt_table(8, (intptr_t) exc08);
	set_idt_table(9, (intptr_t) exc09);
	set_idt_table(10, (intptr_t) exc10);
	set_idt_table(11, (intptr_t) exc11);
	set_idt_table(12, (intptr_t) exc12);
	set_idt_table(13, (intptr_t) exc13);
	set_idt_table(14, (intptr_t) exc14);
	/*
	 * Skip exception 15
	 */
	set_idt_table_void(15);
	/*
	 * Exceptions from 16 to 19
	 */
	set_idt_table(16, (intptr_t) exc16);
	set_idt_table(17, (intptr_t) exc17);
	set_idt_table(18, (intptr_t) exc18);
	set_idt_table(19, (intptr_t) exc19);
	/*
	 * Skip exceptions from 20 to 31
	 */
	set_idt_table_void(20);
	set_idt_table_void(21);
	set_idt_table_void(22);
	set_idt_table_void(23);
	set_idt_table_void(24);
	set_idt_table_void(25);
	set_idt_table_void(26);
	set_idt_table_void(27);
	set_idt_table_void(28);
	set_idt_table_void(29);
	set_idt_table_void(30);
	set_idt_table_void(31);
	/*
	 * IRQ from 0 to 15 matches vectors from 32 to 47 and are dedicated
	 * to i8259 hardware
	 */
	set_idt_table(32, (intptr_t) hwint32);
	set_idt_table(33, (intptr_t) hwint33);
	set_idt_table(34, (intptr_t) hwint34);
	set_idt_table(35, (intptr_t) hwint35);
	set_idt_table(36, (intptr_t) hwint36);
	set_idt_table(37, (intptr_t) hwint37);
	set_idt_table(38, (intptr_t) hwint38);
	set_idt_table(39, (intptr_t) hwint39);
	set_idt_table(40, (intptr_t) hwint40);
	set_idt_table(41, (intptr_t) hwint41);
	set_idt_table(42, (intptr_t) hwint42);
	set_idt_table(43, (intptr_t) hwint43);
	set_idt_table(44, (intptr_t) hwint44);
	set_idt_table(45, (intptr_t) hwint45);
	set_idt_table(46, (intptr_t) hwint46);
	set_idt_table(47, (intptr_t) hwint47);
	/*
	 * Software interrupt vectors from 48 to 63
	 */
	set_idt_table(48, (intptr_t) swint48);
	set_idt_table(49, (intptr_t) swint49);
	set_idt_table(50, (intptr_t) swint50);
	set_idt_table(51, (intptr_t) swint51);
	set_idt_table(52, (intptr_t) swint52);
	set_idt_table(53, (intptr_t) swint53);
	set_idt_table(54, (intptr_t) swint54);
	set_idt_table(55, (intptr_t) swint55);
	set_idt_table(56, (intptr_t) swint56);
	set_idt_table(57, (intptr_t) swint57);
	set_idt_table(58, (intptr_t) swint58);
	set_idt_table(59, (intptr_t) swint59);
	set_idt_table(60, (intptr_t) swint60);
	set_idt_table(61, (intptr_t) swint61);
	set_idt_table(62, (intptr_t) swint62);
	set_idt_table(63, (intptr_t) swint63);
	/*
	 * Hardware interrupt vectors from 64 to 95
	 */
	set_idt_table(64, (intptr_t) swint64);
	set_idt_table(65, (intptr_t) swint65);
	set_idt_table(66, (intptr_t) swint66);
	set_idt_table(67, (intptr_t) swint67);
	set_idt_table(68, (intptr_t) swint68);
	set_idt_table(69, (intptr_t) swint69);
	set_idt_table(70, (intptr_t) swint70);
	set_idt_table(71, (intptr_t) swint71);
	set_idt_table(72, (intptr_t) swint72);
	set_idt_table(73, (intptr_t) swint73);
	set_idt_table(74, (intptr_t) swint74);
	set_idt_table(75, (intptr_t) swint75);
	set_idt_table(76, (intptr_t) swint76);
	set_idt_table(77, (intptr_t) swint77);
	set_idt_table(78, (intptr_t) swint78);
	set_idt_table(79, (intptr_t) swint79);
	set_idt_table(80, (intptr_t) swint80);
	set_idt_table(81, (intptr_t) swint81);
	set_idt_table(82, (intptr_t) swint82);
	set_idt_table(83, (intptr_t) swint83);
	set_idt_table(84, (intptr_t) swint84);
	set_idt_table(85, (intptr_t) swint85);
	set_idt_table(86, (intptr_t) swint86);
	set_idt_table(87, (intptr_t) swint87);
	set_idt_table(88, (intptr_t) swint88);
	set_idt_table(89, (intptr_t) swint89);
	set_idt_table(90, (intptr_t) swint90);
	set_idt_table(91, (intptr_t) swint91);
	set_idt_table(92, (intptr_t) swint92);
	set_idt_table(93, (intptr_t) swint93);
	set_idt_table(94, (intptr_t) swint94);
	set_idt_table(95, (intptr_t) swint95);
	/*
	 * set remaining vectors as unused
	 */
	for (i = 96; i < 256; i++) {
		set_idt_table_void(i);
	}
}

void enable_int(unsigned intno)
{
	if (intno > 31) {
		intno -= 32;
		if (intno < NR_IRQ_VECTORS) {
			enable_irq(intno);
		}
	}
}

void disable_int(unsigned intno)
{
	if (intno > 31) {
		intno -= 32;
		if (intno < NR_IRQ_VECTORS) {
			disable_irq(intno);
		}
	}
}

int add_int_cb(int_handler_t cb, unsigned intno)
{
	if (intno < 32)
		return (EINVAL);

	intno -= 32;
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
	if (intno < 32)
		return (EINVAL);

	intno -= 32;
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
