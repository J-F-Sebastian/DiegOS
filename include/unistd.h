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

/*
 * This <unistd.h> is far from being a TRUE unistd.h.
 * DiegOS is NOT unix and NOT unix-like. But C and libc comes with a Unix
 * interface and you won't make it without creat, open and stdio.h stuff.
 * Also, this file has been copied from a POSIX compliant system. This is
 * worth than anything else.
 */

#ifndef _UNISTD_H_
#define _UNISTD_H_

#include <sys/types.h>
#include <stddef.h>

/* Values used by access().  POSIX Table 2-8. */
#define F_OK               0	/* test if file exists */
#define X_OK               1	/* test if file is executable */
#define W_OK               2	/* test if file is writable */
#define R_OK               4	/* test if file is readable */

/* Values used for whence in lseek(fd, offset, whence).  POSIX Table 2-9. */
#define SEEK_SET           0	/* offset is absolute  */
#define SEEK_CUR           1	/* offset is relative to current position */
#define SEEK_END           2	/* offset is relative to end of file */

/* This value is required by POSIX Table 2-10. */
#define _POSIX_VERSION 199009L	/* which standard is being conformed to */

/* These three definitions are required by POSIX Sec. 8.2.1.2. */
#define STDIN_FILENO       0	/* file descriptor for stdin */
#define STDOUT_FILENO      1	/* file descriptor for stdout */
#define STDERR_FILENO      2	/* file descriptor for stderr */

/* NULL must be defined in <unistd.h> according to POSIX Sec. 2.7.1. */
#ifndef NULL
#define NULL    ((void *)0)
#endif

/* The following relate to configurable system variables. POSIX Table 4-2. */
#define _SC_ARG_MAX		    1
#define _SC_CHILD_MAX		2
#define _SC_CLOCKS_PER_SEC	3
#define _SC_CLK_TCK         3
#define _SC_NGROUPS_MAX		4
#define _SC_OPEN_MAX		5
#define _SC_JOB_CONTROL		6
#define _SC_SAVED_IDS		7
#define _SC_VERSION		    8
#define _SC_STREAM_MAX		9
#define _SC_TZNAME_MAX      10

/* The following relate to configurable pathname variables. POSIX Table 5-2. */
#define _PC_LINK_MAX	    	1	/* link count */
#define _PC_MAX_CANON	    	2	/* size of the canonical input queue */
#define _PC_MAX_INPUT	    	3	/* type-ahead buffer size */
#define _PC_NAME_MAX	    	4	/* file name size */
#define _PC_PATH_MAX	    	5	/* pathname size */
#define _PC_PIPE_BUF	    	6	/* pipe size */
#define _PC_NO_TRUNC		    7	/* treatment of long name components */
#define _PC_VDISABLE		    8	/* tty disable */
#define _PC_CHOWN_RESTRICTED	9	/* chown restricted or not */

/* POSIX defines several options that may be implemented or not, at the
 * implementer's whim.  This implementer has made the following choices:
 *
 * _POSIX_JOB_CONTROL	    not defined:	no job control
 * _POSIX_SAVED_IDS 	    not defined:	no saved uid/gid
 * _POSIX_NO_TRUNC	    defined as -1:	long path names are truncated
 * _POSIX_CHOWN_RESTRICTED  defined:		you can't give away files
 * _POSIX_VDISABLE	    defined:		tty functions can be disabled
 */
#define _POSIX_NO_TRUNC       (-1)
#define _POSIX_CHOWN_RESTRICTED  1

/* Function Prototypes. */
int close(int fd);
off_t lseek(int fd, off_t offset, int _whence);
ssize_t read(int fd, void *buf, size_t n);
unsigned int sleep(unsigned int seconds);
unsigned int msleep(unsigned int mseconds);
ssize_t write(int fd, const void *buf, size_t n);
int access(const char *path, int amode);

#endif				/* _UNISTD_H */
