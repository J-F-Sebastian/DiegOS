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
#include "../../../kernel/platform_include.h"
#include "../mmu.h"
#include "../processor.h"

static char cpu_signature[768];

const char *processor_init()
{
	unsigned long ram;

	total_memory(&ram);
	/*
	 * actual RAM
	 */
	arm_mmu_config(0, ram, 0, 3);
	/*
	 * Valid address space mapping no hardware
	 */
	arm_mmu_config(ram, 2048 - ram, ram, 0);
	/*
	 * Shadow RAM
	 */
	arm_mmu_config(2048, 2048 + ram, 0, 0);
	/*
	 * Shadow memory mapping no hardware
	 */
	arm_mmu_config(2048 + ram, 2048 - ram, 2048 + ram, 0);

	arm_processor_detect(cpu_signature);

	return (cpu_signature);
}
