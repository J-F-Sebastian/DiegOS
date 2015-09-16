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

#ifndef _IA32_PRIVATE_H_
#define _IA32_PRIVATE_H_

void execute_cpuid(void *data, unsigned infotype);

void idt_init(void);

/*
 * externs for hw_interrupts.s and
 * sw_interrupts.s and exceptions.s
 */
extern void hwint00(void);
extern void hwint01(void);
extern void hwint02(void);
extern void hwint03(void);
extern void hwint04(void);
extern void hwint05(void);
extern void hwint06(void);
extern void hwint07(void);
extern void hwint08(void);
extern void hwint09(void);
extern void hwint10(void);
extern void hwint11(void);
extern void hwint12(void);
extern void hwint13(void);
extern void hwint14(void);
extern void hwint15(void);

extern void swint00(void);
extern void swint01(void);
extern void swint02(void);
extern void swint03(void);
extern void swint04(void);
extern void swint05(void);
extern void swint06(void);
extern void swint07(void);
extern void swint08(void);
extern void swint09(void);
extern void swint10(void);
extern void swint11(void);
extern void swint12(void);
extern void swint13(void);
extern void swint14(void);
extern void swint15(void);

extern void exc00(void);
extern void exc01(void);
extern void exc02(void);
extern void exc03(void);
extern void exc04(void);
extern void exc05(void);
extern void exc06(void);
extern void exc07(void);
extern void exc08(void);
extern void exc09(void);
extern void exc10(void);
extern void exc11(void);
extern void exc12(void);
extern void exc13(void);
extern void exc14(void);
extern void exc16(void);
extern void exc17(void);
extern void exc18(void);
extern void exc19(void);

#endif

