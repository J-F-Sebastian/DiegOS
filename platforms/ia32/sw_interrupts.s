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

.file "sw_interrupts.s"
.text
/* SW INT */
.globl swint48
.globl swint49
.globl swint50
.globl swint51
.globl swint52
.globl swint53
.globl swint54
.globl swint55
.globl swint56
.globl swint57
.globl swint58
.globl swint59
.globl swint60
.globl swint61
.globl swint62
.globl swint63
/* HW INT */
.globl swint64
.globl swint65
.globl swint66
.globl swint67
.globl swint68
.globl swint69
.globl swint70
.globl swint71
.globl swint72
.globl swint73
.globl swint74
.globl swint75
.globl swint76
.globl swint77
.globl swint78
.globl swint79
.globl swint80
.globl swint81
.globl swint82
.globl swint83
.globl swint84
.globl swint85
.globl swint86
.globl swint87
.globl swint88
.globl swint89
.globl swint90
.globl swint91
.globl swint92
.globl swint93
.globl swint94
.globl swint95

.type swint48, @function
.type swint49, @function
.type swint50, @function
.type swint51, @function
.type swint52, @function
.type swint53, @function
.type swint54, @function
.type swint55, @function
.type swint56, @function
.type swint57, @function
.type swint58, @function
.type swint59, @function
.type swint60, @function
.type swint61, @function
.type swint62, @function
.type swint63, @function
.type swint64, @function
.type swint65, @function
.type swint66, @function
.type swint67, @function
.type swint68, @function
.type swint69, @function
.type swint70, @function
.type swint71, @function
.type swint72, @function
.type swint73, @function
.type swint74, @function
.type swint75, @function
.type swint76, @function
.type swint77, @function
.type swint78, @function
.type swint79, @function
.type swint80, @function
.type swint81, @function
.type swint82, @function
.type swint83, @function
.type swint84, @function
.type swint85, @function
.type swint86, @function
.type swint87, @function
.type swint88, @function
.type swint89, @function
.type swint90, @function
.type swint91, @function
.type swint92, @function
.type swint93, @function
.type swint94, @function
.type swint95, @function

.macro swint int
 pusha                          /* stack pointer is not changed   */
 call *int_table + 4*\int(,1)   /* eax = (*int_table[int])()      */
 popa
 iret                           /* restart the process            */
.endm

.align 16
swint48:
swint 16

.align 16
swint49:
swint 17

.align 16
swint50:
swint 18

.align 16
swint51:
swint 19

.align 16
swint52:
swint 20

.align 16
swint53:
swint 21

.align 16
swint54:
swint 22

.align 16
swint55:
swint 23

.align 16
swint56:
swint 24

.align 16
swint57:
swint 25

.align 16
swint58:
swint 26

.align 16
swint59:
swint 27

.align 16
swint60:
swint 28

.align 16
swint61:
swint 29

.align 16
swint62:
swint 30

.align 16
swint63:
swint 31

.align 16
swint64:
swint 32

.align 16
swint65:
swint 33

.align 16
swint66:
swint 34

.align 16
swint67:
swint 35

.align 16
swint68:
swint 36

.align 16
swint69:
swint 37

.align 16
swint70:
swint 38

.align 16
swint71:
swint 39

.align 16
swint72:
swint 40

.align 16
swint73:
swint 41

.align 16
swint74:
swint 42

.align 16
swint75:
swint 43

.align 16
swint76:
swint 44

.align 16
swint77:
swint 45

.align 16
swint78:
swint 46

.align 16
swint79:
swint 47

.align 16
swint80:
swint 48

.align 16
swint81:
swint 49

.align 16
swint82:
swint 50

.align 16
swint83:
swint 51

.align 16
swint84:
swint 52

.align 16
swint85:
swint 53

.align 16
swint86:
swint 54

.align 16
swint87:
swint 55

.align 16
swint88:
swint 56

.align 16
swint89:
swint 57

.align 16
swint90:
swint 58

.align 16
swint91:
swint 59

.align 16
swint92:
swint 60

.align 16
swint93:
swint 61

.align 16
swint94:
swint 62

.align 16
swint95:
swint 63
