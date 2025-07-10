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

#include <string.h>
#include <stdio.h>
#include <diegos/interrupts.h>
#include <processor/ia32.h>
#include <processor/apic.h>
#include "../../kernel/platform_include.h"
#include "ia32_private.h"

#define MASK1   (FPU | MMX | FXSR | SSE | SSE2)
#define MASK2   (SSE3 | SSSE3 | SSE4_1 | SSE4_2)

static const char *families[] = {
	"80486 or AMD/Cyrix 5x86",
	"Pentium or Nx586 or M1 or K5/K6",
	"Pentium II/III/4/M or Core 2",
	"Itanium"
};

static char cpu_signature[768];

/*
 * ECX and EDX contents if provided by CPUID , EAX=1
 */
static unsigned cpu_caps[2] = { 0, 0 };

const char *processor_init()
{
	struct cpuid_data info;
	unsigned cpuid_level, excpuid_level, position, i;
	short model, family;

	execute_cpuid(&info, 0);
	/*
	 * Copy the string "GenuineIntel" or any other...
	 */
	memset(cpu_signature, 0, sizeof(cpu_signature));
	memcpy(cpu_signature, &info.word[EBX], 4);
	memcpy(cpu_signature + 4, &info.word[EDX], 4);
	memcpy(cpu_signature + 8, &info.word[ECX], 4);
	cpuid_level = info.word[EAX];

	if (cpuid_level > 0) {
		/*
		 * We can get Model, Family, Stepping
		 */
		execute_cpuid(&info, 1);
		cpu_caps[0] = info.word[ECX];
		cpu_caps[1] = info.word[EDX];
		family = (info.word[EAX] >> 8) & 0xF;
		if (family == 0xF)
			family += (info.word[EAX] >> 16) & 0xF;

		model = (info.word[EAX] >> 4) & 0xF;
		if ((family == 0x6) || (family == 0xF))
			model += (info.word[EAX] >> 20) & 0xFF;

		if ((family > 3) && (family < 8)) {
			cpu_signature[12] = ' ';
			strncpy(cpu_signature + 13,
				families[family - 4], sizeof(cpu_signature) - 13);
		}

		strcat(cpu_signature, "\n(");
		if (info.word[EDX] & FPU) {
			strcat(cpu_signature, " FPU");
		}
		if (info.word[EDX] & CX8) {
			strcat(cpu_signature, " CMPXCHG8");
		}
		if (info.word[EDX] & TSC) {
			strcat(cpu_signature, " TSC");
		}
		if (info.word[EDX] & APIC) {
			strcat(cpu_signature, " APIC");
		}
		if (info.word[EDX] & MTRR) {
			strcat(cpu_signature, " MTRR");
		}
		if (info.word[EDX] & PSN) {
			strcat(cpu_signature, " PSN");
		}
		if (info.word[EDX] & MMX) {
			strcat(cpu_signature, " MMX");
		}
		if (info.word[EDX] & FXSR) {
			strcat(cpu_signature, " FXSR");
		}
		if (info.word[EDX] & SSE) {
			strcat(cpu_signature, " SSE");
		}
		if (info.word[EDX] & SSE2) {
			strcat(cpu_signature, " SSE2");
		}

		if (info.word[ECX] & SSE3) {
			strcat(cpu_signature, " SSE3");
		}
		if (info.word[ECX] & SSSE3) {
			strcat(cpu_signature, " SSSE3");
		}
		if (info.word[ECX] & SSE4_1) {
			strcat(cpu_signature, " SSE4.1");
		}
		if (info.word[ECX] & SSE4_2) {
			strcat(cpu_signature, " SSE4.2");
		}
		if (info.word[ECX] & X2APIC) {
			strcat(cpu_signature, " X2APIC");
		}
		if (info.word[ECX] & XSAVE) {
			strcat(cpu_signature, " XSAVE");
		}
		strcat(cpu_signature, " ) ");
	}

	execute_cpuid(&info, 0x80000000UL);
	excpuid_level = info.word[EAX];

	if (excpuid_level > 0x80000003UL) {
		/*
		 * Extended CPU Info are supported!
		 */
		strcat(cpu_signature, "\n");
		position = strlen(cpu_signature);
		execute_cpuid(&info, 0x80000002);
		for (i = 0; i < 4; i++, position += 4)
			memcpy(cpu_signature + position, info.word + i, 4);
		execute_cpuid(&info, 0x80000003);
		for (i = 0; i < 4; i++, position += 4)
			memcpy(cpu_signature + position, info.word + i, 4);
		execute_cpuid(&info, 0x80000004);
		for (i = 0; i < 4; i++, position += 4)
			memcpy(cpu_signature + position, info.word + i, 4);
	}

	if (cpu_caps[1] & MTRR)
		init_mtrr();

	if (cpu_caps[1] & APIC)
		init_apic();

	idt_init();

#if defined(ENABLE_SIMD) || defined(ENABLE_FP)
	if ((info.word[EDX] & (SSE | SSE2)) || (info.word[ECX] & (SSE3 | SSSE3 | SSE4_1 | SSE4_2))) {
		init_simd();
	} else if (info.word[EDX] & (FPU | MMX)) {
		init_fp();
	}
	add_exc_cb(exc_handler_fp, 7);
#endif

	return (cpu_signature);
}

int cpu_check_capability(int capset, int capability)
{
	if (capset < 0 || capset > 1)
		return -1;

	return (cpu_caps[capset] & capability) ? 1 : 0;
}
