/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2022 Diego Gallizioli
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
/* SW INT */
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
/* HW INT */
.globl swint16
.globl swint17
.globl swint18
.globl swint19
.globl swint20
.globl swint21
.globl swint22
.globl swint23
.globl swint24
.globl swint25
.globl swint26
.globl swint27
.globl swint28
.globl swint29
.globl swint30
.globl swint31
.globl swint32
.globl swint33
.globl swint34
.globl swint35
.globl swint36
.globl swint37
.globl swint38
.globl swint39
.globl swint40
.globl swint41
.globl swint42
.globl swint43
.globl swint44
.globl swint45
.globl swint46
.globl swint47

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
.type  swint16, @function
.type  swint17, @function
.type  swint18, @function
.type  swint19, @function
.type  swint20, @function
.type  swint21, @function
.type  swint22, @function
.type  swint23, @function
.type  swint24, @function
.type  swint25, @function
.type  swint26, @function
.type  swint27, @function
.type  swint28, @function
.type  swint29, @function
.type  swint30, @function
.type  swint31, @function
.type  swint32, @function
.type  swint33, @function
.type  swint34, @function
.type  swint35, @function
.type  swint36, @function
.type  swint37, @function
.type  swint38, @function
.type  swint39, @function
.type  swint40, @function
.type  swint41, @function
.type  swint42, @function
.type  swint43, @function
.type  swint44, @function
.type  swint45, @function
.type  swint46, @function
.type  swint47, @function

.macro swint int
cld				/* save interrupted process state */
pusha				/* stack pointer is not changed   */
call	*int_table + 4*\int	/* eax = (*irq_table[int])()      */
popa
iret				/* restart the process            */
.endm

.align 16
swint00:
swint 16

.align 16
swint01:
swint 17

.align 16
swint02:
swint 18

.align 16
swint03:
swint 19

.align 16
swint04:
swint 20

.align 16
swint05:
swint 21

.align 16
swint06:
swint 22

.align 16
swint07:
swint 23

.align 16
swint08:
swint 24

.align 16
swint09:
swint 25

.align 16
swint10:
swint 26

.align 16
swint11:
swint 27

.align 16
swint12:
swint 28

.align 16
swint13:
swint 29

.align 16
swint14:
swint 30

.align 16
swint15:
swint 31

.align 16
swint16:
swint 32

.align 16
swint17:
swint 33

.align 16
swint18:
swint 34

.align 16
swint19:
swint 35

.align 16
swint20:
swint 36

.align 16
swint21:
swint 37

.align 16
swint22:
swint 38

.align 16
swint23:
swint 39

.align 16
swint24:
swint 40

.align 16
swint25:
swint 41

.align 16
swint26:
swint 42

.align 16
swint27:
swint 43

.align 16
swint28:
swint 44

.align 16
swint29:
swint 45

.align 16
swint30:
swint 46

.align 16
swint31:
swint 47

.align 16
swint32:
swint 48

.align 16
swint33:
swint 49

.align 16
swint34:
swint 50

.align 16
swint35:
swint 51

.align 16
swint36:
swint 52

.align 16
swint37:
swint 53

.align 16
swint38:
swint 54

.align 16
swint39:
swint 55

.align 16
swint40:
swint 56

.align 16
swint41:
swint 57

.align 16
swint42:
swint 58

.align 16
swint43:
swint 59

.align 16
swint44:
swint 60

.align 16
swint45:
swint 61

.align 16
swint46:
swint 62

.align 16
swint47:
swint 63
