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

#ifndef _CX82310_GPIO_H_
#define _CX82310_GPIO_H_

/*
 * All registers are 32 bit wide
 */

#define CX82310GPIO_BASE 0x3500A0UL

enum CX82310GPIO {
	GPIO_ISM1,
	GPIO_ISM2,
	GPIO_ISM3,
	GPIO_OPT = GPIO_ISM3 + 2,
	GPIO_OE1,
	GPIO_OE2,
	GPIO_OE3,
	GPIO_DATA_IN1,
	GPIO_DATA_IN2,
	GPIO_DATA_IN3,
	GPIO_DATA_OUT1,
	GPIO_DATA_OUT2,
	GPIO_DATA_OUT3,
	GPIO_ISR1,
	GPIO_ISR2,
	GPIO_ISR3,
	GPIO_IER1,
	GPIO_IER2,
	GPIO_IER3,
	GPIO_IPC1,
	GPIO_IPC2,
	GPIO_IPC3,
	GPIO_IE1,
	GPIO_IE2,
	GPIO_IE3
};

#endif
