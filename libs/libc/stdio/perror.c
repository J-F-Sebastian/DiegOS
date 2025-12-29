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

#include <stdio.h>
#include <errno.h>

void perror(const char *s)
{
	const char *errmsg = "Unknown error code";

	if (s) {
		(void)fprintf(stderr, "%s: ", s);
	}

	switch (errno) {
	case EOK:
		errmsg = "no error";
		break;
	case EPERM:
		errmsg = "operation not permitted";
		break;
	case ENOENT:
		errmsg = "no such file or directory";
		break;
	case ESRCH:
		errmsg = "no such process";
		break;
	case EINTR:
		errmsg = "interrupted function call";
		break;
	case EIO:
		errmsg = "input/output error";
		break;
	case ENXIO:
		errmsg = "no such device or address";
		break;
	case E2BIG:
		errmsg = "arg list too long";
		break;
	case EBADF:
		errmsg = "bad file descriptor";
		break;
	case EAGAIN:
		errmsg = "resource temporarily unavailable";
		break;
	case ENOMEM:
		errmsg = "not enough space";
		break;
	case EACCES:
		errmsg = "permission denied";
		break;
	case EFAULT:
		errmsg = "bad address";
		break;
	case ENOTBLK:
		errmsg = "Block device required";
		break;
	case EBUSY:
		errmsg = "resource busy";
		break;
#if 0
		case EEXIST(-17)	/* file exists */
		    case EXDEV(-18)	/* improper link */
		    case ENODEV(-19)	/* no such device */
		    case ENOTDIR(-20)	/* not a directory */
		    case EISDIR(-21)	/* is a directory */
		    case EINVAL(-22)	/* invalid argument */
		    case ENFILE(-23)	/* too many open files in system */
		    case EMFILE(-24)	/* too many open files */
		    case ENOTTY(-25)	/* inappropriate I/O control operation */
		    case ETXTBSY(-26)	/* Text file busy */
		    case EFBIG(-27)	/* file too large */
		    case ENOSPC(-28)	/* no space left on device */
		    case ESPIPE(-29)	/* invalid seek */
		    case EROFS(-30)	/* read-only file system */
		    case EMLINK(-31)	/* too many links */
		    case EPIPE(-32)	/* broken pipe */
		    case EDOM(-33)	/* domain error        (from ANSI C std) */
		    case ERANGE(-34)	/* result too large    (from ANSI C std) */
		    case ENAMETOOLONG(-36)	/* file name too long */
		    case ENOLCK(-37)	/* no locks available */
		    case ENOSYS(-38)	/* function not implemented */
		    case ENOTEMPTY(-39)	/* directory not empty */
		    case ELOOP(-40)	/* too many levels of symlinks detected */
		    case EIDRM(-43)	/* Identifier removed */
		    case EILSEQ(-44)	/* illegal byte sequence */
		    case EFTYPE(-45)	/* wrong file format or type */
		    case EOVERFLOW(-46)
		    /* value too large to store in data type */
		    /* The following errors relate to networking. */
		    case EPACKSIZE(-50)	/* invalid packet size for some protocol */
		    case ENOBUFS(-51)	/* not enough buffers left */
		    case EBADIOCTL(-52)	/* illegal ioctl for device */
		    case EBADMODE(-53)	/* badmode in ioctl */
		    case EWOULDBLOCK(-54)	/* call would block on nonblocking socket */
		    case ENETUNREACH(-55)	/* network unreachable */
		    case EHOSTUNREACH(-56)	/* host unreachable */
		    case EISCONN(-57)	/* already connected */
		    case EADDRINUSE(-58)	/* address in use */
		    case ECONNREFUSED(-59)	/* connection refused */
		    case ECONNRESET(-60)	/* connection reset */
		    case ETIMEDOUT(-61)	/* connection timed out */
		    case EURG(-62)	/* urgent data present */
		    case ENOURG(-63)	/* no urgent data present */
		    case ENOTCONN(-64)	/* no connection (yet or anymore) */
		    case ESHUTDOWN(-65)	/* a write call to a shutdown connection */
		    case ENOCONN(-66)	/* no such connection */
		    case EAFNOSUPPORT(-67)	/* address family not supported */
		    case EPROTONOSUPPORT(-68)	/* protocol not supported by AF */
		    case EPROTOTYPE(-69)	/* Protocol wrong type for socket */
		    case EINPROGRESS(-70)	/* Operation now in progress */
		    case EADDRNOTAVAIL(-71)	/* Can't assign requested address */
		    case EALREADY(-72)	/* Connection already in progress */
		    case EMSGSIZE(-73)	/* Message too long */
		    case ENOTSOCK(-74)	/* Socket operation on non-socket */
		    case ENOPROTOOPT(-75)	/* Protocol not available */
		    case EOPNOTSUPP(-76)	/* Operation not supported */
		    case ENOTSUP EOPNOTSUPP	/* Not supported */
		    case ENETDOWN(-77)	/* network is down */
		    case EPFNOSUPPORT(-78)	/* Protocol family not supported */
		    case EDESTADDRREQ(-79)	/* Destination address required */
		    case EHOSTDOWN(-80)	/* Host down */
		    case EINVDATA(-81)
		    /* Packet data not valid / CRC error */
		    case EGENERIC(-99)	/* generic error */
#endif
	}
	fprintf(stderr, "%s\n", errmsg);
}
