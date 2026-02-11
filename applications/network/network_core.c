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

#include <diegos/net_buffers.h>
#include <diegos/kernel.h>
#include <libs/802_x.h>
#include <network/protocols/arp.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

#define __NET_CORE_VER__ "1.0"

static int network_core_process_in(struct packet *pkt)
{
	struct ieee_802_3_hdr *ptr = pkt->data;
	struct arp_header *hdr;

	/*
	 * We support only untagged ethernet frames for now!
	 */
	switch ((int)ntohs(ptr->type))
	{
	case ETHERTYPE_IP:
		fprintf(stderr, "IP packet\n");
		break;
	case ETHERTYPE_ARP:
		fprintf(stderr, "ARP packet\n");
		hdr = (struct arp_header *)pkt->data_payload_cursor;
		fprintf(stderr, " hw space %#X prot space %#X\n HW len %d prot len %d\n opcode %d\n",
			ntohs(hdr->hw_address_space),
			ntohs(hdr->prot_address_space),
			hdr->hw_address_len,
			hdr->prot_address_len,
			ntohs(hdr->opcode));

		break;
	case ETHERTYPE_RARP:
		fprintf(stderr, "RARP packet\n");
		break;
	default:
		fprintf(stderr, "UNSUPPORTED packet\n");
		return (ENOTSUP);
	}

	return (EOK);
}

static void network_core_main_entry(void)
{
	struct packet *pkt;
	net_interface_t *intf;
	BOOL keepon;
	int in, out;

	printf("Network core version %s\n", __NET_CORE_VER__);

	while (TRUE)
	{
		netbuf_wait();

		keepon = TRUE;
		while (keepon)
		{
			fprintf(stderr, "Packet\n");
			in = netbuf_process_in(&pkt);
			if (EOK == in)
			{
				if (EOK != network_core_process_in(pkt))
				{
					printf("Unknown packet!\n");
				}
				netbuf_put(pkt);
			}

			out = netbuf_process_out(&pkt, &intf);
			if (EOK == out)
			{
				netbuf_put(pkt);
			}

			/*
			 * Keep going if in or out are EOK,
			 * stop when both are different than EOK.
			 * Hint: EOK value is 0.
			 */
			keepon = (in && out) ? FALSE : TRUE;
		}
	}
}

void network_run(void)
{
	uint8_t pid;
	BOOL retcode;

	retcode = thread_create("network", THREAD_PRIO_HIGH, network_core_main_entry, 0, 16 * 1024, &pid);

	assert(TRUE == retcode);
}
