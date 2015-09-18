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

#ifndef _LIMITS_H_
#define _LIMITS_H_

#define CHAR_BIT           8	/* # bits in a char */
#define CHAR_MIN        -128	/* minimum value of a char */
#define CHAR_MAX         127	/* maximum value of a char */
#define SCHAR_MIN       -128	/* minimum value of a signed char */
#define SCHAR_MAX        127	/* maximum value of a signed char */
#define UCHAR_MAX        255	/* maximum value of an unsigned char */
#define MB_LEN_MAX         1	/* maximum length of a multibyte char */

#define SHRT_MIN  (-32767-1)	/* minimum value of a short */
#define SHRT_MAX       32767	/* maximum value of a short */
#define USHRT_MAX     0xFFFF	/* maximum value of unsigned short */

#define INT_MIN (-2147483647-1)	/* minimum value of a 32-bit int */
#define INT_MAX   2147483647	/* maximum value of a 32-bit int */
#define UINT_MAX  0xFFFFFFFF	/* maximum value of an unsigned 32-bit int */

#define LONG_MIN (-2147483647L-1) /* minimum value of a long */
#define LONG_MAX  2147483647L	  /* maximum value of a long */
#define ULONG_MAX 0xFFFFFFFFL	  /* maximum value of an unsigned long */

#define LLONG_MIN  (-0x7FFFFFFFFFFFFFFFLL-1) 
/* minimum value of a long long */
#define LLONG_MAX  0x7FFFFFFFFFFFFFFFLL	     
/* maximum value of a long long */
#define ULLONG_MAX 0xFFFFFFFFFFFFFFFFULL     
/* maximum value of an unsigned long long */

#define NAME_MAX 255
#define PATH_MAX 1023

#define OPEN_MAX 64

#endif /* _LIMITS_H */
