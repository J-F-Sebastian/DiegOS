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

#ifndef _802_X_H_
#define _802_X_H_

#include <types_common.h>

#define MAC_ADDR_SIZE      (6)
#define MAC_HDR_SIZE       (6 + 6 + 2)
#define MAC_HDR_8023Q_SIZE (6 + 6 + 2 + 4)
#define MAC_FCS_SIZE       (4)
#define MAC_MIN_FRAME_SIZE (64)
#define MAC_MAX_FRAME_SIZE (1518)
#define MAC_MAX_JUMBO_SIZE (9018)

#define MAC_MIN_MTU_SIZE   (MAC_MIN_FRAME_SIZE - MAC_HDR_SIZE - MAC_FCS_SIZE)
#define MAC_MAX_MTU_SIZE   (MAC_MAX_FRAME_SIZE - MAC_HDR_SIZE - MAC_FCS_SIZE)
#define MAC_JUMBO_MTU_SIZE (MAC_MAX_JUMBO_SIZE - MAC_HDR_SIZE - MAC_FCS_SIZE)

#define ETHERTYPE_IP      0x0800
#define ETHERTYPE_ARP     0x0806
#define ETHERTYPE_RARP    0x8035
#define ETHERTYPE_8021Q   0x8100
#define ETHERTYPE_PPP     0x880B
#define ETHERTYPE_PPPoE_D 0x8863
#define ETHERTYPE_PPPoE_S 0x8864
#define ETHERTYPE_SVLAN   0x88A8

#pragma pack(push, 1)

/*
 * Ethernet encoded 802.3 MAC address.
 * the union is reliable for size computation.
 */
typedef union ieee_addr
{
	uint8_t mac[MAC_ADDR_SIZE];
	uint16_t macw[MAC_ADDR_SIZE / 2];
} ieee_addr_u;

/*
 * Ethernet encoded 802.3 MAC header.
 */
struct ieee_802_3_hdr
{
	ieee_addr_u src;
	ieee_addr_u dst;
	uint16_t type;
};

/*
 * Ethernet encoded 802.1Q MAC header.
 */
struct ieee_802_3q_hdr
{
	ieee_addr_u src;
	ieee_addr_u dst;
	/* Tag Protocol Identifier */
	uint16_t tpid;
	/* Tag Control Information */
	uint16_t tci;
	uint16_t type;
};

/*
 * Ethernet encoded 802.1ad MAC header.
 * This is the "Double Tag" or "Q-in-Q" frame.
 */
struct ieee_802_1ad_hdr
{
	ieee_addr_u src;
	ieee_addr_u dst;
	/* Tag Protocol Identifier */
	uint16_t tpid_svlan;
	/* Tag Control Information */
	uint16_t tci_svlan;
	/* Tag Protocol Identifier */
	uint16_t tpid_cvlan;
	/* Tag Control Information */
	uint16_t tci_cvlan;
	uint16_t type;
};

#pragma pack(pop)

inline void copy_ieee_addr(const ieee_addr_u *src, ieee_addr_u *dst)
{
	dst->macw[0] = src->macw[0];
	dst->macw[1] = src->macw[1];
	dst->macw[2] = src->macw[2];
}

inline BOOL cmp_ieee_addr(const ieee_addr_u *src, const ieee_addr_u *dst)
{
	return (((dst->macw[0] == src->macw[0]) &&
		 (dst->macw[1] == src->macw[1]) && (dst->macw[2] == src->macw[2]))
		    ? TRUE
		    : FALSE);
}

inline BOOL is_broadcast_ieee_addr(const ieee_addr_u *addr)
{
	return ((addr->macw[0] == 0xFFFF) &&
		(addr->macw[1] == 0xFFFF) &&
		(addr->macw[2] == 0xFFFF))
		   ? TRUE
		   : FALSE;
}

