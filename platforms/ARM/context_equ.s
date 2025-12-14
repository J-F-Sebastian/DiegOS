/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2024 Diego Gallizioli
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

.file "context_equ.s"

# Layout of the initial stack of a thread
#
#  TOP OF THE STACK (NOT a valid pointer)
#-----------------------
#  R0          Register 0
#  R1          Register 1
#  R2          Register 2
#  R3          Register 3
#  R4          Register 4
#  R5          Register 5
#  R6          Register 6
#  R7          Register 7
#  R8          Register 8
#  R9          Register 9
#  R10         Register 10
#  R11         Register 11
#  R12         Register 12
#  R14         Link register
#  CPSR        Current Program Status Register

# NOTE: Values start at 4 as we use a full stack model;
# the top of the stack passed as a parameter is NOT a valid
# pointer, it is one word PAST the first stack item.

.equ    TR0,    4
.equ    TR1,    TR0 + 4
.equ    TR2,    TR1 + 4
.equ    TR3,    TR2 + 4
.equ    TR4,    TR3 + 4
.equ    TR5,    TR4 + 4
.equ    TR6,    TR5 + 4
.equ    TR7,    TR6 + 4
.equ    TR8,    TR7 + 4
.equ    TR9,    TR8 + 4
.equ    TR10,   TR9 + 4
.equ    TR11,   TR10 + 4
.equ    TR12,   TR11 + 4
.equ    TR14,   TR12 + 4
.equ    CPSR,   TR13 + 4
.equ    SEND,   TR13 + 4

.equ    TLR,    TR14
.equ    TSP,    TR13

