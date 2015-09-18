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

.file "ints_equ.s"

/* 8259A interrupt controller ports. */
.equ	INT_CTL,        0x20
.equ	INT_CTLMASK,	0x21
.equ	INT2_CTL,	0xA0
.equ	INT2_CTLMASK,	0xA1
.equ	ENABLE,		0x20
.equ    SPEC_EOI,       0x60
.equ    CASCADE_IRQ,    0x02