/*
 *
 *2.1.3. 48-Bit MAC Assignments under the IANA OUI
 *
 *The OUI 00‑00‑5E has been assigned to IANA, as stated in Section 1.3 above.
 * This includes 2**24 48‑bit multicast identifiers from 01‑00‑5E‑00‑00‑00 to 01‑00‑5E‑FF‑FF‑FF
 * and 2**24 EUI‑48 unicast identifiers from 00‑00‑5E‑00‑00‑00 to 00‑00‑5E‑FF‑FF‑FF.
 *
 *Of these identifiers, the sub-blocks reserved or thus far assigned are as follows:
 *
 *Unicast, all blocks of 2**8 addresses thus far:
 *
 *   00‑00‑5E‑00‑00‑00 through 00‑00‑5E‑00‑00‑FF:
 *	reserved and require IESG Ratification for assignment (see Section 5.1).
 *    00‑00‑5E‑00‑01‑00 through 00‑00‑5E‑00‑01‑FF:
 *	assigned for the Virtual Router Redundancy Protocol (VRRP) [RFC5798].
 *    00‑00‑5E‑00‑02‑00 through 00‑00‑5E‑00‑02‑FF:
 *	assigned for the IPv6 Virtual Router Redundancy Protocol (IPv6 VRRP) [RFC5798].
 *    00‑00‑5E‑00‑52‑00 through 00‑00‑5E‑00‑52‑FF:
 *	used for very small assignments. As of 2024, 4 out of these 256 values have been assigned. See [EthernetNum].
 *    00‑00‑5E‑00‑53‑00 through 00‑00‑5E‑00‑53‑FF:
 *	assigned for use in documentation by this document.
 *    00‑00‑5E‑90‑01‑00 through 00‑00‑5E‑90‑01‑FF:
 *	used for very small assignments that need parallel unicast and multicast MAC addresses.
 *      As of 2024, 1 out of these 256 values has been assigned. See [EthernetNum].
 *
 *Multicast:
 *
 *    01‑00‑5E‑00‑00‑00 through 01‑00‑5E‑7F‑FF‑FF:
 *	2**23 addresses assigned for IPv4 multicast [RFC1112].
 *    01‑00‑5E‑80‑00‑00 through 01‑00‑5E‑8F‑FF‑FF:
 *	2**20 addresses assigned for MPLS multicast [RFC5332].
 *    01‑00‑5E‑90‑00‑00 through 01‑00‑5E‑90‑00‑FF:
 *	2**8 addresses being used for very small assignments. As of 2024, 4 out of these 256 values have been assigned. See [EthernetNum].
 *    01‑00‑5E‑90‑01‑00 through 01‑00‑5E‑90‑01‑FF:
 *	used for very small assignments that need parallel unicast and multicast MAC addresses.
 *      As of 2024, 1 out of these 256 values has been assigned. See [EthernetNum].
 *    01‑00‑5E‑90‑10‑00 through 01‑00‑5E‑90‑10‑FF:
 *	2**8 addresses assigned for use in documentation by this document.
 *
 *For more detailed and up-to-date information, see the "IANA OUI Ethernet Numbers" registry at [EthernetNum].
 *
 */
inline BOOL is_multicast_ieee_addr(const ieee_addr_u *addr)
{
	return ((addr->mac[0] == 0x01) &&
		(addr->mac[1] == 0x00) &&
		(addr->mac[2] == 0x5E)) ? TRUE : FALSE;
}

inline BOOL is_multicast_ipv4_ieee_addr(const ieee_addr_u *addr)
{
	return ((addr->mac[0] == 0x01) &&
		(addr->mac[1] == 0x00) &&
		(addr->mac[2] == 0x5E) &&
		(addr->mac[3] < 0x7F)) ? TRUE : FALSE;
}

inline BOOL is_multicast_mpls_ieee_addr(const ieee_addr_u *addr)
{
	return ((addr->mac[0] == 0x01) &&
		(addr->mac[1] == 0x00) &&
		(addr->mac[2] == 0x5E) &&
		(addr->mac[3] > 0x7F) &&
		(addr->mac[3] < 0x90)) ? TRUE : FALSE;
}

#endif
