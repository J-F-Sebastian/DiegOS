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

/* The <assert.h> header contains a macro called "assert" that allows
 * programmers to put assertions in the code.  These assertions can be verified
 * at run time.  If an assertion fails, an error message is printed and the
 * program aborts.
 * Assertion checking can be disabled by adding the statement
 *
 *	#define NDEBUG
 *
 * to the program before the
 *
 *	#include <assert.h>
 *
 * statement.
 */

#ifndef _ASSERT_H_
#define _ASSERT_H_

#ifdef NDEBUG
/* Debugging disabled -- do not evaluate assertions. */
#define assert(expr)
#else
/* Debugging enabled -- verify assertions at run time. */

void assertion (const char *expr, const char *file, int line, const char *fn);

#define	assert(expr)\
    if (!(expr)) {\
        assertion(#expr, __FILE__, __LINE__, __FUNCTION__); \
    }

#endif
#endif 
