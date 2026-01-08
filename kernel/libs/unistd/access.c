/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2025 Diego Gallizioli
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

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "fdescr_private.h"

#define DEV_PATH "/dev"

int access(const char *path, int amode)
{
	device_t *dev;
	int tmode;

	if (!path) {
		errno = EINVAL;
		return (-1);
	}

	if (amode & ~(R_OK | W_OK | X_OK | F_OK)) {
		errno = EINVAL;
		return (-1);
	}

	/*
	 * Only devices right now!
	 */
	if (strncmp(path, DEV_PATH, sizeof(DEV_PATH) - 1)) {
		errno = EACCES;
		return (-1);
	}

	dev = device_lookup_name(path + sizeof(DEV_PATH));
	if (!dev) {
		errno = ENOENT;
		return (-1);
	}

	if (amode == F_OK) {
		return (0);
	}

	if (amode & X_OK) {
		errno = EACCES;
		return (-1);
	}

	tmode = 0;
	if (dev->header.read_fn) {
		tmode |= R_OK;
	}

	if (dev->header.write_fn) {
		tmode |= W_OK;
	}

	if ((amode & tmode) != amode) {
		errno = EACCES;
		return (-1);
	}

	return (0);
}
