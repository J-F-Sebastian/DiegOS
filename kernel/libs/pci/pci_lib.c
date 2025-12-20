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

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <libs/chunks.h>
#include <libs/list.h>
#include <libs/pci.h>
#include <libs/pci_lib.h>
#include <kprintf.h>
#include <diegos/kernel.h>

#include "pci_devices.h"

static chunks_pool_t *pci_dev_pool = NULL;
static list_inst pci_dev_list;
static unsigned dev_counter = 0;

typedef struct pci_dev_list {
	list_node header;
	pci_bus_device_t data;
} pci_dev_list_t;

static const char *get_class_desc(uint8_t class)
{
	/* Base Class is the upper byte of the 3 bytes array */
	if (class < 0x12) {
		return PCI_CLASSES[class];
	} else if (class < 0xFF) {
		return PCI_CLASSES[0x12];
	} else {
		return PCI_CLASSES[0x13];
	}
}

static const char *get_subclass_desc(uint8_t class, uint8_t subclass, uint8_t progif)
{
	const struct pci_subclass *subclassptr;
	unsigned i = 0;

	if (class < 0x12) {
		if (!subclass_desc_tree[class].descr)
			return "N/A";
		subclassptr = subclass_desc_tree[class].descr;
		if (0x00 == class) {
			if (0x00 == subclass)
				return subclassptr[0].descr;
			if (0x01 == subclass)
				return subclassptr[1].descr;
			return "N/A";
		}
		while (i < subclass_desc_tree[class].num) {
			if ((subclassptr->sc == subclass) &&
			    ((subclassptr->pif == progif) || (subclassptr->pif == 0xFF))) {
				return subclassptr->descr;
			}
			subclassptr++;
			i++;
		}
	}
	return "N/A";
}

/*
 * Return TRUE if a function is found as valid.
 * Return FALSE in any other case.
 */
static BOOL pci_bus_enumerate_function(unsigned bus,
				       unsigned device, unsigned function, BOOL * is_multi)
{
	uint32_t buffer[PCI_HEADER_SIZE_REGS];
	pci_config_t *cfgspace = (pci_config_t *) buffer;
	pci_dev_list_t *newdev = NULL;
	bdf_addr_t address = pci_create_bdf(bus, device, function);
	unsigned i;
	const char *pci_class = NULL;
	const char *pci_sub_class = NULL;
	uint8_t cap_ptr = 0;
	uint8_t *class_code;

	pci_read_config(address, buffer);

	if ((INVALID_VID == cfgspace->header.vendorid) &&
	    (INVALID_DID == cfgspace->header.deviceid)) {
		return (FALSE);
	}

	newdev = chunks_pool_malloc(pci_dev_pool);
	if (!newdev) {
		return (FALSE);
	}
	if (EOK != list_prepend(&pci_dev_list, &newdev->header)) {
		chunks_pool_free(pci_dev_pool, newdev);
		return (FALSE);
	}

	newdev->data.system_deviceid = dev_counter++;
	newdev->data.function = function;
	newdev->data.device = device;
	newdev->data.bus = bus;
	newdev->data.vendorid = cfgspace->header.vendorid;
	newdev->data.deviceid = cfgspace->header.deviceid;
	newdev->data.revisionid = cfgspace->header.revisionid;
	newdev->data.msi_msg_data = 0;
	newdev->data.msi_msg_ptr = NULL;
	/*
	 * Check header type and copy BARs from proper union
	 */
	if (cfgspace->header.header_type & HEADER_TYPE_MULTIFUNCTION) {
		*is_multi = TRUE;
	} else {
		*is_multi = FALSE;
	}

	switch (cfgspace->header.header_type & ~HEADER_TYPE_MULTIFUNCTION) {
	case HEADER_TYPE_HOST:
		for (i = 0; i < 6; i++) {
			newdev->data.BAR[i] = cfgspace->type0.BAR[i];
		}
		newdev->data.int_line = cfgspace->type0.int_line;
		newdev->data.int_pin = cfgspace->type0.int_pin;
		/*
		 * Browse capabilities (if supported) and enable MSI.
		 * cap_ptr stores the next position in the config space,
		 * and is word aligned.
		 * This code is messy, neee some polishing...
		 */
		if (cfgspace->header.status & STATUS_CAP_LIST) {
			cap_ptr = cfgspace->type0.cap_pointer & 0xFC;
			while (cap_ptr) {
				if (0x05 == cfgspace->type0.extended[cap_ptr - 64]) {
					/*
					 * MSI is supported ! Store the message data
					 * (MSI identifier) as a multiple of dev_counter
					 * (so it matches the system_deviceid plus 1)
					 * and as a multiple of 32 to accomodate multiple message
					 * capable devices
					 */
					newdev->data.msi_msg_data = dev_counter * 32 +
					    ((cfgspace->type0.extended[cap_ptr - 62] & 0xE) >> 1);
					/* pointer to be done ! */
					break;
				}
				cap_ptr = cfgspace->type0.extended[cap_ptr - 63] & 0xFC;
			}
		}
		break;

	case HEADER_TYPE_PCI2PCI_BRIDGE:
		for (i = 0; i < 2; i++) {
			newdev->data.BAR[i] = cfgspace->type1.BAR[i];
		}
		newdev->data.int_line = cfgspace->type1.int_line;
		newdev->data.int_pin = cfgspace->type1.int_pin;
		break;

	default:
		/* UNSUPPORTED HEADER TYPE !!! */
		list_remove(&pci_dev_list, &newdev->header);
		chunks_pool_free(pci_dev_pool, newdev);
		return (FALSE);
	}

	class_code = cfgspace->header.class_code;
	pci_class = get_class_desc(class_code[CLASS_CODE_CLASS]);
	pci_sub_class = get_subclass_desc(class_code[CLASS_CODE_CLASS],
					  class_code[CLASS_CODE_SUBCLASS],
					  class_code[CLASS_CODE_PROG_IF]);

#if 0
	kprintf("[%u] %.2u:%.2u.%u - (%s, %s) id %#x:%#x,%#x,",
		newdev->data.system_deviceid,
		bus,
		device,
		function,
		pci_class,
		pci_sub_class,
		cfgspace->header.vendorid, cfgspace->header.deviceid, cfgspace->header.revisionid);
	kprintf(" INT %x MSI %x, Type %x\n",
		newdev->data.int_line, newdev->data.msi_msg_data, cfgspace->header.header_type);
#else
	kprintf("[%u] %.2u:%.2u.%u - (%s, %s)\n",
		newdev->data.system_deviceid, bus, device, function, pci_class, pci_sub_class);
#endif
	return (TRUE);
}

