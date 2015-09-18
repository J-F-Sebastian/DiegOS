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

#ifndef _STDDEF_H_
#define _STDDEF_H_

/* The following is not portable, but the compiler accepts it. */
#ifdef __GNUC__
//#define offsetof(type, ident)	__builtin_offsetof (type, ident)
#else
#define offsetof(type, ident)	((size_t) (unsigned long) &((type *)0)->ident)
#endif

typedef int ptrdiff_t;		/* result of subtracting two pointers */

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;	/* type returned by sizeof */
#endif

#ifndef _WCHAR_T
#define _WCHAR_T
typedef char wchar_t;		/* type expanded character set */
#endif

#ifndef NULL
#define NULL (void *)0
#endif // NULL

#endif /* _STDDEF_H */
