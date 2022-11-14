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

#ifndef _CX82310_TM_H_
#define _CX82310_TM_H_

/*
 * All registers are 32 bit wide
 */

#define CX82310TM_BASE 0x350020UL

enum CX82310TM {
	TM_Cnt1,
	TM_Cnt2,
	TM_Cnt3,
	TM_Cnt4,
	TM_Lmt1,
	TM_Lmt2,
	TM_Lmt3,
	TM_Lmt4,
};

#endif
