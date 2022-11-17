/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2022 Diego Gallizioli
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
#include <errno.h>

#include "null.h"

/*
 * Handle 1 device system wide
 */
static int null_init(unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	return (EOK);
}

static int null_start(unsigned unitno)
{
	if (unitno) {
		return (EINVAL);
	}

	return (EOK);
}

static int null_stop(unsigned unitno)
{
	if (unitno) {
		return (EINVAL);
	}

	return (EOK);
}

static int null_done(unsigned unitno)
{
	if (unitno) {
		return (EINVAL);
	}

	return (EOK);
}

static int null_ioctrl(void *data, unsigned opcode, unsigned unitno)
{
	return EPERM;
}

static unsigned null_status(unsigned unitno)
{
	if (!unitno) {
		return (DRV_STATUS_RUN);
	}

	return (0);
}

static int null_write(const void *buf, unsigned bytes, unsigned unitno)
{
	if (unitno) {
		return ENXIO;
	}

	return (EOK);
}

static int null_write_multi(const void **buf, const unsigned *bytes, unsigned items,
			    unsigned unitno)
{
	if (unitno) {
		return ENXIO;
	}

	return (EOK);
}

char_driver_t null_drv = {
	.cmn = {
		.name = "null",
		.init_fn = null_init,
		.start_fn = null_start,
		.stop_fn = null_stop,
		.done_fn = null_done,
		.ioctrl_fn = null_ioctrl,
		.status_fn = null_status,
		.poll_fn = NULL}
	,
	.write_fn = null_write,
	.read_fn = NULL,
	.write_multi_fn = null_write_multi,
	.read_multi_fn = NULL
};
