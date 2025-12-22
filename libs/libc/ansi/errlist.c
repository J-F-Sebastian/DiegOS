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

#include <types_common.h>

static const char unknown[] = "Unknown error";

const char *strerrorlist[] = {
	"No Error",		/* EOK */
	"Operation not permitted",	/* EPERM */
	"No such file or directory",	/* ENOENT */
	"No such process",	/* ESRCH */
	"Interrupted system call",	/* EINTR */
	"I/O error",		/* EIO */
	"No such device or address",	/* ENXIO */
	"Argument list too long",	/* E2BIG */
	"Executable format error",	/* ENOEXEC */
	"Bad file number",	/* EBADF */
	unknown,
	"Resource temporarily unavailable",	/* EAGAIN */
	"Not enough memory",	/* ENOMEM */
	"Permission denied",	/* EACCES */
	"Bad address",		/* EFAULT */
	"Block device required",	/* ENOTBLK */
	"Resource busy",	/* EBUSY */
	"File exists",		/* EEXIST */
	"Cross-device link",	/* EXDEV */
	"No such device",	/* ENODEV */
	"Not a directory",	/* ENOTDIR */
	"Is a directory",	/* EISDIR */
	"Invalid argument",	/* EINVAL */
	"File table overflow",	/* ENFILE */
	"Too many open files",	/* EMFILE */
	"Not a typewriter",	/* ENOTTY */
	"Text file busy",	/* ETXTBSY */
	"File too large",	/* EFBIG */
	"No space left on device",	/* ENOSPC */
	"Illegal seek",		/* ESPIPE */
	"Read-only file system",	/* EROFS */
	"Too many links",	/* EMLINK */
	"Broken pipe",		/* EPIPE */
	"Math argument",	/* EDOM */
	"Result too large",	/* ERANGE */
	unknown,
	"File name too long",	/* ENAMETOOLONG */
	"No locks available",	/* ENOLCK */
	"Function not implemented",	/* ENOSYS */
	"Directory not empty",	/* ENOTEMPTY */
	"Too many levels of symbolic links",	/* ELOOP */
	unknown,
	unknown,
	"Identifier removed",	/* EIDRM */
	"Illegal byte sequence",	/* EILSEQ */
	"Wrong file format or type",	/* EFTYPE */
	"Value too large to be stored in data type",	/* EOVERFLOW */
	unknown,		/* 47 */
	unknown,		/* 48 */
	unknown,		/* 49 */
	"Invalid packet size",	/* EPACKSIZE */
	"Not enough buffers left",	/* ENOBUFS */
	"Illegal ioctl for device",	/* EBADIOCTL */
	"Bad mode for ioctl",	/* EBADMODE */
	"Would block",		/* EWOULDBLOCK */
	"Network unreachable",	/* ENETUNREACH */
	"Host unreachable",	/* EHOSTUNREACH */
	"Already connected",	/* EISCONN */
	"Address in use",	/* EADDRINUSE */
	"Connection refused",	/* ECONNREFUSED */
	"Connection reset",	/* ECONNRESET */
	"Connection timed out",	/* ETIMEDOUT */
	"Urgent data present",	/* EURG */
	"No urgent data present",	/* ENOURG */
	"No connection",	/* ENOTCONN */
	"Already shutdown",	/* ESHUTDOWN */
	"No such connection",	/* ENOCONN */
	"Address family not supported",	/* EAFNOSUPPORT */
	"Protocol not supported by AF",	/* EPROTONOSUPPORT */
	"Protocol wrong type for socket",	/* EPROTOTYPE */
	"Operation in progress",	/* EINPROGRESS */
	"Address not available",	/* EADDRNOTAVAIL */
	"Connection already in progress",	/* EALREADY */
	"Message too long",	/* EMSGSIZE */
	"Socket operation on non-socket",	/* ENOTSOCK */
	"Protocol not available",	/* ENOPROTOOPT */
	"Operation not supported",	/* EOPNOTSUPP */
	"Network is down",	/* ENETDOWN */
	"Protocol family not supported",	/* EPFNOSUPPORT */
	"Destination address required",	/* EDESTADDRREQ */
	"Host is down",		/* EHOSTDOWN */
	unknown, unknown, unknown, unknown, unknown,
	unknown, unknown, unknown, unknown, unknown,
	unknown, unknown, unknown, unknown, unknown,
	unknown, unknown, unknown,
	"Generic error"
};

const int nstrerror = NELEMENTS(strerrorlist);
