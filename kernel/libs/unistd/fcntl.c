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

#include <fcntl.h>
#include <errno.h>

#include "fdescr_private.h"

int fcntl(int fd, int cmd, ...)
{
	fd_data_t *fdata = fdget(fd);
	int retcode;

	if (!fdata) {
		errno = EINVAL;
		return (-1);
	}

	switch (cmd) {
	case F_DUPFD:		/* duplicate file descriptor */
		break;
	case F_GETFD:		/* get file descriptor flags */
		break;
	case F_SETFD:		/* set file descriptor flags */
		break;
	case F_GETFL:		/* get file status flags */
		retcode = 0;
		if ((fdata->flags & (FD_DATA_IS_R | FD_DATA_IS_W)) == (FD_DATA_IS_R | FD_DATA_IS_W)) {
			retcode = O_RDWR;
		} else if (fdata->flags & FD_DATA_IS_R) {
			retcode = O_RDONLY;
		} else if (fdata->flags & FD_DATA_IS_W) {
			retcode = O_WRONLY;
		}
		return (retcode);
	case F_SETFL:		/* set file status flags */
		break;
	case F_GETOWN:
		/* get the process or process group ID specified to
		 * receive SIGURG signals when out-of-band data is available.
		 * SOCKETS ONLY
		 */
		break;
	case F_SETOWN:
		/* set the process or process group ID specified to
		 * receive SIGURG signals when out-of-band data is available.
		 * SOCKETS ONLY
		 */
		break;
	case F_GETLK:		/* get record locking information */
		break;
	case F_SETLK:		/* set record locking information */
		break;
	case F_SETLKW:		/* set record locking info; wait if blocked */
		break;
	}

	errno = EINVAL;
	return (-1);
}
