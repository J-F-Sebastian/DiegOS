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

#ifndef PCI_DEVICES_H_INCLUDED
#define PCI_DEVICES_H_INCLUDED

struct pci_subclass {
	uint8_t sc, pif;
	const char *descr;
};

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

static const struct pci_subclass PCI_CLASS0[] = {
	{
		.sc = 0x00,
		.pif = 0x00,
		.descr = "Not VGA-Compatible device"
	},
	{
		.sc = 0x01,
		.pif = 0x00,
		.descr = "VGA-Compatible device"
	}
};

static const struct pci_subclass PCI_CLASS1[] = {
	{
		.sc = 0x00,
		.pif = 0x00,
		.descr = "SCSI Controller"
	},
	{
		.sc = 0x01,
		.pif = 0xFF,
		.descr = "IDE Controller"
	},
	{
		.sc = 0x02,
		.pif = 0x00,
		.descr = "Floppy Controller"
	},
	{
		.sc = 0x03,
		.pif = 0x00,
		.descr = "IPI Controller"
	},
	{
		.sc = 0x04,
		.pif = 0x00,
		.descr = "RAID Controller"
	},
	{
		.sc = 0x05,
		.pif = 0x20,
		.descr = "ATA Controller with single DMA"
	},	
	{
		.sc = 0x05,
		.pif = 0x30,
		.descr = "ATA Controller with chained DMA"
	},
	{
		.sc = 0x80,
		.pif = 0x00,
		.descr = "Generic mass storage controller"
	}
};

static const struct pci_subclass PCI_CLASS2[] = {
	{
		.sc = 0x00,
		.pif = 0x00,
		.descr = "Ethernet Controller"
	},
	{
		.sc = 0x01,
		.pif = 0x00,
		.descr = "Token Ring Controller"
	},
	{
		.sc = 0x02,
		.pif = 0x00,
		.descr = "FDDI Controller"
	},
	{
		.sc = 0x03,
		.pif = 0x00,
		.descr = "ATM Controller"
	},
	{
		.sc = 0x04,
		.pif = 0x00,
		.descr = "ISDN Controller"
	},
	{
		.sc = 0x05,
		.pif = 0x00,
		.descr = "WorldFip Controller"
	},		
	{
		.sc = 0x80,
		.pif = 0x00,
		.descr = "Generic network controller"
	}
};

static const struct pci_subclass PCI_CLASS3[] = {
	{
		.sc = 0x00,
		.pif = 0x00,
		.descr = "VGA Controller"
	},
	{
		.sc = 0x00,
		.pif = 0x01,
		.descr = "8514-compatible Controller"
	},
	{
		.sc = 0x01,
		.pif = 0x00,
		.descr = "XGA Controller"
	},
	{
		.sc = 0x02,
		.pif = 0x00,
		.descr = "3D Controller"
	},	
	{
		.sc = 0x80,
		.pif = 0x00,
		.descr = "Generic display controller"
	}
};

static const struct pci_subclass PCI_CLASS4[] = {
	{
		.sc = 0x00,
		.pif = 0x00,
		.descr = "Video Device"
	},
	{
		.sc = 0x01,
		.pif = 0x00,
		.descr = "Audio Device"
	},
	{
		.sc = 0x02,
		.pif = 0x00,
		.descr = "Computer Telephony Device"
	},	
	{
		.sc = 0x80,
		.pif = 0x00,
		.descr = "Generic multimedia controller"
	}
};

static const struct pci_subclass PCI_CLASS5[] = {
	{
		.sc = 0x00,
		.pif = 0x00,
		.descr = "RAM memory Controller"
	},
	{
		.sc = 0x01,
		.pif = 0x00,
		.descr = "Flash memory Controller"
	},	
	{
		.sc = 0x80,
		.pif = 0x00,
		.descr = "Generic memory controller"
	}
};
	
