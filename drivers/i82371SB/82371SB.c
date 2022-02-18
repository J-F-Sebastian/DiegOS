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

static pci_bus_device_t *instance[] = { NULL, NULL, NULL, NULL };

static const uint16_t devid[] = {
	i82371SB_BRIDGE_DID,
	i82371SB_IDE_DID,
	i82371SB_USB_DID,
	i82371AB_PWR_DID
};

static const char *names[] = {
	"82371SB PIIX3 PCI-to-ISA Bridge (Triton II)",
	"82371SB PIIX3 IDE Interface (Triton II)",
	"82371SB PIIX3 USB Host Controller (Triton II)",
	"82371AB PIIX3 Power Management"
};

static int i82371sb_init(unsigned unitno)
{
	if (unitno >= NELEMENTS(devid)) {
		return (ENXIO);
	}

	instance[unitno] = pci_bus_find_device(i82371SB_VID, devid[unitno], NULL);

	if (instance[unitno]) {
		kdrvprintf("%u:%u.%u %#x:%#x ==> %s\n",
			   instance[unitno]->bus,
			   instance[unitno]->device,
			   instance[unitno]->function,
			   instance[unitno]->vendorid, instance[unitno]->deviceid, names[unitno]);

		return (EOK);
	}

	return (ENXIO);
}

int i82371sb_start(unsigned unitno)
{
	if ((unitno >= NELEMENTS(devid)) || !instance[unitno]) {
		return (ENXIO);
	}

	return EOK;
}

int i82371sb_stop(unsigned unitno)
{
	if ((unitno >= NELEMENTS(devid)) || !instance[unitno]) {
		return (ENXIO);
	}

	return EOK;
}

int i82371sb_done(unsigned unitno)
{
	if ((unitno >= NELEMENTS(devid)) || !instance[unitno]) {
		return (ENXIO);
	}

	return EOK;
}

static unsigned i82371sb_status(unsigned unitno)
{
	if (unitno >= NELEMENTS(devid)) {
		return 0;
	}
	return (DRV_STATUS_RUN | DRV_IS_CHAR);
}

char_driver_t i82371sb_drv = {
	.cmn = {
		.name = "PIIX3",
		.init_fn = i82371sb_init,
		.start_fn = i82371sb_start,
		.stop_fn = i82371sb_stop,
		.done_fn = i82371sb_done,
		.ioctrl_fn = NULL,
		.status_fn = i82371sb_status,
		.poll_fn = NULL}
	,
	.write_fn = NULL,
	.read_fn = NULL,
	.write_multi_fn = NULL,
	.read_multi_fn = NULL
};
