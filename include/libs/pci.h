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

#ifndef _PCI_H_
#define _PCI_H_

/*
 * This header file is implementing the standard PCI 2.x
 */

/* PCI 2.x bus limitations */
#define MAX_PCI_BUSSES  (256)
#define MAX_PCI_DEVS    (32)
#define MAX_PCI_FUNCS   (8)

#define INVALID_VID     (0xFFFF)
#define INVALID_DID     (0xFFFF)

/* header size */
#define PCI_HEADER_SIZE_BYTES       (256)
#define PCI_HEADER_SIZE_REGS        (64)

/* header type register */
enum {
	HEADER_TYPE_HOST = 0x00,
	HEADER_TYPE_PCI2PCI_BRIDGE = 0x01,
	HEADER_TYPE_CARDBUS_BRIDGE = 0x02,
	HEADER_TYPE_MULTIFUNCTION = 0x80
};

/* command register */
enum {
	COMMAND_IO_SPACE = 1 << 0,
	COMMAND_MEM_SPACE = 1 << 1,
	COMMAND_BUS_MASTER = 1 << 2,
	COMMAND_SPEC_CYCLES = 1 << 3,
	COMMAND_MEM_WI_ENA = 1 << 4,
	COMMAND_VGA_SNOOP = 1 << 5,
	COMMAND_PARITY_ERR = 1 << 6,
	COMMAND_SERR_ENA = 1 << 8,
	COMMAND_B2B_ENA = 1 << 9,
	COMMAND_INT_DISABLE = 1 << 10
};

/* status register */
enum {
	STATUS_INT_STATUS = 1 << 3,
	STATUS_CAP_LIST = 1 << 4,
	STATUS_66MHZ_CAP = 1 << 5,
	STATUS_B2B_CAP = 1 << 7,
	STATUS_MS_D_PAR_ERR = 1 << 8,
	STATUS_DEVSEL_FAST = 0 << 9,
	STATUS_DEVSEL_MED = 1 << 9,
	STATUS_DEVSEL_SLOW = 2 << 9,
	STATUS_SIG_TGT_ABORT = 1 << 11,
	STATUS_RCV_TGT_ABORT = 1 << 12,
	STATUS_RCV_MS_ABORT = 1 << 13,
	STATUS_SIG_SYS_ERROR = 1 << 14,
	STATUS_PARITY_ERROR = 1 << 15
};

/* BIST register */
enum {
	BIST_START = 1 << 6,
	BIST_CAPABLE = 1 << 7
};

/* CLASS CODE register */
enum {
	CLASS_CODE_CLASS = 2,
	CLASS_CODE_SUBCLASS = 1,
	CLASS_CODE_PROG_IF = 0
};

typedef struct pci_config_space_header {
	uint16_t vendorid;
	uint16_t deviceid;
	uint16_t command;
	uint16_t status;
	uint8_t revisionid;
	uint8_t class_code[3];
	uint8_t cacheline_size;
	uint8_t latency_timer;
	uint8_t header_type;
	uint8_t BIST;
	/* 16 bytes */
} pci_config_space_header_t;

/* interrupt pin register */
enum {
	INTERRUPT_PIN_INTA = 0x01,
	INTERRUPT_PIN_INTB = 0x02,
	INTERRUPT_PIN_INTC = 0x03,
	INTERRUPT_PIN_INTD = 0x04
};

/* BAR registers */
enum {
	BAR_IO_MAPPED = 1 << 0,
	BAR_32_BIT = 0 << 1,
	BAR_64_BIT = 2 << 1,
	BAR_PREFETCH = 1 << 3
};

#define BAR_MEM_SPACE_MASK  (0xFFFFFFF0)
#define BAR_IO_SPACE_MASK   (0xFFFFFFFC)

typedef struct pci_config_space_type0 {
	uint32_t BAR[6];
	uint32_t CIS_pointer;
	uint16_t subsystem_vendorid;
	uint16_t subsystem_id;
	/* 48 bytes */
	uint32_t ROM_base_address;
	uint32_t cap_pointer:8, reserved:24;
	uint32_t reserved1;
	uint8_t int_line;
	uint8_t int_pin;
	uint8_t min_gnt;
	uint8_t max_latency;
	/* 64 bytes */
	uint8_t extended[192];
	/* 256 bytes */
} pci_config_space_type0_t;

typedef struct pci_config_space_type1 {
	uint32_t BAR[2];
	uint8_t primary_bus_num;
	uint8_t secondary_bus_num;
	uint8_t subordinate_bus_num;
	uint8_t secondary_lat_timer;
	uint8_t IO_base;
	uint8_t IO_limit;
	uint16_t secondary_status;
	uint16_t memory_base;
	uint16_t memory_limit;
	uint16_t prefetch_memory_base;
	uint16_t prefetch_memory_limit;
	uint32_t prefetch_base_upper_32_bits;
	uint32_t prefetch_limit_upper_32_bits;
	uint16_t IO_base_upper_16_bits;
	uint16_t IO_limit_upper_16_bits;
	uint8_t cap_pointer;
	uint8_t reserved[3];
	uint32_t ROM_base_address;
	uint8_t int_line;
	uint8_t int_pin;
	uint16_t bridge_control;
	/* 64 bytes */
	uint8_t extended[192];
	/* 256 bytes */
} pci_config_space_type1_t;

typedef struct pci_config {
	pci_config_space_header_t header;
	union {
		pci_config_space_type0_t type0;
		pci_config_space_type1_t type1;
	};
} pci_config_t;

typedef uint32_t bdf_addr_t;

BOOL pci_bus_check(void);

/*
 * Returns a coded PCI address; the value is a bitwise compression
 * of Bus, Device and Function IDs.
 *
 * PARAMETERS IN
 * unsigned busid       - the Bus number
 * unsigned deviceid    - the Device number
 * unsigned function    - the Function number in the device
 *
 * RETURNS
 * A coded B/D/F value
 */
bdf_addr_t pci_create_bdf(unsigned busid, unsigned deviceid, unsigned function);

/*
 * Read the 256 bytes long configuration space from a specific device
 * into the provided buffer.
 *
 * PARAMETERS IN
 * bdf_addr_t address   - The coded device address, as returned by
 *                       pci_create_bdf()
 * uint32_t *confmem    - the receiving buffer
 */
void pci_read_config(bdf_addr_t address, uint32_t * confmem);

/*
 * Write the 256 bytes long configuration space from a provided buffer
 * into a specific device.
 *
 * PARAMETERS IN
 * bdf_addr_t address   - The coded device address, as returned by
 *                       pci_create_bdf()
 * uint32_t *confmem    - the source buffer
 */
void pci_write_config(bdf_addr_t address, const uint32_t * confmem);

/*
 * The following API give access to single double words or words inside
 * into a specific device.
 *
 * PARAMETERS IN
 * bdf_addr_t address   - The coded device address, as returned by
 *                       pci_create_bdf()
 * uint32_t *confmem    - the source buffer
 */
void pci_read_config_reg32(bdf_addr_t address, unsigned offset, uint32_t * value);

void pci_read_config_reg16(bdf_addr_t address, unsigned offset, uint16_t * value);

void pci_read_config_reg8(bdf_addr_t address, unsigned offset, uint8_t * value);

void pci_write_config_reg32(bdf_addr_t address, unsigned offset, const uint32_t value);

void pci_write_config_reg16(bdf_addr_t address, unsigned offset, const uint16_t value);

void pci_write_config_reg8(bdf_addr_t address, unsigned offset, const uint8_t value);

#endif