static const struct pci_subclass PCI_CLASS6[] = {
	{
		.sc = 0x00,
		.pif = 0x00,
		.descr = "Host/PCI Bridge"
	},
	{
		.sc = 0x01,
		.pif = 0x00,
		.descr = "PCI/ISA Bridge"
	},	
		{
		.sc = 0x02,
		.pif = 0x00,
		.descr = "PCI/EISA Bridge"
	},
	{
		.sc = 0x03,
		.pif = 0x00,
		.descr = "PCI/MicroChannel Bridge"
	},		
	{
		.sc = 0x04,
		.pif = 0x00,
		.descr = "PCI/PCI Bridge"
	},
	{
		.sc = 0x04,
		.pif = 0x01,
		.descr = "Subtractive decode PCI/PCI Bridge"
	},		
	{
		.sc = 0x05,
		.pif = 0x00,
		.descr = "PCI/PCMCIA Bridge"
	},
	{
		.sc = 0x06,
		.pif = 0x00,
		.descr = "PCI/NuBus Bridge"
	},		
	{
		.sc = 0x07,
		.pif = 0x00,
		.descr = "PCI/CardBus Bridge"
	},
	{
		.sc = 0x08,
		.pif = 0xFF,
		.descr = "RACEway Bridge"
	},	
	{
		.sc = 0x09,
		.pif = 0x40,
		.descr = "Semi-transparent PCI/PCI Bridge primary bus"
	},
	{
		.sc = 0x09,
		.pif = 0x80,
		.descr = "Semi-transparent PCI/PCI Bridge secondary bus"
	},	
	{
		.sc = 0x0A,
		.pif = 0x00,
		.descr = "InfiniBand/PCI host Bridge"
	},	
	{
		.sc = 0x80,
		.pif = 0x00,
		.descr = "Generic bridge"
	}
};
 
static const struct pci_subclass PCI_CLASS7[] = {
	{
		.sc = 0x00,
		.pif = 0x00,
		.descr = "Generic XT-compatible serial controller"
	},
	{
		.sc = 0x00,
		.pif = 0x01,
		.descr = "16450-compatible serial controller"
	},	
		{
		.sc = 0x00,
		.pif = 0x02,
		.descr = "16550-compatible serial controller"
	},
	{
		.sc = 0x00,
		.pif = 0x03,
		.descr = "16650-compatible serial controller"
	},		
	{
		.sc = 0x00,
		.pif = 0x04,
		.descr = "16750-compatible serial controller"
	},
	{
		.sc = 0x00,
		.pif = 0x05,
		.descr = "16850-compatible serial controller"
	},		
	{
		.sc = 0x00,
		.pif = 0x06,
		.descr = "16950-compatible serial controller"
	},
	{
		.sc = 0x01,
		.pif = 0x00,
		.descr = "Parallel port"
	},		
	{
		.sc = 0x01,
		.pif = 0x01,
		.descr = "Bi-Directional Parallel port"
	},
	{
		.sc = 0x01,
		.pif = 0x02,
		.descr = "ECP 1.X compliant parallel port"
	},	
	{
		.sc = 0x01,
		.pif = 0x03,
		.descr = "IEEE 1284 controller"
	},
	{
		.sc = 0x01,
		.pif = 0xFE,
		.descr = "IEEE 1284 target device"
	},	
	{
		.sc = 0x02,
		.pif = 0x00,
		.descr = "Multiport serial controller"
	},	
	{
		.sc = 0x03,
		.pif = 0x00,
		.descr = "Generic modem"
	},		
	{
		.sc = 0x03,
		.pif = 0x01,
		.descr = "Hayes compatible modem, 16450 interface"
	},
	{
		.sc = 0x03,
		.pif = 0x02,
		.descr = "Hayes compatible modem, 16550 interface"
	},	
	{
		.sc = 0x03,
		.pif = 0x03,
		.descr = "Hayes compatible modem, 16650 interface"
	},
	{
		.sc = 0x03,
		.pif = 0x04,
		.descr = "Hayes compatible modem, 16750 interface"
	},	
	{
		.sc = 0x04,
		.pif = 0x00,
		.descr = "GPIB (IEEE 488.1/2) controller"
	},	
	{
		.sc = 0x05,
		.pif = 0x00,
		.descr = "Smart Card"
	},		
	{
		.sc = 0x80,
		.pif = 0x00,
		.descr = "Generic communication device"
	}
};
 
