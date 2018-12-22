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

.file "context_equ.s"

# Layout of the initial stack of a thread

.equ    EAX,    0
.equ    ECX,    EAX + 4
.equ    EDX,    ECX + 4
.equ    EBX,    EDX + 4
.equ    ESP,    EBX + 4
.equ    EBP,    ESP + 4
.equ    ESI,    EBP + 4
.equ    EDI,    ESI + 4
.equ    EFLAGS, EDI + 4
.equ    STRPTR, EFLAGS + 4
.equ    RETPTR, STRPTR + 4
