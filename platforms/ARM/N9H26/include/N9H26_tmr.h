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

#ifndef _N9H26_TMR_H_
#define _N9H26_TMR_H_

#include <types_common.h>

/*
 * All registers are 32 bit wide
 */

#define N9H26TMR_BASE0 0xB8002000UL
#define N9H26TMR_BASE1 0xB8006000UL

enum N9H26TMR {
	/*
	 * These applies to BASE0 only
	 */
	TCSR0,
	TCSR1,
	TICR0,
	TICR1,
	TDR0,
	TDR1,
	TISR,
	WTCR,
	/*
	 * These applies to BASE1 only
	 */
	TCSR2 = TCSR0,
	TCSR3 = TCSR1,
	TICR2 = TICR0,
	TICR3 = TICR1,
	TDR2 = TDR0,
	TDR3 = TDR1,
	TISR2 = TISR,
};

/* Timer Registers */
//#define REG_TCSR0		(TMR_BA+0x00)	/* Timer Control and Status Register 0 */
#define TCSR_DBGACK_EN		BIT32(31)	// ICE debug mode acknowledge enable
#define CEN			BIT32(30)	// Counter Enable
#define TCSR_IE			BIT32(29)	// Interrupt Enable
#define TCSR_MODE		BITMASK32(28,27)// Timer Operating Mode
#define CRST			BIT32(26)	// Counter Reset
#define CACT			BIT32(25)	// Timer is in Active
#define TDR_EN			BIT32(16)	// Timer Data Register update enable
#define TCSR_PRESCALE		BITMASK32(7,0)	// Prescale
						//
//#define REG_TCSR1		(TMR_BA+0x04)	/* Timer Control and Status Register 1 */
//#define REG_TICR0		(TMR_BA+0x08)	/* Timer Initial Control Register 0 */
//#define REG_TICR1		(TMR_BA+0x0C)	/* Timer Initial Control Register 1 */
//#define REG_TDR0		(TMR_BA+0x10)	/* Timer Data Register 0 */
//#define REG_TDR1		(TMR_BA+0x14)	/* Timer Data Register 1 */

//#define REG_TISR		(TMR_BA+0x18)	/* Timer Interrupt Status Register */
#define TIF1			BIT32(1)	// Timer Interrupt Flag 1
#define TIF0			BIT32(0)	// Timer Interrupt Flag 0

//#define REG_WTCR		(TMR_BA+0x1C)	/* Watchdog Timer Control Register */
#define WTCLK			BIT32(10)	// Watchdog Timer Clock
#define WTCR_DBGACK_EN		BIT32(9)	// ICE debug mode acknowledge enable
#define WTTME			BIT32(8)	// Watchdog Timer Test Mode Enable
#define WTE			BIT32(7)	// Watchdog Timer Test Mode Enable
#define WTIE			BIT32(6)	// Watchdog Timer Enable
#define WTIS			BITMASK32(5,4)	// Watchdog Timer Interval Select
#define WTIF			BIT32(3)	// Watchdog Timer Interrupt Flag
#define WTRF			BIT32(2)	// Watchdog Timer Reset Flag
#define WTRE			BIT32(1)	// Watchdog Timer Reset Enable
#define WTR			BIT32(0)	// Watchdog Timer Reset

#endif
