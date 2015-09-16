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

.text
.globl delay_loop
.type  delay_loop, @function

/*
 * void delay_loop(unsigned long loops);
 */
 .align 16
delay_loop:
    movl    4(%esp), %eax
    jmp 1f

.align 16
1:
    jmp 2f

.align 16
2:  decl    %eax
    jnz     2b
    ret
