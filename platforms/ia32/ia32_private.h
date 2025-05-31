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

extern void set_ts(void);
extern void init_fp(void);
extern void init_simd(void);
void init_apic(void);
void init_mtrr(void);
void idt_init(void);

void exc_handler_fp(void);

/*
 * externs for hw_interrupts.s and
 * sw_interrupts.s and exceptions.s
 */
extern void hwint32(void);
extern void hwint33(void);
extern void hwint34(void);
extern void hwint35(void);
extern void hwint36(void);
extern void hwint37(void);
extern void hwint38(void);
extern void hwint39(void);
extern void hwint40(void);
extern void hwint41(void);
extern void hwint42(void);
extern void hwint43(void);
extern void hwint44(void);
extern void hwint45(void);
extern void hwint46(void);
extern void hwint47(void);

extern void swint48(void);
extern void swint49(void);
extern void swint50(void);
extern void swint51(void);
extern void swint52(void);
extern void swint53(void);
extern void swint54(void);
extern void swint55(void);
extern void swint56(void);
extern void swint57(void);
extern void swint58(void);
extern void swint59(void);
extern void swint60(void);
extern void swint61(void);
extern void swint62(void);
extern void swint63(void);
extern void swint64(void);
extern void swint65(void);
extern void swint66(void);
extern void swint67(void);
extern void swint68(void);
extern void swint69(void);
extern void swint70(void);
extern void swint71(void);
extern void swint72(void);
extern void swint73(void);
extern void swint74(void);
extern void swint75(void);
extern void swint76(void);
extern void swint77(void);
extern void swint78(void);
extern void swint79(void);
extern void swint80(void);
extern void swint81(void);
extern void swint82(void);
extern void swint83(void);
extern void swint84(void);
extern void swint85(void);
extern void swint86(void);
extern void swint87(void);
extern void swint88(void);
extern void swint89(void);
extern void swint90(void);
extern void swint91(void);
extern void swint92(void);
extern void swint93(void);
extern void swint94(void);
extern void swint95(void);

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
