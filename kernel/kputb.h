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

#ifndef _KPUTB_H_
#define _KPUTB_H_

/*
 * This function is platform dependent, it must be
 * defined in platform sources and must be linked to
 * the kernel.
 * PARAMETERS
 * char *buffer - character buffer to be sent to the console
 *                TTY output.
 * unsigned bytes - size fo the buffer.
 */
extern void kputb(char *buffer, unsigned bytes);

#endif
