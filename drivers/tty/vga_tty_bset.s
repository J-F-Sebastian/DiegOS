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

.file	"vga_tty_betS"
.text
.globl  vga_tty_bset
.type	vga_tty_bset, @function

/*
** on entry, stack looks like this:
**     12(esp)  ->              words
**      8(esp)  ->              fill value
**      4(esp)  ->              destination
**       (esp)  ->              return address
**
** void vga_tty_bset (uint16_t *dst, uint16_t val, unsigned words);
**
*/
vga_tty_bset:
movl    %edi, %ebx
movl    12(%esp), %ecx
movzwl   8(%esp), %eax
movl     4(%esp), %edi
rep     stosw
movl    %ebx, %edi
ret


