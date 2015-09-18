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

#ifndef _CTYPE_H_
#define _CTYPE_H_

/* Function Prototypes */
int isalnum (int c);	/* alphanumeric [a-z], [A-Z], [0-9] */
int isalpha (int c);	/* alphabetic */
int iscntrl (int c);	/* control characters */
int isdigit (int c);	/* digit [0-9] */
int isgraph (int c);	/* graphic character */
int islower (int c);	/* lower-case letter [a-z] */
int isprint (int c);	/* printable character */
int ispunct (int c);	/* punctuation mark */
int isspace (int c);	/* white space sp, \f, \n, \r, \t, \v*/
int isupper (int c);	/* upper-case letter [A-Z] */
int isxdigit (int c);	/* hex digit [0-9], [a-f], [A-F] */
int tolower (int c);	/* convert to lower-case */
int toupper (int c);	/* convert to upper-case */

#endif 
