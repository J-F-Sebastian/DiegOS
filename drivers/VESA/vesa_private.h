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

#ifndef _VESA_PRIVATE_H_
#define _VESA_PRIVATE_H_

/* VESA modes constants */
/* VESA */
enum vesa_mode {
	VESA640x400x256 = 0x0100,
	VESA640x480x256 = 0x0101,
	VESA800x600x16 = 0x0102,
	VESA800x600x256 = 0x0103,
	VESA1024x768x16 = 0x0104,
	VESA1024x768x256 = 0x0105,
	VESA1280x1024x16 = 0x0106,
	VESA1280x1024x256 = 0x0107,
	VESA80x60text = 0x0108,
	VESA132x25text = 0x0109,
	VESA132x43text = 0x010A,
	VESA132x50text = 0x010B,
	VESA132x60text = 0x010C,
	/* VBE 1.2 */
	VESA320x200x32K = 0x010D,
	VESA320x200x64K = 0x010E,
	VESA320x200x16M = 0x010F,
	VESA640x480x32K = 0x0110,
	VESA640x480x64K = 0x0111,
	VESA640x480x16M = 0x0112,
	VESA800x600x32K = 0x0113,
	VESA800x600x64K = 0x0114,
	VESA800x600x16M = 0x0115,
	VESA1024x768x32K = 0x0116,
	VESA1024x768x64K = 0x0117,
	VESA1024x768x16M = 0x0118,
	VESA1280x1024x32K = 0x0119,
	VESA1280x1024x64K = 0x011A,
	VESA1280x1024x16M = 0x011B,
	/* VBE 2.0 */
	VESA1600x1200x256 = 0x0120,
	VESA1600x1200x32K = 0x0121,
	VESA1600x1200x64K = 0x0122,
	VESAFullMemoryAcc = 0x81FF
};

struct VBEInfoBlock {
	char signature[4];	/* "VESA" */
	/*signature must be "VBE2" to get vesa 2.0 infos */
	uint16_t version;	/* BCD format */
	char *OEMnameptr;
	uint32_t capabilities;
	uint16_t *VESAmodesptr;	/* list terminated by 0xFFFF */
	uint16_t videomem64k;
	/*Following records only for VBE2 */
	uint16_t OEMswversion;	/* BCD */
	char *vendornameptr;
	char *productnameptr;
	void *ProductRevPtr;
	uint16_t VBEAFversion;	/* BCD - only if Cap bit 3 set */
	uint16_t *VBEAFmodesptr;	/* list terminated by $FFFF
					   only if Cap bit 3 set */
	char reserved[216];
	char OEMscratchpad[256];
} __attribute__((packed));

struct ModeInfoBlock {
	uint16_t ModeAttributes;
	uint8_t WinAAttributes;
	uint8_t WinBAttributes;
	uint16_t WinGranularity;
	uint16_t WinSize;
	uint16_t segWINA;
	uint16_t segWINB;
	uintptr_t RealWinFuncPtr;
	uint16_t BPL;
	/* VESA 1.2 */
	uint16_t XResolution;
	uint16_t YResolution;
	uint8_t XCharSize;
	uint8_t YCharSize;
	uint8_t NumberOfPlanes;
	uint8_t BitsPerPixel;
	uint8_t NumberOfBanks;
	uint8_t MemoryModel;
	uint8_t BankSize;
	uint8_t NumberOfPages;
	uint8_t reserved;
	uint8_t RedMaskSize;
	uint8_t RedFieldPos;
	uint8_t GreenMaskSize;
	uint8_t GreenFieldPos;
	uint8_t BlueMaskSize;
	uint8_t BlueFieldPos;
	uint8_t ResMaskSize;
	uint8_t ResFieldPos;
	uint8_t DirectColorInfo;
	/* VESA 2.0 */
	uintptr_t PhysAddress;
	uintptr_t Reserved1;
	uint16_t Reserved2;
	/* VESA 3.0 */
	uint16_t LinBytesPerScanLine;
	uint8_t BnkNumberOfImagePages;
	uint8_t LinNumberOfImagePages;
	uint8_t LinRedMaskSize;
	uint8_t LinRedFieldPosition;
	uint8_t LinGreenMaskSize;
	uint8_t LinGreenFieldPosition;
	uint8_t LinBlueMaskSize;
	uint8_t LinBlueFieldPosition;
	uint8_t LinRsvdMaskSize;
	uint8_t LinRsvdFieldPosition;
	uint32_t MaxPixelClock;
	char reserved3[189];
} __attribute__((packed));

struct XGAInfoBlock {
	char signature[4];
	uint16_t version;
	char *OEMnameptr;
	uint32_t flags;
	uint16_t adptNum;
	char reserved[240];
} __attribute__((packed));

#endif
