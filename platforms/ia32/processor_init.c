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

#include <string.h>
#include <diegos/interrupts.h>
#include "../../kernel/platform_include.h"
#include "ia32_private.h"

#define FPU     (1 << 0)
#define MMX     (1 << 23)
#define FXSR    (1 << 24)
#define SSE     (1 << 25)
#define SSE2    (1 << 26)
#define MASK1   (FPU | MMX | FXSR | SSE | SSE2)

#define SSE3    (1 << 0)
#define SSSE3   (1 << 9)
#define SSE4_1  (1 << 19)
#define SSE4_2  (1 << 20)
#define MASK2   (SSE3 | SSSE3 | SSE4_1 | SSE4_2)

static const char *families[] = {
	"80486 or AMD/Cyrix 5x86",
	"Pentium or Nx586 or M1 or K5/K6",
	"Pentium II/III/4/M or Core 2",
	"Itanium"
};

static char cpu_signature[160];

struct cpuid_data {
	unsigned word[4];
};

const char *processor_init()
{
	struct cpuid_data info;
	unsigned cpuid_level;

	execute_cpuid(&info, 0);
	/*
	 * Copy the string "GenuineIntel" or any other...
	 */
	memset(cpu_signature, 0, sizeof(cpu_signature));
	memcpy(cpu_signature, &info.word[1], 4);
	memcpy(cpu_signature + 4, &info.word[3], 4);
	memcpy(cpu_signature + 8, &info.word[2], 4);
	cpuid_level = info.word[0];

	if (cpuid_level > 0) {
		/*
		 * We can get cacheline sizes too, and features
		 */
		execute_cpuid(&info, 1);
		info.word[0] >>= 8;
		info.word[0] &= 0xF;

		if ((info.word[0] > 3) && (info.word[0] < 8)) {
			cpu_signature[12] = ' ';
			strncpy(cpu_signature + 13,
				families[info.word[0] - 4], sizeof(cpu_signature) - 13);
		}

		if (info.word[3] & MASK1) {
			strcat(cpu_signature, "\n( ");
			if (info.word[3] & FPU) {
				strcat(cpu_signature, "FPU ");
			}
			if (info.word[3] & MMX) {
				strcat(cpu_signature, "MMX ");
			}
			if (info.word[3] & SSE) {
				strcat(cpu_signature, "SSE ");
			}
			if (info.word[3] & SSE2) {
				strcat(cpu_signature, "SSE2 ");
			}
		}
		if (info.word[2] & MASK2) {
			if (info.word[2] & SSE3) {
				strcat(cpu_signature, "SSE3 ");
			}
			if (info.word[2] & SSSE3) {
				strcat(cpu_signature, "SSSE3 ");
			}
			if (info.word[2] & SSE4_1) {
				strcat(cpu_signature, "SSE4.1 ");
			}
			if (info.word[2] & SSE4_2) {
				strcat(cpu_signature, "SSE4.2 ");
			}
		}
		strcat(cpu_signature, ")");
	}

	idt_init();

#if defined(ENABLE_SIMD) || defined(ENABLE_FP)
	if ((info.word[3] & (SSE | SSE2)) || (info.word[2] & (SSE3 | SSSE3 | SSE4_1 | SSE4_2))) {
		init_simd();
	} else if (info.word[3] & (FPU | MMX)) {
		init_fp();
	}
	add_exc_cb(exc_handler_fp, 7);
#endif

	return (cpu_signature);
}
