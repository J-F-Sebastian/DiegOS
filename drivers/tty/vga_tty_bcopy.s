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

.file	"vga_tty_bcopy.S"
.text
.globl  vga_tty_bcopy
.type	vga_tty_bcopy, @function

/*
** on entry, stack looks like this:
**     12(esp)  ->              words
**      8(esp)  ->              destination
**      4(esp)  ->              source
**       (esp)  ->              return address
**
** void vga_tty_bcopy (const uint16_t *src, uint16_t *dst, unsigned words);
**
*/

.align 16
vga_tty_bcopy:

movl    %edi, %eax
movl    %esi, %ebx
movl    12(%esp), %ecx
movl     8(%esp), %edi
movl     4(%esp), %esi
/* VGA memory is word - accessed */
rep     movsw
movl    %eax, %edi
movl    %ebx, %esi
ret


