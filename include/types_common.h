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

#ifndef TYPES_COMMON_H_
#define TYPES_COMMON_H_

#include <platform_config.h>
#include <stdint.h>
#include <endian.h>

typedef unsigned BOOL;
typedef int STATUS;
typedef int64_t quad_t;
typedef uint64_t u_quad_t;

#ifndef NELEMENTS
#define NELEMENTS(x) (sizeof(x)/sizeof(x[0]))
#endif

/* ONLY WITH POWERS OF 2 */
#ifndef ALN
#define ALN(x, y) ((x+y-1) & ~(y-1))
#endif

/* ANY NUMBER */
#ifndef MULT
#define MULT(x, y) (((x+y-1)/y)*y)
#endif

/* ONLY WITH POWERS OF 2 */
#ifndef ALNC
#define ALNC(x) ((x+CACHE_ALN-1) & ~(CACHE_ALN-1))
#endif

/* ANY NUMBER */
#ifndef MULTC
#define MULTC(x) (((x+CACHE_ALN-1)/CACHE_ALN)*CACHE_ALN)
#endif

#ifndef Kbit
#define Kbit 1000
#endif

#ifndef Mbit
#define Mbit (1000*Kbit)
#endif

#ifndef Gbit
#define Gbit (1000*Mbit)
#endif

#ifndef Tbit
#define Tbit (1000*Tbit)
#endif

#ifndef KBYTE
#define KBYTE 1024
#endif

#ifndef MBYTE
#define MBYTE (1024*KBYTE)
#endif

#ifndef GBYTE
#define GBYTE (1024*MBYTE)
#endif

#ifndef TBYTE
#define TBYTE (1024*GBYTE)
#endif

#define NBYTES(x)   (1<<x)

#ifndef GNUPACKED
#define GNUPACKED __attribute__ ((packed))
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#endif
