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
#include <diegos/drivers.h>

#include "kprintf.h"
#include "drivers_private.h"

int driver_def_ok(unsigned unitno)
{
	return (EOK);
}

int driver_def_error(unsigned unitno)
{
	return (EGENERIC);
}

int kdrvprintf(const char *fmt, ...)
{
	int n;
	va_list ap;

	va_start(ap, fmt);
	kvprintf("[DRV]: ", ap);
	n = kvprintf(fmt, ap);
	va_end(ap);

	return (n);
}

/*
 * Private section
 */

BOOL init_drivers_lib()
{
	return (TRUE);
}
