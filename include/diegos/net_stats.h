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

#ifndef _NET_STATS_H_
#define _NET_STATS_H_

#include <stdint.h>

enum net_stats_err
{
	NETSTATS_RUNT,
	NETSTATS_LONG,
	NETSTATS_CRC,
	/*
	 * This does not belong to any counter
	 */
	NETSTATS_OTHER
};

struct net_stats
{
	/*
	 * Total bytes transmitted
	 */
	uint64_t tx_bytes;
	/*
	 * Total bytes received
	 */
	uint64_t rx_bytes;
	/*
	 * Total packets transmitted
	 */
	uint64_t tx_packets;
	/*
	 * Total packets received
	 */
	uint64_t rx_packets;
	/*
	 * Total errored received packets
	 */
	uint64_t rx_err_packets;
	/*
	 * Total runt packets received (length less than 64 octets)
	 * This is an errored packet
	 */
	uint64_t rx_runt_packets;
	/*
	 * Total long packets received (length more than 1500 octets)
	 * This is an errored packet
	 */
	uint64_t rx_long_packets;
	/*
	 * Total packets received with CRC errors
	 * This is an errored packet
	 */
	uint64_t rx_crc_err_packets;
	/*
	 * Total errored transmitted packets
	 */
	uint64_t tx_err_packets;
};

/*
 * Initialize the network statistics structure.
 *
 * PARAMETERS IN
 * struct net_stats *ns - pointer to a net_stats structure
 *
 * RETURNS
 * EOK success
 * EINVAL ns is NULL
 */
int netstats_init(struct net_stats *ns);

/*
 * Update the net_stats structure with good packet received bytes.
 * The function will increase rx_bytes, rx_packets.
 *
 * PARAMETERS IN
 * struct net_stats *ns - pointer to a net_stats structure
 * unsigned bytes - amount of bytes received
 *
 * RETURNS
 * EOK success
 * EINVAL ns is NULL
 */
int netstats_update_rx(struct net_stats *ns, unsigned bytes);

/*
 * Update the net_stats structure with good packet transmitted bytes.
 * The function will increase tx_bytes, tx_packets.
 *
 * PARAMETERS IN
 * struct net_stats *ns - pointer to a net_stats structure
 * unsigned bytes - amount of bytes transmitted
 *
 * RETURNS
 * EOK success
 * EINVAL ns is NULL
 */
int netstats_update_tx(struct net_stats *ns, unsigned bytes);

/*
 * Update the net_stats structure with bad packet received bytes.
 * The function will increase rx_bytes, rx_packets, rx_err_packets,
 * and any other receive error counter specified by err.
 *
 * PARAMETERS IN
 * struct net_stats *ns - pointer to a net_stats structure
 * unsigned bytes - amount of bytes received but errored
 * enum net_stats_err err - type of error
 *
 * RETURNS
 * EOK success
 * EINVAL ns is NULL
 */
int netstats_update_rx_err(struct net_stats *ns, unsigned bytes, enum net_stats_err err);

/*
 * Update the net_stats structure with bad packet transmitted bytes.
 * The function will increase tx_bytes, tx_packets and tx_err_packets.
 *
 * PARAMETERS IN
 * struct net_stats *ns - pointer to a net_stats structure
 * unsigned bytes - amount of bytes transmitted
 *
 * RETURNS
 * EOK success
 * EINVAL ns is NULL
 */
int netstats_update_tx_err(struct net_stats *ns, unsigned bytes);

#endif