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

#include <errno.h>
#include <stddef.h>

const char *errno2str(int errno)
{
	const char *errmsg = NULL;

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
		errmsg = "block device required";
		break;
	case EBUSY:
		errmsg = "resource busy";
		break;
	case EEXIST:
		errmsg = "file exists";
		break;
	case EXDEV:
		errmsg = "improper link";
		break;
	case ENODEV:
		errmsg = "no such device";
		break;
	case ENOTDIR:
		errmsg = "not a directory";
		break;
	case EISDIR:
		errmsg = "is a directory";
		break;
	case EINVAL:
		errmsg = "invalid argument";
		break;
	case ENFILE:
		errmsg = "too many open files in system";
		break;
	case EMFILE:
		errmsg = "too many open files";
		break;
	case ENOTTY:
		errmsg = "inappropriate I/O control operation";
		break;
	case ETXTBSY:
		errmsg = "text file is busy";
		break;
	case EFBIG:
		errmsg = "file is too large";
		break;
	case ENOSPC:
		errmsg = "no space left on device";
		break;
	case ESPIPE:
		errmsg = "invalid seek";
		break;
	case EROFS:
		errmsg = "read-only file system";
		break;
	case EMLINK:
		errmsg = "too many links";
		break;
	case EPIPE:
		errmsg = "broken pipe";
		break;
	case EDOM:
		errmsg = "domain error";
		break;
	case ERANGE:
		errmsg = "result too large";
		break;
	case ENAMETOOLONG:
		errmsg = "file name too long";
		break;
	case ENOLCK:
		errmsg = "no locks available";
		break;
	case ENOSYS:
		errmsg = "function not implemented";
		break;
	case ENOTEMPTY:
		errmsg = "directory not empty";
		break;
	case ELOOP:
		errmsg = "too many levels of symlinks detected";
		break;
	case EIDRM:
		errmsg = "identifier removed";
		break;
	case EILSEQ:
		errmsg = "illegal byte sequence";
		break;
	case EFTYPE:
		errmsg = "wrong file format or type";
		break;
	case EOVERFLOW:
		errmsg = "value too large to store in data type";
		break;
	case EPACKSIZE:
		errmsg = "invalid packet size for some protocol";
		break;
	case ENOBUFS:
		errmsg = "not enough buffers left";
		break;
	case EBADIOCTL:
		errmsg = "illegal ioctl for device";
		break;
	case EBADMODE:
		errmsg = "bad mode in ioctl";
		break;
	case EWOULDBLOCK:
		errmsg = "call would block on non-blocking socket";
		break;
	case ENETUNREACH:
		errmsg = "network is unreachable";
		break;
	case EHOSTUNREACH:
		errmsg = "host is unreachable";
		break;
	case EISCONN:
		errmsg = "already connected";
		break;
	case EADDRINUSE:
		errmsg = "address is in use";
		break;
	case ECONNREFUSED:
		errmsg = "connection is refused";
		break;
	case ECONNRESET:
		errmsg = "connection is reset";
		break;
	case ETIMEDOUT:
		errmsg = "connection timed out";
		break;
	case EURG:
		errmsg = "urgent data present";
		break;
	case ENOURG:
		errmsg = "no urgent data present";
		break;
	case ENOTCONN:
		errmsg = "no connection (yet or anymore)";
		break;
	case ESHUTDOWN:
		errmsg = "a write call to a shutdown connection";
		break;
	case ENOCONN:
		errmsg = "no such connection";
		break;
	case EAFNOSUPPORT:
		errmsg = "address family not supported";
		break;
	case EPROTONOSUPPORT:
		errmsg = "protocol not supported by AF";
		break;
	case EPROTOTYPE:
		errmsg = "wrong protocol type for socket";
		break;
	case EINPROGRESS:
		errmsg = "operation is in progress now";
		break;
	case EADDRNOTAVAIL:
		errmsg = "can't assign the requested address";
		break;
	case EALREADY:
		errmsg = "connection is already in progress";
		break;
	case EMSGSIZE:
		errmsg = "message is too long";
		break;
	case ENOTSOCK:
		errmsg = "socket operation on non-socket";
		break;
	case ENOPROTOOPT:
		errmsg = "protocol is not available";
		break;
	case EOPNOTSUPP:
		/* case ENOTSUP */
		errmsg = "operation is not supported";
		break;
	case ENETDOWN:
		errmsg = "network is down";
		break;
	case EPFNOSUPPORT:
		errmsg = "protocol family is not supported";
		break;
	case EDESTADDRREQ:
		errmsg = "destination address is required";
		break;
	case EHOSTDOWN:
		errmsg = "host is down";
		break;
	case EINVDATA:
		errmsg = "packet data is not valid / CRC error";
		break;
	case EGENERIC:
		errmsg = "generic error";
		break;
	}

	return (errmsg);
}
