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

#ifndef _CX82310_DMAC_H_
#define _CX82310_DMAC_H_

/*
 * All registers are 32 bit wide
 */

#define CX82310DMAC_BASE 0x300000UL

enum CX82310DMAC {
	DMAC_1_Ptr1,
	DMAC_2_Ptr2,
	DMAC_3_Ptr1,
	DMAC_4_Ptr1,
	DMAC_5_Ptr1,
	DMAC_6_Ptr1,
	DMAC_7_Ptr1,
	DMAC_8_Ptr1,
	DMAC_9_Ptr1,
	DMAC_10_Ptr1,
	DMAC_11_Ptr1,
	/* Reserved */
	DMAC_1_Ptr2 = DMAC_11_Ptr1 + 2,
	DMAC_2_Ptr2,
	DMAC_3_Ptr2,
	DMAC_4_Ptr2,
	DMAC_5_Ptr2,
	/* Reserved */
	DMAC_1_Cnt1 = DMAC_5_Ptr2 + 8,
	DMAC_2_Cnt1,
	DMAC_3_Cnt1,
	DMAC_4_Cnt1,
	DMAC_5_Cnt1,
	DMAC_6_Cnt1,
	/* Reserved */
	DMAC_9_Cnt1 = DMAC_6_Cnt1 + 3,
	DMAC_10_Cnt1,
	DMAC_11_Cnt1,
	/* Reserved */
	DMAC_2_Cnt2 = DMAC_11_Cnt1 + 3,
	/* Reserved */
	DMAC_4_Cnt2 = DMAC_2_Cnt2 + 2,
	/* Reserved */
	DMAC_12_Ptr1 = DMAC_4_Cnt2 + 25,
	DMAC_13_Ptr1,
	/* Reserved */
	DMAC_12_Cnt1 = DMAC_13_Ptr1 + 3,
	DMAC_13_Cnt1
	/* Reserved */
};

#endif
