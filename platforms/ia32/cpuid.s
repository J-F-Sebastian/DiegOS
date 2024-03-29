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
.globl execute_cpuid
.type  execute_cpuid, @function

/*
 * void execute_cpuid(struct cpuid_data *data, unsigned infotype);
 */
execute_cpuid:
pushl   %edi
movl	12(%esp), %eax
movl	8(%esp), %edi
cpuid
movl    %eax, (%edi)
movl	%ebx, 4(%edi)
movl	%ecx, 8(%edi)
movl	%edx, 12(%edi)
popl    %edi
ret