static const struct pci_subclass PCI_CLASS8[] = {
	{
		.sc = 0x00,
		.pif = 0x00,
		.descr = "Generic 8259 PIC"
	},
	{
		.sc = 0x00,
		.pif = 0x01,
		.descr = "ISA PIC"
	},	
	{
		.sc = 0x00,
		.pif = 0x02,
		.descr = "EISA PIC"
	},
	{
		.sc = 0x00,
		.pif = 0x10,
		.descr = "I/O APIC"
	},
	{
		.sc = 0x00,
		.pif = 0x20,
		.descr = "I/O(x) PIC"
	},
	{
		.sc = 0x01,
		.pif = 0x00,
		.descr = "Generic 8237 DMA controller"
	},
	{
		.sc = 0x01,
		.pif = 0x01,
		.descr = "ISA DMA controller"
	},
	{
		.sc = 0x01,
		.pif = 0x02,
		.descr = "EISA DMA controller"
	},
	{
		.sc = 0x02,
		.pif = 0x00,
		.descr = "Generic 8254 system timer"
	},
	{
		.sc = 0x02,
		.pif = 0x01,
		.descr = "ISA system timer"
	},
	{
		.sc = 0x02,
		.pif = 0x02,
		.descr = "EISA system timers"
	},
	{
		.sc = 0x03,
		.pif = 0x00,
		.descr = "Generic RTC controller"
	},
	{
		.sc = 0x03,
		.pif = 0x01,
		.descr = "ISA RTC controller"
	},
	{
		.sc = 0x04,
		.pif = 0x00,
		.descr = "Generic PCI Hot-Plug controller"
	},
	{
		.sc = 0x80,
		.pif = 0x00,
		.descr = "Generic system peripheral"
	}
};

static const struct pci_subclass PCI_CLASS9[] = {
	{
		.sc = 0x00,
		.pif = 0x00,
		.descr = "Keyboard controller"
	},
	{
		.sc = 0x01,
		.pif = 0x00,
		.descr = "Digitizer (pen)"
	},	
	{
		.sc = 0x02,
		.pif = 0x00,
		.descr = "Mouse controller"
	},
	{
		.sc = 0x03,
		.pif = 0x00,
		.descr = "Scanner controller"
	},
	{
		.sc = 0x04,
		.pif = 0x00,
		.descr = "Gameport controller"
	},
	{
		.sc = 0x04,
		.pif = 0x10,
		.descr = "Gameport controller (legacy)"
	},
	{
		.sc = 0x80,
		.pif = 0x00,
		.descr = "Generic input controller"
	}
};
 
static const struct pci_subclass PCI_CLASSB[] = {
	{
		.sc = 0x00,
		.pif = 0x00,
		.descr = "386"
	},
	{
		.sc = 0x01,
		.pif = 0x00,
		.descr = "486"
	},	
	{
		.sc = 0x02,
		.pif = 0x00,
		.descr = "Pentium"
	},
	{
		.sc = 0x10,
		.pif = 0x00,
		.descr = "Alpha"
	},
	{
		.sc = 0x20,
		.pif = 0x00,
		.descr = "PowerPC"
	},
	{
		.sc = 0x30,
		.pif = 0x00,
		.descr = "MIPS"
	},
	{
		.sc = 0x40,
		.pif = 0x00,
		.descr = "Co-processor"
	}
};

