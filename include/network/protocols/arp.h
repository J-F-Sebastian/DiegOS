/**
 *
 *                                                   ,----..
 *        ,---,                                     /   /   \   .--.--.
 *      .'  .' `\    ,--,                          /   .     : /  /    '.
 *    ,---.'     \ ,--.'|                         .   /   ;.  \  :  /`. /
 *    |   |  .`\  ||  |,                ,----._,..   ;   /  ` ;  |  |--`
 *    :   : |  '  |`--'_       ,---.   /   /  ' /;   |  ; \ ; |  :  ;_
 *    |   ' '  ;  :,' ,'|     /     \ |   :     ||   :  | ; | '\  \    `.
 *    '   | ;  .  |'  | |    /    /  ||   | .\  ..   |  ' ' ' : `----.   \
 *    |   | :  |  '|  | :   .    ' / |.   ; ';  |'   ;  \; /  | __ \  \  |
 *    '   : | /  ; '  : |__ '   ;   /|'   .   . | \   \  ',  / /  /`--'  /
 *    |   | '` ,/  |  | '.'|'   |  / | `---`-'| |  ;   :    / '--'.     /
 *    ;   :  .'    ;  :    ;|   :    | .'__/\_: |   \   \ .'    `--'---'
 *    |   ,.'      |  ,   /  \   \  /  |   :    :    `---`
 *    '---'         ---`-'    `----'    \   \  /
 *                                       `--`-'
 */

/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2026 Diego Gallizioli
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

#ifndef _ARP_H_
#define _ARP_H_

#include <stdint.h>

#pragma pack(push, 1)

struct arp_header {
	/*
	 * Hardware address space, i.e. the hardware
	 * protocol or the technology the hardware address
	 * belongs to.
	 */
	uint16_t hw_address_space;
	/*
	 * Protocol address space, i.e. the protocol
	 * the address belongs to.
	 */
	uint16_t prot_address_space;
	/*
	 * Hardware address length in octets.
	 */
	uint8_t hw_address_len;
	/*
	 * Protocol address length in octets.
	 */
	uint8_t prot_address_len;
	/*
	 * ARP protocol opcode.
	 */
	uint16_t opcode;
	/*
	 * Following this header there are 4 fields:
	 *
	 * hardware address of sender (hw_address_len octets)
	 * protocol address of sender (prot_address_len octets)
	 * hardware address of target (hw_address_len octets)
	 * protocol address of target (prot_address_len octets)
	 */
};

#pragma pack(pop)

#endif
