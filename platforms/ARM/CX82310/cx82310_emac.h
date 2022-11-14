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

#ifndef _CX82310_EMAC_H_
#define _CX82310_EMAC_H_

/*
 * All registers are 32 bit wide.
 * The registers are documented with "x" assigned to "1" and "2",
 * but the set is repeated twice at 2 different offsets, for brevity
 * the registers are enumerated only once.
 */

#define CX82310EMAC1_BASE 0x310000UL
#define CX82310EMAC2_BASE 0x320000UL

enum CX82310EMAC {
	E_DMA_x,
	E_NA_x,
	E_Stat_x,
	E_IE_x,
	E_LP_x,
	E_MII_x,
	ET_DMA_x,
};

#endif