static void pci_bus_enumerate_device(unsigned bus, unsigned device)
{
	unsigned fun;
	BOOL is_multi;

	if (pci_bus_enumerate_function(bus, device, 0, &is_multi) && is_multi) {
		for (fun = 1; fun < MAX_PCI_FUNCS; fun++) {
			if (pci_bus_enumerate_function(bus, device, fun, &is_multi)) {
			}
		}
	}
}

static void pci_bus_enumerate_bus(unsigned bus)
{
	unsigned dev;

	for (dev = 0; dev < MAX_PCI_DEVS; dev++) {
		pci_bus_enumerate_device(bus, dev);
	}
}

static void pci_bus_enumerate(void)
{
	unsigned bus;

	for (bus = 0; bus < MAX_PCI_BUSSES; bus++) {
		pci_bus_enumerate_bus(bus);
	}
}

/*
 * Do not support PCI-to-PCI bridges yet
 */
STATUS pci_bus_init()
{
	kprintf("Probing PCI...\n");

	if (EOK != list_init(&pci_dev_list)) {
		kerrprintf("Cannot init PCI devices list");
	}

	if (!pci_bus_check()) {
		kerrprintf("no bus found.\n");
		return (EOK);
	}

	pci_dev_pool = chunks_pool_create("PCI BUS", 0, sizeof(pci_dev_list_t), 8, 8);

	if (!pci_dev_pool) {
		kerrprintf("Unable to allocate PCI chunk pool");
		return (ENOMEM);
	}

	pci_bus_enumerate();

	if (!list_count(&pci_dev_list)) {
		kmsgprintf("No PCI devices detected!\n");
	}

	return (EOK);
}

pci_bus_device_t *pci_bus_find_device(uint16_t vendorid, uint16_t deviceid, pci_bus_device_t * prev)
{
	pci_dev_list_t *cursor = list_head(&pci_dev_list);

	if (prev) {
		while (cursor &&
		       (cursor->data.bus != prev->bus) &&
		       (cursor->data.device != prev->device) &&
		       (cursor->data.function != prev->function)) {
			cursor = (pci_dev_list_t *) cursor->header.next;
		}
	}

	while (cursor) {
		if ((cursor->data.vendorid == vendorid) && (cursor->data.deviceid == deviceid)) {
			return (&cursor->data);
		}
		cursor = (pci_dev_list_t *) cursor->header.next;
	}

	return (NULL);
}
