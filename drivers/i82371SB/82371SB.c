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

#include <diegos/hal_interrupts.h>
#include <diegos/drivers.h>
#include <errno.h>
#include <libs/pci.h>
#include <libs/pci_lib.h>
#include <processor/ports.h>

#include "82371SB.h"

static pci_bus_device_t *instance[] = { NULL, NULL, NULL, NULL };

static const uint16_t devid[] = {
	i82371SB_BRIDGE_DID,
	i82371AB_PWR_DID,
	i82371SB_IDE_DID,
	i82371SB_USB_DID
};

static const char *names[] = {
	"82371SB PIIX3 PCI-to-ISA Bridge (Triton II)",
	"82371AB PIIX3 Power Management" "82371SB PIIX3 IDE Interface (Triton II)",
	"82371SB PIIX3 USB Host Controller (Triton II)"
};

static unsigned status[] = {
	DRV_STATUS_RUN | DRV_IS_CHAR,
	DRV_STATUS_RUN | DRV_IS_CHAR,
	DRV_STATUS_RUN | DRV_IS_BLOCK,
	DRV_STATUS_RUN | DRV_IS_NET
};

/*
 * The driver accepts and supports 4 units corresponding to 4 device
 * functions
 *
 * ISA Bridge
 * IDE Interface
 * USB Host
 * Power Management
 */

/******************************************/
/*    ISA Bridge and Power Management     */
/******************************************/

static int i82371sb_mfn_init(unsigned unitno)
{
	uint16_t value = 0;
	pci_bus_device_t *p;

	if (unitno > 1) {
		return (ENXIO);
	}

	if (instance[unitno]) {
		return (EPERM);
	}

	p = instance[unitno] = pci_bus_find_device(i82371SB_VID, devid[unitno], NULL);

	if (p) {
		/*
		 * Read the PCICMD register
		 */
		pci_read_config_reg16(pci_create_bdf(p->bus, p->device, p->function), 4, &value);

		kdrvprintf("%u:%u.%u %#x:%#x ==> %s\n",
			   p->bus, p->device, p->function, p->vendorid, p->deviceid, names[unitno]);

		return (EOK);
	}

	return (ENXIO);
}

static int i82371sb_mfn_start(unsigned unitno)
{
	if (unitno > 1) {
		return (ENXIO);
	}

	if (!instance[unitno]) {
		return (EACCES);
	}

	return (EOK);
}

static int i82371sb_mfn_stop(unsigned unitno)
{
	if (unitno > 1) {
		return (ENXIO);
	}

	if (!instance[unitno]) {
		return (EACCES);
	}

	return (EOK);
}

static int i82371sb_mfn_done(unsigned unitno)
{
	if (unitno > 1) {
		return (ENXIO);
	}

	if (!instance[unitno]) {
		return (EACCES);
	}

	return (EOK);
}

static unsigned i82371sb_mfn_status(unsigned unitno)
{
	if (unitno > 1) {
		return 0;
	}
	return (status[unitno]);
}

static short i82371sb_mfn_poll(unsigned unitno, poll_table_t *table)
{
	if (unitno > 1) {
		return (ENXIO);
	}

	if (!instance[unitno]) {
		return (EACCES);
	}

	return (EOK);
}

static int i82371sb_mfn_ioctrl(void *arg, unsigned cmd, unsigned unitno)
{
	if (unitno > 1) {
		return (ENXIO);
	}

	if (!instance[unitno]) {
		return (EACCES);
	}

	return (EOK);
}

static int i82371sb_mfn_write(const void *buf, unsigned *bytes, unsigned unitno)
{
	if (unitno > 1) {
		return (ENXIO);
	}

	if (!instance[unitno]) {
		return (EACCES);
	}

	return (EIO);
}

static int i82371sb_mfn_read(void *buf, unsigned *bytes, unsigned unitno)
{
	if (unitno > 1) {
		return (ENXIO);
	}

	if (!instance[unitno]) {
		return (EACCES);
	}

	return (EIO);
}

/******************************************/
/*      IDE bus master controller         */
/******************************************/

static int i82371sb_ide_init(unsigned unitno)
{
	uint16_t value = 0;
	pci_bus_device_t *p;

	if (unitno) {
		return (ENXIO);
	}

	if (instance[2]) {
		return (EPERM);
	}

	p = instance[2] = pci_bus_find_device(i82371SB_VID, devid[2], NULL);

	if (p) {
		/*
		 * Read the PCICMD register
		 */
		pci_read_config_reg16(pci_create_bdf(p->bus, p->device, p->function), 4, &value);

		/* IDE Interface */
		/* USB Host */
		kdrvprintf("%u:%u.%u %#x:%#x ==> %s\n\t\tports %s - Bus Master %s\n",
			   p->bus,
			   p->device,
			   p->function,
			   p->vendorid,
			   p->deviceid,
			   names[2],
			   (value & 1) ? "enabled" : "disabled",
			   (value & 4) ? "enabled" : "disabled");

		return (EOK);
	}

	return (ENXIO);
}

static int i82371sb_ide_start(unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	if (!instance[2]) {
		return (EACCES);
	}

	return (EOK);
}

static int i82371sb_ide_stop(unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	if (!instance[2]) {
		return (EACCES);
	}

	return (EOK);
}

