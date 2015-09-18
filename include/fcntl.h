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

#ifndef _FCNTL_H_
#define _FCNTL_H_

#include <sys/types.h>

/* These values are used for cmd in fcntl().  POSIX Table 6-1.  */
#define F_DUPFD            0	/* duplicate file descriptor */
#define F_GETFD	           1	/* get file descriptor flags */
#define F_SETFD            2	/* set file descriptor flags */
#define F_GETFL            3	/* get file status flags */
#define F_SETFL            4	/* set file status flags */
#define F_GETOWN           5    /* get the process or process group ID specified to
                                 * receive SIGURG signals when out-of-band data is available.
                                 * SOCKETS ONLY
                                 */
#define F_SETOWN           6    /* set the process or process group ID specified to
                                 * receive SIGURG signals when out-of-band data is available.
                                 * SOCKETS ONLY
                                 */
#define F_GETLK            7	/* get record locking information */
#define F_SETLK            8	/* set record locking information */
#define F_SETLKW           9	/* set record locking info; wait if blocked */

/* File descriptor flags used for fcntl().  POSIX Table 6-2. */
#define FD_CLOEXEC         1	/* close on exec flag for third arg of fcntl */

/* L_type values for record locking with fcntl().  POSIX Table 6-3. */
#define F_RDLCK            1	/* shared or read lock */
#define F_WRLCK            2	/* exclusive or write lock */
#define F_UNLCK            3	/* unlock */

/* Oflag values for open().  POSIX Table 6-4. */
#define O_CREAT        00100	/* creat file if it doesn't exist */
#define O_EXCL         00200	/* exclusive use flag */
#define O_NOCTTY       00400	/* do not assign a controlling terminal */
#define O_TRUNC        01000	/* truncate flag */

/* File status flags for open() and fcntl().  POSIX Table 6-5. */
#define O_APPEND       02000	/* set append mode */
#define O_NONBLOCK     04000	/* no delay */

/* File access modes for open() and fcntl().  POSIX Table 6-6. */
#define O_RDONLY           0	/* open(name, O_RDONLY) opens read only */
#define O_WRONLY           1	/* open(name, O_WRONLY) opens write only */
#define O_RDWR             2	/* open(name, O_RDWR) opens read/write */

/* Mask for use with file access modes.  POSIX Table 6-7. */
#define O_ACCMODE         03	/* mask for file access modes */

/* Struct used for locking.  POSIX Table 6-8. */
struct flock {
    short l_type; /* type: F_RDLCK, F_WRLCK, or F_UNLCK */
    short l_whence; /* flag for starting offset */
    off_t l_start; /* relative offset in bytes */
    off_t l_len; /* size; if 0, then until EOF */
    pid_t l_pid; /* process id of the locks' owner */
};

/* Function Prototypes. */
int creat(const char *name, int perms);
int fcntl(int fd, int cmd, ...);
int open(const char *name, int flags, int perms);

#endif /* _FCNTL_H */
