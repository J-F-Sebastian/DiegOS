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

#include <diegos/devices.h>
#include <diegos/interrupts.h>

static device_t *ttydev = NULL;

/*
 * kputb won't take semaphores, mutexes, or any other system call.
 * output will go through anyway for debugging purposes.
 */
void kputb(char *buffer, unsigned bytes)
{
    if (!ttydev) {
        ttydev = device_lookup_name(DEFAULT_DBG_TTY);
        if (!ttydev) {
            return;
        }
    }

    ttydev->cdrv->write_fn(buffer, bytes, 0);
}
