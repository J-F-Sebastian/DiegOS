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

.file	"sw_interrupts.s"
.text
.globl swint00
.globl swint01
.globl swint02
.globl swint03
.globl swint04
.globl swint05
.globl swint06
.globl swint07
.globl swint08
.globl swint09
.globl swint10
.globl swint11
.globl swint12
.globl swint13
.globl swint14
.globl swint15
.type  swint00, @function
.type  swint01, @function
.type  swint02, @function
.type  swint03, @function
.type  swint04, @function
.type  swint05, @function
.type  swint06, @function
.type  swint07, @function
.type  swint08, @function
.type  swint09, @function
.type  swint10, @function
.type  swint11, @function
.type  swint12, @function
.type  swint13, @function
.type  swint14, @function
.type  swint15, @function

.macro swint int
cld				/* save interrupted process state */
pusha				/* stack pointer is not changed */
call	*int_table + 4*\int	/* eax = (*irq_table[int])()      */
popa
iret				/* restart (another) process      */
.endm

.align 16
swint00:
swint 0

.align 16
swint01:
swint 1

.align 16
swint02:
swint 2

.align 16
swint03:
swint 3

.align 16
swint04:
swint 4

.align 16
swint05:
swint 5

.align 16
swint06:
swint 6

.align 16
swint07:
swint 7

.align 16
swint08:
swint 8

.align 16
swint09:
swint 9

.align 16
swint10:
swint 10

.align 16
swint11:
swint 11

.align 16
swint12:
swint 12

.align 16
swint13:
swint 13

.align 16
swint14:
swint 14

.align 16
swint15:
swint 15

