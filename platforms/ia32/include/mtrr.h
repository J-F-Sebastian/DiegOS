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

#ifndef _MTRR_H_
#define _MTRR_H_

#include <stdint.h>
#include <types_common.h>

enum {
	/* 0x00 Uncacheable */
	MTRR_TYPE_UC,
	/* 0x01 Write Combining */
	MTRR_TYPE_WC,
	/* 0x04 Write-Through */
	MTRR_TYPE_WT = 4,
	/* 0x05 Write-protected */
	MTRR_TYPE_WP,
	/* 0x06 Write-Back */
	MTRR_TYPE_WB
};

BOOL mtrr_configure(uintptr_t base, uintptr_t size, int cache_type);

void mtrr_dump(void);

#endif
