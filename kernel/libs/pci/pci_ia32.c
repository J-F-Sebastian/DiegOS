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
#include <string.h>
#include <ia32/ports.h>

#include "pci.h"

#define PCI_CONFIG_ADDRESS  (0xCF8)
#define PCI_CONFIG_DATA     (0xCFC)

#define PCI_CONFIG_BDF_MASK (0x00FFFF00UL)

static inline bdf_addr_t pci_create_bdf_internal (unsigned busid,
                                                  unsigned deviceid,
                                                  unsigned function)
{
    /* 256 busses */
    busid &= (MAX_PCI_BUSSES - 1);
    /* 32 devices*/
    deviceid &= (MAX_PCI_DEVS - 1);
    /* 8 functions */
    function &= (MAX_PCI_FUNCS - 1);

    return ((bdf_addr_t) ((busid << 16) | (deviceid << 11) | (function << 8)));
}

BOOL pci_bus_check()
{
    unsigned tmp;
    BOOL retcode = FALSE;
    unsigned retries = 10;

    out_dword(PCI_CONFIG_ADDRESS, (unsigned)(1<<31));
    tmp = in_dword(PCI_CONFIG_ADDRESS);
    while ((tmp != (unsigned)(1<<31)) && retries--) {
        tmp = in_dword(PCI_CONFIG_ADDRESS);
    }

    if (tmp == (unsigned)(1 << 31)) retcode = TRUE;

    out_dword(PCI_CONFIG_ADDRESS, tmp);

    return (retcode);
}

bdf_addr_t pci_create_bdf(unsigned busid,
                          unsigned deviceid,
                          unsigned function)
{
    return (pci_create_bdf_internal(busid, deviceid, function));
}

void pci_read_config(bdf_addr_t address, uint32_t *confmem)
{
    unsigned loop;

    if (!confmem) {
        return;
    }

    address &= PCI_CONFIG_BDF_MASK;
    address |= (1<<31);

    out_dword(PCI_CONFIG_ADDRESS, address);
    confmem[0] = in_dword(PCI_CONFIG_DATA);

    /*
     * there is no such thing as a VendorID equal to 0xFFFF,
     * nor a DeviceID equal to 0xFFFF.
     * So it means the device is not present.
     * fill the buffer with 0xFF to stay on the safe side.
     */
    if (0xFFFFFFFFUL == confmem[0]) {
        memset(confmem + 1, 0xFF, PCI_HEADER_SIZE_REGS - sizeof(confmem));
        return;
    }

    address += 4;
    for (loop = 1; loop < PCI_HEADER_SIZE_REGS; loop++, address+=4) {
        out_dword(PCI_CONFIG_ADDRESS, address);
        confmem[loop] = in_dword(PCI_CONFIG_DATA);
    }
}

void pci_write_config(bdf_addr_t address, const uint32_t *confmem)
{
    unsigned loop;

    if (!confmem) {
        return;
    }

    address &= PCI_CONFIG_BDF_MASK;
    address |= (1<<31);

    for (loop = 0; loop < 64; loop++, address+=4) {
        out_dword(PCI_CONFIG_ADDRESS, address);
        out_dword(PCI_CONFIG_DATA, confmem[loop]);
    }
}

static inline void
pci_config_space_set_addr (bdf_addr_t address, unsigned offset)
{
    address &= PCI_CONFIG_BDF_MASK;
    offset &= ~3;
    offset &= (PCI_HEADER_SIZE_BYTES -1);
    address |= offset;
    address |= (1<<31);

    out_dword(PCI_CONFIG_ADDRESS, address);
}

void pci_read_config_reg32(bdf_addr_t address,
                           unsigned offset,
                           uint32_t *value)
{
    if (!value) {
        return;
    }

    pci_config_space_set_addr(address, offset);

    *value = in_dword(PCI_CONFIG_DATA);
}

void pci_write_config_reg32(bdf_addr_t address,
                            unsigned offset,
                            const uint32_t value)
{
    pci_config_space_set_addr(address, offset);

    out_dword(PCI_CONFIG_DATA, value);
}

void pci_read_config_reg16(bdf_addr_t address,
                           unsigned offset,
                           uint16_t *value)
{
    if (!value) {
        return;
    }

    pci_config_space_set_addr(address, offset);

    *value = in_word(PCI_CONFIG_DATA+(offset & 2));
}

void pci_write_config_reg16(bdf_addr_t address,
                            unsigned offset,
                            const uint16_t value)
{
    pci_config_space_set_addr(address, offset);

    out_word(PCI_CONFIG_DATA +(offset & 2), value);
}

void pci_read_config_reg8(bdf_addr_t address,
                          unsigned offset,
                          uint8_t *value)
{
    if (!value) {
        return;
    }

    pci_config_space_set_addr(address, offset);

    *value = in_byte(PCI_CONFIG_DATA+(offset & 3));
}

void pci_write_config_reg8(bdf_addr_t address,
                           unsigned offset,
                           const uint8_t value)
{
    pci_config_space_set_addr(address, offset);

    out_byte(PCI_CONFIG_DATA +(offset & 3), value);
}
