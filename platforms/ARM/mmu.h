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

#ifndef _ARM_MMU_
#define _ARM_MMU_

/*
 * Enables the MMU.
 * Before calling this function be sure to have set up VMSA by calling
 * arm_mmu_config(), or virtual-to-physical memory mapping will fail.
 */
void arm_mmu_enable(void);

/*
 * Disables the MMU.
 * This operation has an impact on caching. Check ARM documentation for details.
 */
void arm_mmu_disable(void);

/*
 * Configures the MMU translation table entries.
 * Only section descriptors are configured, TLBs are not supported
 * by DiegOS.
 * mbstart is the starting address (i.e. row in the translation_table) in MBytes,
 * mbsize the size of the area to be configured (i.e. the number of rows to be
 * written to), phy is the physical starting address in MBytes of the memory
 * mapped by the entries of the MMU, while caching can have 4 values
 *
 * 0 Noncacheable, nonbufferable
 * 1 Noncacheable, bufferable
 * 2 Write-through
 * 3 Write-back
 *
 * PARAMETERS IN
 * unsigned mbstart - the starting address in MByte to be configured (virtual address space)
 * unsigned mbsize - the number of MBytes to be configured (virtual address space)
 * unsigned phy - the physical address aligned to 1 MByte to be mapped to mbstart
 * unsigned caching - the caching policy to be assigned to the physical memory
 *
 * RETURNS
 * EINVAL if the parameters are invalid
 * EOK in any other case
 */
int arm_mmu_config(unsigned mbstart, unsigned mbsize, unsigned phy, unsigned caching);

#endif
