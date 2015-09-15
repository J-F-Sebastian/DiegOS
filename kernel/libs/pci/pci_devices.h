#ifndef PCI_DEVICES_H_INCLUDED
#define PCI_DEVICES_H_INCLUDED

struct pci_vendor_device_node {
    const uint16_t id;
    const char *description;
};

struct pci_vendor_node {
    struct pci_vendor_device_node header;
    const struct pci_vendor_device_node *devices;
};

/* All devices lists sorted by DEVICEID */
const struct pci_vendor_device_node intel_list[] = {
    {0x100e, "Pro 1000/MT"},
    {0x1229, "Pro 100"},
    {0x1237, "82440LX/EX Chipset (Natoma)"},
    {0x7000, "82371SB PIIX3 PCI-to-ISA Bridge (Triton II)"},
    {0x7010, "82371SB PIIX3 IDE Interface (Triton II)"},
    {0x7020, "82371SB PIIX3 USB Host Controller (Triton II)"},
    {0x7113, "82371AB/EB/MB PIIX4/4E/4M Power Management Controller"},
    {0xFFFF, "Unknown"}
};

const struct pci_vendor_device_node realtek_list[] = {
    {0x8139, "RTL8139 10/100Mb Ethernet Controller"},
    {0x8169, "RTL8169 10/100Mb Ethernet Controller"},
    {0xFFFF, "Unknown"}
};

const struct pci_vendor_device_node amd_list[] = {
    {0x2000, "PCnet LANCE PCI Ethernet Controller (Am79C970)"},
    {0xFFFF, "Unknown"}
};

/* All vendors list sorted by VENDORID */
const struct pci_vendor_node pci_known_devices_list[] = {
    {{0x1022, "AMD"}, amd_list},
    {{0x10EC, "Realtek"}, realtek_list},
    {{0x8086, "Intel"}, intel_list},
    {{0xFFFF, "Unknown"}, NULL},
    };




#endif // PCI_DEVICES_H_INCLUDED
