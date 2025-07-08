/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2022 Diego Gallizioli
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

#ifndef _APIC_H_
#define _APIC_H_

enum {
	/* 0000b */
	APIC_DIV_2,
	/* 0001b */
	APIC_DIV_4,
	/* 0010b */
	APIC_DIV_8,
	/* 0011b */
	APIC_DIV_16,
	/* 1000b */
	APIC_DIV_32 = 8,
	/* 1001b */
	APIC_DIV_64,
	/* 1010b */
	APIC_DIV_128,
	/* 1011b */
	APIC_DIV_1
};

BOOL is_apic_supported(void);

BOOL apic_configure(char oneshot, char enableint, char vector, char divisor);

BOOL apic_write_counter(unsigned count);

unsigned apic_read_counter(void);

void apic_write_eoi(void);

unsigned apic_read_version();

#endif
