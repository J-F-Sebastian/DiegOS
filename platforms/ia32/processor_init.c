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

#include "../../kernel/platform_include.h"
#include "ia32_private.h"

static const char *families[] = {
    "80486 or AMD/Cyrix 5x86",
    "Pentium or Nx586 or M1 or K5/K6",
    "Pentium II/III/4/M or Core 2",
    "Itanium"
};

static char cpu_signature[80];

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
                    families[info.word[0] - 4],
                    sizeof(cpu_signature) - 13);
        }
    }

    idt_init();

    return (cpu_signature);
}

