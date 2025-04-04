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

#ifndef _DEVICE_KEYBOARD_H_
#define _DEVICE_KEYBOARD_H_

/*
 * Function to be used as read access function for keyboard drivers
 * and devices.
 * Please read include/diegos/devices.h for a description of the function,
 * its behavior, returned values, and parameters.
 */
int keyboard_read_fn(void *buf, size_t bytes, void *drv, unsigned unitno);

#endif
