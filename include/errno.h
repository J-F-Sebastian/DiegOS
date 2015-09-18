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

#ifndef _ERRNO_H_
#define _ERRNO_H_

extern int errno;		  /* place where the error numbers go */

#define EOK           (0)   /* no error */

#define EPERM         (-1)  /* operation not permitted */
#define ENOENT        (-2)  /* no such file or directory */
#define ESRCH         (-3)  /* no such process */
#define EINTR         (-4)  /* interrupted function call */
#define EIO           (-5)  /* input/output error */
#define ENXIO         (-6)  /* no such device or address */
#define E2BIG         (-7)  /* arg list too long */
#define EBADF         (-9)  /* bad file descriptor */
#define EAGAIN        (-11)  /* resource temporarily unavailable */
#define ENOMEM        (-12)  /* not enough space */
#define EACCES        (-13)  /* permission denied */
#define EFAULT        (-14)  /* bad address */
#define ENOTBLK       (-15)  /* Block device required */
#define EBUSY         (-16)  /* resource busy */
#define EEXIST        (-17)  /* file exists */
#define EXDEV         (-18)  /* improper link */
#define ENODEV        (-19)  /* no such device */
#define ENOTDIR       (-20)  /* not a directory */
#define EISDIR        (-21)  /* is a directory */
#define EINVAL        (-22)  /* invalid argument */
#define ENFILE        (-23)  /* too many open files in system */
#define EMFILE        (-24)  /* too many open files */
#define ENOTTY        (-25)  /* inappropriate I/O control operation */
#define ETXTBSY       (-26)  /* Text file busy */
#define EFBIG         (-27)  /* file too large */
#define ENOSPC        (-28)  /* no space left on device */
#define ESPIPE        (-29)  /* invalid seek */
#define EROFS         (-30)  /* read-only file system */
#define EMLINK        (-31)  /* too many links */
#define EPIPE         (-32)  /* broken pipe */
#define EDOM          (-33)  /* domain error    	(from ANSI C std) */
#define ERANGE        (-34)  /* result too large	(from ANSI C std) */
#define ENAMETOOLONG  (-36)  /* file name too long */
#define ENOLCK        (-37)  /* no locks available */
#define ENOSYS        (-38)  /* function not implemented */
#define ENOTEMPTY     (-39)  /* directory not empty */
#define ELOOP         (-40)  /* too many levels of symlinks detected */
#define EIDRM         (-43)  /* Identifier removed */
#define EILSEQ        (-44)  /* illegal byte sequence */
#define EFTYPE        (-45)  /* wrong file format or type */
#define EOVERFLOW     (-46)  /* value too large to store in data type */

/* The following errors relate to networking. */
#define EPACKSIZE     (-50)  /* invalid packet size for some protocol */
#define ENOBUFS       (-51)  /* not enough buffers left */
#define EBADIOCTL     (-52)  /* illegal ioctl for device */
#define EBADMODE      (-53)  /* badmode in ioctl */
#define EWOULDBLOCK   (-54)  /* call would block on nonblocking socket */
#define ENETUNREACH   (-55)  /* network unreachable */
#define EHOSTUNREACH  (-56)  /* host unreachable */
#define EISCONN	      (-57)  /* already connected */
#define EADDRINUSE    (-58)  /* address in use */
#define ECONNREFUSED  (-59)  /* connection refused */
#define ECONNRESET    (-60)  /* connection reset */
#define ETIMEDOUT     (-61)  /* connection timed out */
#define EURG	      (-62)  /* urgent data present */
#define ENOURG	      (-63)  /* no urgent data present */
#define ENOTCONN      (-64)  /* no connection (yet or anymore) */
#define ESHUTDOWN     (-65)  /* a write call to a shutdown connection */
#define ENOCONN       (-66)  /* no such connection */
#define EAFNOSUPPORT  (-67)  /* address family not supported */
#define EPROTONOSUPPORT (-68) /* protocol not supported by AF */
#define EPROTOTYPE    (-69)  /* Protocol wrong type for socket */
#define EINPROGRESS   (-70)  /* Operation now in progress */
#define EADDRNOTAVAIL (-71)  /* Can't assign requested address */
#define EALREADY      (-72)  /* Connection already in progress */
#define EMSGSIZE      (-73)  /* Message too long */
#define ENOTSOCK      (-74)  /* Socket operation on non-socket */
#define ENOPROTOOPT   (-75)  /* Protocol not available */
#define EOPNOTSUPP    (-76)  /* Operation not supported */
#define ENOTSUP       EOPNOTSUPP  /* Not supported */
#define ENETDOWN      (-77)  /* network is down */
#define EPFNOSUPPORT  (-78)  /* Protocol family not supported */
#define EDESTADDRREQ  (-79)  /* Destination address required */
#define EHOSTDOWN     (-80)  /* Host down */
#define EINVDATA      (-81)  /* Packet data not valid / CRC error */

#define EGENERIC      (-99) /* generic error */

#endif