static int i82371sb_ide_done(unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	if (!instance[2]) {
		return (EACCES);
	}

	return (EOK);
}

static unsigned i82371sb_ide_status(unsigned unitno)
{
	if (unitno) {
		return 0;
	}
	return (status[2]);
}

static short i82371sb_ide_poll(unsigned unitno, poll_table_t *table)
{
	if (unitno) {
		return (ENXIO);
	}

	if (!instance[2]) {
		return (EACCES);
	}

	return (EOK);
}

static int i82371sb_ide_ioctrl(void *arg, unsigned cmd, unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	if (!instance[2]) {
		return (EACCES);
	}

	return (EOK);
}

static int i82371sb_ide_write(const void *buf, unsigned *bytes, unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	if (!instance[2]) {
		return (EACCES);
	}

	return (EIO);
}

static int i82371sb_ide_read(void *buf, unsigned *bytes, unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	if (!instance[2]) {
		return (EACCES);
	}

	return (EIO);
}

/******************************************/
/*      USB bus master controller         */
/******************************************/

static int i82371sb_usb_init(unsigned unitno)
{
	uint16_t value = 0;
	pci_bus_device_t *p;

	if (unitno) {
		return (ENXIO);
	}

	if (instance[3]) {
		return (EPERM);
	}

	p = instance[3] = pci_bus_find_device(i82371SB_VID, devid[3], NULL);

	if (p) {
		/*
		 * Read the PCICMD register
		 */
		pci_read_config_reg16(pci_create_bdf(p->bus, p->device, p->function), 4, &value);

		/* USB Host */
		kdrvprintf("%u:%u.%u %#x:%#x ==> %s\n\t\tports %s - Bus Master %s\n",
			   p->bus,
			   p->device,
			   p->function,
			   p->vendorid,
			   p->deviceid,
			   names[3],
			   (value & 1) ? "enabled" : "disabled",
			   (value & 4) ? "enabled" : "disabled");

		return (EOK);
	}

	return (ENXIO);
}

static int i82371sb_usb_start(unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	if (!instance[3]) {
		return (EACCES);
	}

	return (EOK);
}

static int i82371sb_usb_stop(unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	if (!instance[3]) {
		return (EACCES);
	}

	return (EOK);
}

static int i82371sb_usb_done(unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	if (!instance[3]) {
		return (EACCES);
	}

	return (EOK);
}

static unsigned i82371sb_usb_status(unsigned unitno)
{
	if (unitno) {
		return 0;
	}
	return (status[3]);
}

static short i82371sb_usb_poll(unsigned unitno, poll_table_t *table)
{
	if (unitno) {
		return (ENXIO);
	}

	if (!instance[3]) {
		return (EACCES);
	}

	return (EOK);
}

static int i82371sb_usb_ioctrl(void *arg, unsigned cmd, unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	if (!instance[3]) {
		return (EACCES);
	}

	return (EOK);
}

static int i82371sb_usb_write(const void *buf, unsigned *bytes, unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	if (!instance[3]) {
		return (EACCES);
	}

	return (EIO);
}

static int i82371sb_usb_read(void *buf, unsigned *bytes, unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	if (!instance[3]) {
		return (EACCES);
	}

	return (EIO);
}

char_driver_t i82371sb_mfn_drv = {
	.cmn = {
		.name = "PIIX3-MFN",
		.init_fn = i82371sb_mfn_init,
		.start_fn = i82371sb_mfn_start,
		.stop_fn = i82371sb_mfn_stop,
		.done_fn = i82371sb_mfn_done,
		.ioctrl_fn = i82371sb_mfn_ioctrl,
		.status_fn = i82371sb_mfn_status,
		.poll_fn = i82371sb_mfn_poll}
	,
	.write_fn = i82371sb_mfn_write,
	.read_fn = i82371sb_mfn_read,
	.write_multi_fn = NULL,
	.read_multi_fn = NULL
};

char_driver_t i82371sb_ide_drv = {
	.cmn = {
		.name = "PIIX3-IDE",
		.init_fn = i82371sb_ide_init,
		.start_fn = i82371sb_ide_start,
		.stop_fn = i82371sb_ide_stop,
		.done_fn = i82371sb_ide_done,
		.ioctrl_fn = i82371sb_ide_ioctrl,
		.status_fn = i82371sb_ide_status,
		.poll_fn = i82371sb_ide_poll}
	,
	.write_fn = i82371sb_ide_write,
	.read_fn = i82371sb_ide_read,
	.write_multi_fn = NULL,
	.read_multi_fn = NULL
};

char_driver_t i82371sb_usb_drv = {
	.cmn = {
		.name = "PIIX3-USB",
		.init_fn = i82371sb_usb_init,
		.start_fn = i82371sb_usb_start,
		.stop_fn = i82371sb_usb_stop,
		.done_fn = i82371sb_usb_done,
		.ioctrl_fn = i82371sb_usb_ioctrl,
		.status_fn = i82371sb_usb_status,
		.poll_fn = i82371sb_usb_poll}
	,
	.write_fn = i82371sb_usb_write,
	.read_fn = i82371sb_usb_read,
	.write_multi_fn = NULL,
	.read_multi_fn = NULL
};
