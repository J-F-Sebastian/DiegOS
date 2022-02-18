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

#ifndef _STDINT_H
#define _STDINT_H

#include <cdefs.h>

/* 
 * Integer types of precisely the given bitsize. 
 * Applies to 32-bit processors only.
 */
typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef long long int int64_t;

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

/* Integer type capable of holding a pointer and the largest integer type. */
typedef int intptr_t;
typedef unsigned uintptr_t;

typedef int64_t intmax_t;
typedef uint64_t uintmax_t;

#if !__cplusplus || defined(__STDC_LIMIT_MACROS)
#ifndef _LIMITS_H
#include <limits.h>
#endif

/* Range definitions for each of the above types conform <limits.h>. */
#define INT8_MIN		(-INT8_MAX-1)
#define INT16_MIN		(-INT16_MAX-1)
#define INT32_MIN		(-INT32_MAX-1)
#define INT64_MIN		(-INT64_MAX-1)

#define INT8_MAX		127
#define INT16_MAX		32767
#define INT32_MAX		2147483647
#define INT64_MAX		9223372036854775807LL

#define UINT8_MAX		255
#define UINT16_MAX		65535
#define UINT32_MAX		4294967295U
#define UINT64_MAX		18446744073709551615ULL

#define INT_LEAST8_MIN		INT8_MIN
#define INT_LEAST16_MIN		INT16_MIN
#define INT_LEAST32_MIN		INT32_MIN
#define INT_LEAST64_MIN		INT64_MIN

#define INT_LEAST8_MAX		INT8_MAX
#define INT_LEAST16_MAX		INT16_MAX
#define INT_LEAST32_MAX		INT32_MAX
#define INT_LEAST64_MAX		INT64_MAX

#define UINT_LEAST8_MAX		UINT8_MAX
#define UINT_LEAST16_MAX	UINT16_MAX
#define UINT_LEAST32_MAX	UINT32_MAX
#define UINT_LEAST64_MAX	UINT64_MAX

#define INT_FAST8_MIN		(-INT_FAST8_MAX-1)
#define INT_FAST16_MIN		(-INT_FAST16_MAX-1)
#define INT_FAST32_MIN		INT32_MIN
#define INT_FAST64_MIN		INT64_MIN

#define INT_FAST8_MAX		INT32_MAX
#define INT_FAST16_MAX		INT32_MAX
#define INT_FAST32_MAX		INT32_MAX
#define INT_FAST64_MAX		INT64_MAX

#define UINT_FAST8_MAX		UINT32_MAX
#define UINT_FAST16_MAX		UINT32_MAX
#define UINT_FAST32_MAX		UINT32_MAX
#define UINT_FAST64_MAX		UINT64_MAX

#define INTPTR_MIN		INT_MIN
#define INTPTR_MAX		INT_MAX
#define UINTPTR_MAX		UINT_MAX

#define INTMAX_MIN		INT64_MIN
#define INTMAX_MAX		INT64_MAX
#define UINTMAX_MAX		UINT64_MAX

#endif				/* !__cplusplus || __STDC_LIMIT_MACROS */

/* Constants of the proper type. */
#define INT8_C(c)	c
#define INT16_C(c)	c
#define INT32_C(c)	c
#define INT64_C(c)	__CONCAT(c,l)

#define UINT8_C(c)	__CONCAT(c,u)
#define UINT16_C(c)	__CONCAT(c,u)
#define UINT32_C(c)	__CONCAT(c,u)
#define UINT64_C(c)	__CONCAT(c,lu)

#define INTMAX_C(c)	INT64_C(c)
#define UINTMAX_C(c)	UINT64_C(c)

#endif				/* _STDINT_H */
