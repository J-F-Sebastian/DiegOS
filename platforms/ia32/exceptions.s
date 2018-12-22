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

.file "exceptions.s"
.section .text

.globl exc00
.globl exc01
.globl exc02
.globl exc03
.globl exc04
.globl exc05
.globl exc06
.globl exc07
.globl exc08
.globl exc09
.globl exc10
.globl exc11
.globl exc12
.globl exc13
.globl exc14
.globl exc16
.globl exc17
.globl exc18
.globl exc19
.type	exc00, @function
.type	exc01, @function
.type	exc02, @function
.type	exc03, @function
.type	exc04, @function
.type	exc05, @function
.type	exc06, @function
.type	exc07, @function
.type	exc08, @function
.type	exc09, @function
.type	exc10, @function
.type	exc11, @function
.type	exc12, @function
.type	exc13, @function
.type	exc14, @function
.type	exc16, @function
.type	exc17, @function
.type	exc18, @function
.type	exc19, @function

.macro exc_master vect
pusha
call	*exc_table+4*\vect(,1)	/* eax = (*exc_table[vect])()  */
popa
iret				        /* reenable interrupts */
.endm

.align 64
exc00:
exc_master 0
exc01:
exc_master 1
exc02:
exc_master 2
exc03:
exc_master 3
exc04:
exc_master 4
exc05:
exc_master 5
exc06:
exc_master 6
exc07:
exc_master 7
exc08:
exc_master 8
exc09:
exc_master 9
exc10:
exc_master 10
exc11:
exc_master 11
exc12:
exc_master 12
exc13:
exc_master 13
exc14:
exc_master 14
exc16:
exc_master 16
exc17:
exc_master 17
exc18:
exc_master 18
exc19:
exc_master 19

