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

#ifndef _ia32_h_
#define _ia32_h_

#define CACHE_ALN 64

#define _BYTE_ORDER _LITTLE_ENDIAN

/* The following defines can be explicitely uncommented or declared
 * on the compiler's command line.
 * Processors with no support for a specific feature will still run as
 * expected, the defines enable software support.
 * Defines are NOT platform dependent - they are used in the kernel and in 
 * other platform independent code.
 */

#define ENABLE_FP
//#define ENABLE_SIMD

#endif

