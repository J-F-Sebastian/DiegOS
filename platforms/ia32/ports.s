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

.file	"ports.s"
.sect .text

.globl 	in_byte
.type	in_byte, @function

in_byte:
subl    %eax, %eax
movw	4(%esp), %dx		/* port*/
inb	    %dx			        /* read 1 byte*/
ret

.globl	in_word
.type	in_word,@function

in_word:
subl    %eax, %eax
movw	4(%esp), %dx		/* port */
inw	    %dx			        /* read 1 word */
ret

.globl	in_dword
.type	in_dword,@function

in_dword:
subl    %eax, %eax
movw	4(%esp), %dx		/* port */
inl	    %dx			        /* read 1 double word */
ret

.globl	out_byte
.type	out_byte, @function

out_byte:
movw    4(%esp), %dx		/* port */
movb	8(%esp), %al		/* value */
out	    %al,%dx		        /* output 1 byte */
ret

.globl	out_word
.type	out_word,@function

out_word:
movw    4(%esp), %dx		/* port */
movw    8(%esp), %ax		/* value */
out 	%ax, %dx		    /* output 1 word */
ret

.globl	out_dword
.type	out_dword,@function

out_dword:
movw    4(%esp), %dx		/* port */
movl    8(%esp), %eax		/* value */
outl	%dx                 /* output 1 double word */
ret
