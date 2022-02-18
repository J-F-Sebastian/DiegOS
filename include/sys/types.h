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

#ifndef _TYPES_H_
#define _TYPES_H_

#ifndef _SSIZE_T
#define _SSIZE_T
typedef int ssize_t;
#endif

/* Types used in disk, inode, etc. data structures. */
typedef short dev_t;		/* holds (major|minor) device pair */
typedef char gid_t;		/* group id */
typedef unsigned short ino_t;	/* i-node number */
typedef unsigned short mode_t;	/* file type and permissions bits */
typedef char nlink_t;		/* number of links to a file */
typedef long int off_t;		/* offset within a file */
typedef int pid_t;		/* process id (must be signed) */
typedef int Mode_t;

#endif
