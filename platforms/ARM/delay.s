/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2025 Diego Gallizioli
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
.func  delay_loop

/*
 * void delay_loop(unsigned long loops);
 *
 * The loop is tight and short but functional.
 * ARM earlier than ARMv6 do not support branch prediction.
 * The loop will load the subs and bhi instruction into the
 * processor pipeline.
 * The branch will delay execution for 2 clock ticks, waiting 
 * as the pipeline is made empty.
 * The processor will touch in 16 bytes which won't cross a cachelines
 * boundary as the routine is 16-bytes aligned.
 */
.align 16
delay_loop:
tst	r0, #0
bxeq	lr
.align 16
the_loop:
subs	r0, r0, #1
bhi	the_loop
bx	lr
