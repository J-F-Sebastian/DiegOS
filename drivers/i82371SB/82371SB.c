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

#include <diegos/interrupts.h>
#include <diegos/drivers.h>
#include <errno.h>
#include <libs/pci.h>
#include <libs/pci_lib.h>
#include <ia32/ports.h>

#include "82371SB.h"

static pci_bus_device_t *instance = NULL;

static int i82371sb_init (unsigned unitno)
{
    if (unitno) {
        return (ENXIO);
    }

    instance = pci_bus_find_device(i82371SB_VID, i82371SB_BRIDGE_DID, NULL);

    if (instance) {
        return (EOK);
    }

    return (ENXIO);
}

 int i82371sb_start (unsigned unitno)
 {
    if (unitno || !instance) {
        return (ENXIO);
    }

    return EOK;
 }

int i82371sb_stop (unsigned unitno)
{
    if (unitno || !instance) {
        return (ENXIO);
    }

    return EOK;
}

int i82371sb_done (unsigned unitno)
{
  if (unitno || !instance) {
        return (ENXIO);
    }

    return EOK;
}

char_driver_t i82371sb_drv = {
    .name = "PIIX3",
    .init_fn = i82371sb_init,
    .start_fn = i82371sb_start,
    .stop_fn = i82371sb_stop,
    .done_fn = i82371sb_done,
    .write_fn = NULL,
    .read_fn = NULL,
    .write_multi_fn = NULL,
    .read_multi_fn = NULL,
    .ioctrl_fn = NULL,
    .status_fn = NULL
};

