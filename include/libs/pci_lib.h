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

#ifndef _PCI_LIB_H_
#define _PCI_LIB_H_

#include <types_common.h>

typedef struct pci_bus_device {
	uint16_t function:3, device:5, bus:8;
	uint16_t vendorid;
	uint16_t deviceid;
	uint8_t revisionid;
	uint8_t system_deviceid;
	uint8_t class_code[3];
	uint32_t CIS_pointer;
	/* 16 bytes */
	/*
	 *       Memory Space BAR Layout
	 * |31 ----------------------- 4 | 3                | 2 - 1 | 0 
	 * |16-Byte Aligned Base Address | Prefetchable     | Type  | Always 0

	 *     I/O Space BAR Layout
	 * |31 - 2                      | 1        | 0
	 * |4-Byte Aligned Base Address     | Reserved | Always 1
	 */
	uint32_t BAR[6];
	/* 40 bytes */
	uint8_t int_pin;
	uint8_t int_line;
	uint16_t msi_msg_data;
	uint32_t *msi_msg_ptr;
	/* 48 bytes */
} pci_bus_device_t;

inline void pci_device_read(volatile uint32_t *bar, unsigned offset, uint32_t *value)
{
	*value = *(bar + offset / 4);
}

inline void pci_device_write(volatile uint32_t *bar, unsigned offset, const uint32_t value)
{
	*(bar + offset / 4) = value;
}

inline void
pci_device_read_multi(volatile uint32_t *bar, unsigned offset, uint32_t *values, unsigned numregs)
{
	bar += offset / 4;
	while (numregs--) {
		*values++ = *bar++;
	}
}

inline void
pci_device_write_multi(volatile uint32_t *bar,
		       unsigned offset, const uint32_t *values, unsigned numregs)
{
	bar += offset / 4;
	while (numregs--) {
		*bar++ = *values++;
	}
}

inline void pci_device_readb(volatile uint8_t *bar, unsigned offset, uint8_t *value)
{
	*value = *(bar + offset);
}

inline void pci_device_writeb(volatile uint8_t *bar, unsigned offset, const uint8_t value)
{
	*(bar + offset) = value;
}

inline void
pci_device_read_multib(volatile uint8_t *bar, unsigned offset, uint8_t *values, unsigned numregs)
{
	bar += offset;
	while (numregs--) {
		*values++ = *bar++;
	}
}

inline void
pci_device_write_multib(volatile uint8_t *bar,
			unsigned offset, const uint8_t *values, unsigned numregs)
{
	bar += offset;
	while (numregs--) {
		*bar++ = *values++;
	}
}

STATUS pci_bus_init(void);

pci_bus_device_t *pci_bus_find_device(uint16_t vendorid,
				      uint16_t deviceid, pci_bus_device_t * prev);

#endif
