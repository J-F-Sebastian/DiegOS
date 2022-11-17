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

/* The <stdarg.h> header is ANSI's way to handle variable numbers of params.
 * Some programming languages require a function that is declared with n
 * parameters to be called with n parameters.  C does not.  A function may
 * called with more parameters than it is declared with.  The well-known
 * printf function, for example, may have arbitrarily many parameters.
 * The question arises how one can access all the parameters in a portable
 * way.  The C standard defines three macros that programs can use to
 * advance through the parameter list.  The definition of these macros for
 * MINIX are given in this file.  The three macros are:
 *
 *	va_start(ap, parmN)	prepare to access parameters
 *	va_arg(ap, type)	get next parameter value and type
 *	va_end(ap)		access is finished
 *
 * Ken Thompson's famous line from V6 UNIX is equally applicable to this file:
 *
 *	"You are not expected to understand this"
 *
 */

#ifndef _STDARG_H
#define _STDARG_H

#ifdef __GNUC__

/* The GNU C-compiler uses its own, but similar varargs mechanism. */

typedef __builtin_va_list va_list;

/* Amount of space required in an argument list for an arg of type TYPE.
 * TYPE may alternatively be an expression whose type is used.
 */

#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#define va_start(ap, last) __builtin_va_start((ap), (last))
#define va_arg(ap, type) __builtin_va_arg((ap), type)
#define va_end(ap) __builtin_va_end(ap)
#define va_copy(dest, src) __builtin_va_copy((dest), (src))

#else				/* not __GNUC__ */

typedef char *va_list;

#define __vasz(x)		((sizeof(x)+sizeof(int)-1) & ~(sizeof(int) -1))

#define va_start(ap, parmN)	((ap) = (va_list)&parmN + __vasz(parmN))
#define va_arg(ap, type)      \
  (*((type *)((va_list)((ap) = (void *)((va_list)(ap) + __vasz(type))) \
						    - __vasz(type))))
#define va_copy(ap2, ap) (ap2) = (ap)
#define va_end(ap)

#endif				/* __GNUC__ */

#endif				/* _STDARG_H */
