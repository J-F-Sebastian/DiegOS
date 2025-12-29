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

#define MAC_ADDR_SIZE           (6)
#define MAC_HDR_SIZE            (6 + 6 + 2)
#define MAC_HDR_8023Q_SIZE      (6 + 6 + 2 + 4)
#define MAC_FCS_SIZE            (4)
#define MAC_MIN_FRAME_SIZE      (64)
#define MAC_MAX_FRAME_SIZE      (1518)
#define MAC_MAX_JUMBO_SIZE      (9018)

#define MAC_MIN_MTU_SIZE    (MAC_MIN_FRAME_SIZE - MAC_HDR_SIZE - MAC_FCS_SIZE)
#define MAC_MAX_MTU_SIZE    (MAC_MAX_FRAME_SIZE - MAC_HDR_SIZE - MAC_FCS_SIZE)
#define MAC_JUMBO_MTU_SIZE  (MAC_MAX_JUMBO_SIZE - MAC_HDR_SIZE - MAC_FCS_SIZE)

#define ETHERTYPE_IP		0x0800
#define	ETHERTYPE_ARP		0x0806
#define	ETHERTYPE_RARP		0x8035
#define ETHERTYPE_8021Q		0x8100
#define ETHERTYPE_PPP		0x880B
#define ETHERTYPE_PPPoE_D	0x8863
#define ETHERTYPE_PPPoE_S	0x8864
#define ETHERTYPE_SVLAN		0x88A8

typedef union ieee_addr {
	uint8_t mac[MAC_ADDR_SIZE];
	uint16_t macw[MAC_ADDR_SIZE / 2];
} ieee_addr_t;

typedef struct ieee_802_3_hdr {
	ieee_addr_t src;
	ieee_addr_t dst;
	uint16_t type;
} ieee_802_3_hdr_t;

/*
 * Ethernet encoded 802.1Q MAC header
 */
typedef struct ieee_802_3q_hdr {
	ieee_addr_t src;
	ieee_addr_t dst;
	/* Tag Protocol Identifier */
	uint16_t tpid;
	/* Tag Control Information */
	uint16_t tci;
	uint16_t type;
} ieee_802_3q_hdr_t;

inline void copy_ieee_addr(const ieee_addr_t *src, ieee_addr_t *dst)
{
	dst->macw[0] = src->macw[0];
	dst->macw[1] = src->macw[1];
	dst->macw[2] = src->macw[2];
}

inline BOOL cmp_ieee_addr(const ieee_addr_t *src, const ieee_addr_t *dst)
{
	return (((dst->macw[0] == src->macw[0]) &&
		 (dst->macw[1] == src->macw[1]) && (dst->macw[2] == src->macw[2])) ? TRUE : FALSE);
}

#endif
