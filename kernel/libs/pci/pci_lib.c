#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <chunks.h>
#include <list.h>
#include <pci.h>
#include <pci_lib.h>
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

/* #define PCI_EXT_DBG */
#ifdef PCI_EXT_DBG
static const char *PCI_CLASSES[] = {
    /* 00h */
    "Device was built before Class Code definitions were finalized",
    /* 01h */
    "Mass storage controller",
    /* 02h */
    "Network controller",
    /* 03h */
    "Display controller",
    /* 04h */
    "Multimedia device",
    /* 05h */
    "Memory controller",
    /* 06h */
    "Bridge device",
    /* 07h */
    "Simple communication controllers",
    /* 08h */
    "Base system peripherals",
    /* 09h */
    "Input devices",
    /* 0Ah */
    "Docking stations",
    /* 0Bh */
    "Processors",
    /* 0Ch */
    "Serial bus controllers",
    /* 0Dh */
    "Wireless controller",
    /* 0Eh */
    "Intelligent I/O controllers",
    /* 0Fh */
    "Satellite communication controllers",
    /* 10h */
    "Encryption/Decryption controllers",
    /* 11h */
    "Data acquisition and signal processing controllers",
    /* 12h - FEh Reserved */
    "Reserved",
    /* FFh Device does not fit in any defined classes*/
    "Device does not fit in any defined classes"
};
#endif // PCI_EXT_DBG

/*
 * Return TRUE if a function is found as valid.
 * Return FALSE in any other case.
 */
static BOOL pci_bus_enumerate_function(unsigned bus,
                                       unsigned device,
                                       unsigned function,
                                       BOOL *is_multi)
{
    uint32_t buffer[PCI_HEADER_SIZE_REGS];
    pci_config_t *cfgspace = (pci_config_t *)buffer;
    pci_dev_list_t *newdev = NULL;
    bdf_addr_t address = pci_create_bdf(bus, device, function);
    unsigned i;
#ifdef PCI_EXT_DBG
    unsigned j;
    const struct pci_vendor_node *cursor;
    const struct pci_vendor_device_node *cursor2;
    const char *pci_class = NULL;
#endif
    uint8_t cap_ptr = 0;

    pci_read_config(address, buffer);

    if ((0xFFFF == cfgspace->header.vendorid) &&
            (0xFFFF == cfgspace->header.deviceid)) {
        return (FALSE);
    }

    newdev = chunks_pool_malloc(pci_dev_pool);
    if (!newdev) {
        return (FALSE);
    }
    if (EOK != list_add(&pci_dev_list, NULL, &newdev->header)) {
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

    switch (cfgspace->header.header_type & 0x7F) {
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
                    if (0x05 == cfgspace->type0.extended[cap_ptr - 63]) {
                        /*
                        * MSI is supported ! Store the message data
                        * (MSI identifier) as a multiple of dev_counter
                        * (so it matches the system_deviceid plus 1)
                        * and as a multiple of 32 to accomodate multiple message
                        * capable devices
                        */
                        newdev->data.msi_msg_data = dev_counter*32 +
                                                    ((cfgspace->type0.extended[cap_ptr - 62] & 0xE) >> 1);
                        /* pointer to be done ! */
                        break;
                    }
                    cap_ptr = cfgspace->type0.extended[cap_ptr - 64] & 0xFC;
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

#ifdef PCI_EXT_DBG
    /* Base Class is the upper byte of the 3 bytes array */
    if (cfgspace->header.class_code[2] < 0x12) {
        pci_class = PCI_CLASSES[cfgspace->header.class_code[2]];
    } else if (cfgspace->header.class_code[2] < 0xFF) {
        pci_class = PCI_CLASSES[0x12];
    } else {
        pci_class = PCI_CLASSES[0x13];
    }

    kprintf("[%u] bdf %.2u:%.2u.%u - (%s) id %#x:%#x, rev %#x,"
            " INT line %#x MSI %#x,  Hdr Type %#x\n",
            newdev->data.system_deviceid,
            bus,
            device,
            function,
            pci_class,
            cfgspace->header.vendorid,
            cfgspace->header.deviceid,
            cfgspace->header.revisionid,
            newdev->data.int_line,
            newdev->data.msi_msg_data,
            cfgspace->header.header_type);

    /*
     * This can be slow, so switch it off if you need more speed on
     * booting. The right solution would be to let drivers print their
     * own device name - and they should know best - but unknown/undetected
     * devices would go unnoticed...
     * NOTE: If you fail sorting the data structures defined in
     * pci_devices.h and accessed from pci_known_devices_list, the following
     * algorith will fail.
     */

    cursor = pci_known_devices_list;
    for (i = 0;
            (cursor[i].header.id < cfgspace->header.vendorid) &&
            (cursor[i].header.id != INVALID_VID);
            i++) {};

    if (cursor[i].header.id == cfgspace->header.vendorid) {
        for (j = 0, cursor2 = cursor[i].devices;
                (cursor2[j].id < cfgspace->header.deviceid) &&
                (cursor2[j].id != INVALID_DID);
                j++) {};

        if (cursor2[j].id == cfgspace->header.deviceid) {
            kprintf("\t\t\t%s %s\n",
                    cursor[i].header.description,
                    cursor2[j].description);
        }
    }
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

static void pci_bus_enumerate (void)
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

    pci_dev_pool = chunks_pool_create("PCI BUS",
                                      0,
                                      sizeof(pci_dev_list_t),
                                      8,
                                      8);

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

pci_bus_device_t *pci_bus_find_device(uint16_t vendorid,
                                      uint16_t deviceid,
                                      pci_bus_device_t *prev)
{
    pci_dev_list_t *cursor = list_head(&pci_dev_list);

    if (prev) {
        while (cursor &&
                (cursor->data.bus != prev->bus) &&
                (cursor->data.device != prev->device) &&
                (cursor->data.function != prev->function)) {
            cursor = (pci_dev_list_t *)cursor->header.next;
        }
    }

    while (cursor) {
        if ((cursor->data.vendorid == vendorid) &&
                (cursor->data.deviceid == deviceid)) {
            return (&cursor->data);
        }
        cursor = (pci_dev_list_t *)cursor->header.next;
    }

    return (NULL);
}

