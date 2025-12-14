/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2025 Diego Gallizioli
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

static const char *architectures[] = {
	"Reserved",
	"ARMv4",
	"ARMv4T",
	"ARMv5",
	"ARMv5T",
	"ARMv5TE",
	"ARMv5TEJ",
	"ARMv6"
};

void arm_processor_detect(char *cpu_signature)
{
	unsigned idcode, ctype, temp1, temp2, temp3;

	asm volatile ("MRC p15, 0, %0, c0, c0, 0\n"
		      "MRC p15, 0, %1, c0, c0, 1\n":"=r" (idcode), "=r"(ctype));

	switch (idcode >> 24) {
	case 'A':
		strcat(cpu_signature, "ARM Limited ");
		break;

	case 'D':
		strcat(cpu_signature, "DEC ");
		break;

	case 'M':
		strcat(cpu_signature, "Motorola-Freescale ");
		break;

	case 'V':
		strcat(cpu_signature, "Marvell ");
		break;

	case 'i':
		strcat(cpu_signature, "Intel ");
		break;

	default:
		strcat(cpu_signature, "Unknown ");
		break;
	}

	if (((idcode >> 16) & 0xF) < NELEMENTS(architectures)) {
		strcat(cpu_signature, architectures[(idcode >> 16) & 0xF]);
	} else {
		strcat(cpu_signature, "Unknown ");
	}

	cpu_signature += strlen(cpu_signature);
	sprintf(cpu_signature, "%3X Rev %d\n", (idcode >> 4) & 0xFFF, idcode & 0xF);

	if ((ctype >> 24) & 0x1F) {
		strcat(cpu_signature, "Cache Write-Back ");
	} else {
		strcat(cpu_signature, "Cache Write-Through ");
	}

	if (((ctype >> 2) & 7) == 1) {
		strcat(cpu_signature, "No I Cache ");
	} else {
		cpu_signature += strlen(cpu_signature);
		temp1 = ctype & 0xFFF;
		if (temp1 & 0x4) {
			temp3 = 3 << ((temp1 >> 3) - 1);
			temp1 = 768 << ((temp1 >> 6) & 0xF);
		} else {
			temp3 = 1 << (temp1 >> 3);
			temp1 = 512 << ((temp1 >> 6) & 0xF);
		}
		temp2 = 8 << (ctype & 0x3);
		sprintf(cpu_signature, "I %dKB cl %d bytes %d-way", temp1, temp2, temp3);
		cpu_signature += strlen(cpu_signature);
	}

	if (((ctype >> 14) & 7) == 1) {
		strcat(cpu_signature, "No D Cache ");
	} else {
		cpu_signature += strlen(cpu_signature);
		temp1 = (ctype >> 12) & 0xFFF;
		temp2 = 8 << (temp1 & 0x3);
		if (temp1 & 0x4) {
			temp3 = 3 << ((temp1 >> 3) - 1);
			temp1 = 768 << ((temp1 >> 6) & 0xF);
		} else {
			temp3 = 1 << (temp1 >> 3);
			temp1 = 512 << ((temp1 >> 6) & 0xF);
		}
		sprintf(cpu_signature, "D %dKB cl %d bytes %d-way", temp1, temp2, temp3);
		cpu_signature += strlen(cpu_signature);
	}
}
