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

#ifndef _IA32_H_
#define _IA32_H_

/*
 * Values to be used with cpu_check_capability, capset = 1
 */
#define FPU     (1 << 0)
#define TSC	(1 << 5)
#define CX8	(1 << 8)
#define APIC	(1 << 9)
#define MTRR	(1 << 12)
#define PSN	(1 << 18)
#define MMX     (1 << 23)
#define FXSR    (1 << 24)
#define SSE     (1 << 25)
#define SSE2    (1 << 26)

/*
 * Values to be used with cpu_check_capability, capset = 0
 */
#define SSE3    (1 << 0)
#define SSSE3   (1 << 9)
#define SSE4_1  (1 << 19)
#define SSE4_2  (1 << 20)
#define X2APIC	(1 << 21)
#define XSAVE	(1 << 26)

/*
 * Check the availability of one or more capability on the running CPU.
 * The capability sets are 2 to keep consistency between Intel architecture
 * and documentation and DiegOS implementation.
 *
 * PARAMETERS IN
 * int capset		- which set to be checked
 * int capability	- the capability bit(s)
 *
 * RETURNS
 *
 * -1 in case of error
 *  1 if the capabilities are supported
 *  0 if the capabilities are not supported
 */
int cpu_check_capability(int capset, int capability);

void read_msr(void *buffer, unsigned msrreg);
void write_msr(void *buffer, unsigned msrreg);

/*
 * The 4 unsigned words are linked to the following registers
 *
 * EAX
 * EBX
 * ECX
 * EDX
 *
 * We do not bother here about sizes, integers etc. as this file
 * is totally platform dependent.
 */

#define	EAX	0
#define EBX	1
#define ECX	2
#define EDX	3

struct cpuid_data {
	unsigned word[4];
};

void execute_cpuid(struct cpuid_data *data, unsigned infotype);

#endif
