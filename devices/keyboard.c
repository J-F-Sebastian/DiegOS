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

#include <errno.h>
#include <diegos/devices.h>
#include <diegos/drivers.h>

#include "scancodes.h"

int keyboard_read_fn(void *buf, size_t bytes, void *drv, unsigned unitno)
{
	char_driver_t *cdrv;
	int retcode;
	size_t inputcnt = 0;
	char decbuffer;
	char *output = (char *)buf;

	if (!buf || !drv) {
		return (EINVAL);
	}

	cdrv = (char_driver_t *) drv;

	while (inputcnt != bytes) {
		retcode = cdrv->read_fn(&decbuffer, sizeof(decbuffer), unitno);
		if (retcode < 0) {
			return (retcode);
		}
		decbuffer = scancode_to_char(decbuffer);
		if (decbuffer) {
			*output++ = decbuffer;
			inputcnt++;
		}
	}

	return (int)(inputcnt);
}
