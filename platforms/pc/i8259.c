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

#include <types_common.h>
#include <stdio.h>

#include <processor/ports.h>
#include <platform/i8259.h>

#define ICW1_AT         0x11	/* edge triggered, cascade, need ICW4 */
#define ICW4_AT         0x01	/* not SFNM, not buffered, normal EOI, 8086 */

/*
 * Edge triggered interrupts are controlled by ELCR1/2 registers in PCI/ISA
 * bridges. This stuff should go somewhere under a PCI Bridge driver.
 */

void i8259_init()
{
	/* ICW1 for master */
	out_byte(INT_CTL, ICW1_AT);
	/* ICW2 for master */
	out_byte(INT_CTLMASK, IRQ0_VECTOR);
	/* ICW3 tells slaves */
	out_byte(INT_CTLMASK, (1 << CASCADE_IRQ));
	out_byte(INT_CTLMASK, ICW4_AT);
	out_byte(INT_CTLMASK, ~(1 << CASCADE_IRQ));	/* IRQ 0-7 mask */

	out_byte(INT2_CTL, ICW1_AT);
	out_byte(INT2_CTLMASK, IRQ8_VECTOR);
	/* ICW2 for slave */
	out_byte(INT2_CTLMASK, CASCADE_IRQ);	/* ICW3 is slave nr */
	out_byte(INT2_CTLMASK, ICW4_AT);
	out_byte(INT2_CTLMASK, ~0);	/* IRQ 8-15 mask */
}