static const struct pci_subclass PCI_CLASSC[] = {
	{
		.sc = 0x00,
		.pif = 0x00,
		.descr = "IEEE 1394 (FireWire)"
	},
	{
		.sc = 0x00,
		.pif = 0x10,
		.descr = "IEEE 1394 (OpenHCI)"
	},	
	{
		.sc = 0x01,
		.pif = 0x00,
		.descr = "ACCESS.bus"
	},
	{
		.sc = 0x02,
		.pif = 0x00,
		.descr = "SSA"
	},
	{
		.sc = 0x03,
		.pif = 0x00,
		.descr = "Universal Serial Bus (UHC) Host controller"
	},
	{
		.sc = 0x03,
		.pif = 0x10,
		.descr = "Universal Serial Bus (OHC) Host controller"
	},
	{
		.sc = 0x03,
		.pif = 0x20,
		.descr = "Universal Serial Bus 2 (EHC) Host controller"
	},
	{
		.sc = 0x03,
		.pif = 0x80,
		.descr = "Universal Serial Bus (USB) Host controller"
	},
	{
		.sc = 0x03,
		.pif = 0xFE,
		.descr = "Universal Serial Bus device"
	},
	{
		.sc = 0x04,
		.pif = 0x00,
		.descr = "Fibre Channel"
	},
	{
		.sc = 0x05,
		.pif = 0x00,
		.descr = "SMBus (System Management Bus)"
	},
	{
		.sc = 0x06,
		.pif = 0x00,
		.descr = "InfiniBand"
	},
	{
		.sc = 0x07,
		.pif = 0x00,
		.descr = "IPMI SMIC Interface"
	},
	{
		.sc = 0x07,
		.pif = 0x01,
		.descr = "IPMI Kybd Controller Style Interface"
	},
	{
		.sc = 0x07,
		.pif = 0x02,
		.descr = "IPMI Block Transfer Interface"
	},
	{
		.sc = 0x08,
		.pif = 0x00,
		.descr = "SERCOS Interface Standard (IEC 61491)"
	},
	{
		.sc = 0x09,
		.pif = 0x00,
		.descr = "CANbus"
	}
};

static const struct pci_subclass PCI_CLASSD[] = {
	{
		.sc = 0x00,
		.pif = 0x00,
		.descr = "iRDA compatible controller"
	},
	{
		.sc = 0x01,
		.pif = 0x00,
		.descr = "Consumer IR controller"
	},	
	{
		.sc = 0x10,
		.pif = 0x00,
		.descr = "RF controller"
	},
	{
		.sc = 0x11,
		.pif = 0x00,
		.descr = "Bluetooth"
	},
	{
		.sc = 0x12,
		.pif = 0x00,
		.descr = "Broadband"
	},
	{
		.sc = 0x80,
		.pif = 0x00,
		.descr = "Generic wireless controller"
	}	
};

struct pci_subclass_descr {
	const struct pci_subclass *descr;
	unsigned num;
};

static const struct pci_subclass_descr subclass_desc_tree[] = {
	{ PCI_CLASS0, NELEMENTS(PCI_CLASS0) },
	{ PCI_CLASS1, NELEMENTS(PCI_CLASS1) },
	{ PCI_CLASS2, NELEMENTS(PCI_CLASS2) },
	{ PCI_CLASS3, NELEMENTS(PCI_CLASS3) },
	{ PCI_CLASS4, NELEMENTS(PCI_CLASS4) },
	{ PCI_CLASS5, NELEMENTS(PCI_CLASS5) },
	{ PCI_CLASS6, NELEMENTS(PCI_CLASS6) },
	{ PCI_CLASS7, NELEMENTS(PCI_CLASS7) },
	{ PCI_CLASS8, NELEMENTS(PCI_CLASS8) },
	{ PCI_CLASS9, NELEMENTS(PCI_CLASS9) },	
	{ NULL, 0},
	{ PCI_CLASSB, NELEMENTS(PCI_CLASSB) },
	{ PCI_CLASSC, NELEMENTS(PCI_CLASSC) },
	{ PCI_CLASSD, NELEMENTS(PCI_CLASSD) },
	{ NULL, 0},
	{ NULL, 0},
	{ NULL, 0},
	{ NULL, 0}
};

#endif // PCI_DEVICES_H_INCLUDED
