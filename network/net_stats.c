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

#include <diegos/net_stats.h>
#include <string.h>
#include <errno.h>

int netstats_init(struct net_stats *ns)
{
	if (ns) {
		memset(ns, 0, sizeof(*ns));
		return EOK;
	}
	return EINVAL;
}

int netstats_update_rx(struct net_stats *ns, unsigned bytes)
{
	if (ns && bytes) {
		ns->rx_bytes += bytes;
		ns->rx_packets++;
		return EOK;
	}
	return EINVAL;
}

int netstats_update_tx(struct net_stats *ns, unsigned bytes)
{
	if (ns && bytes)
	{
		ns->tx_bytes += bytes;
		ns->tx_packets++;
		return EOK;
	}
	return EINVAL;
}

int netstats_update_rx_err(struct net_stats *ns, unsigned bytes, enum net_stats_err err)
{
	if (ns && bytes) {
		ns->rx_bytes += bytes;
		ns->rx_packets++;
		ns->rx_err_packets++;

		switch (err)
		{
		case NETSTATS_RUNT:
			ns->rx_runt_packets++;
			break;
		case NETSTATS_LONG:
			ns->rx_long_packets++;
			break;
		case NETSTATS_CRC:
			ns->rx_crc_err_packets++;
			break;
		case NETSTATS_OTHER:
		/* FALLTHRU */
		default:
			break;
		}
		return EOK;
	}
	return EINVAL;
}

int netstats_update_tx_err(struct net_stats *ns, unsigned bytes)
{
	if (ns && bytes) {
		ns->tx_bytes += bytes;
		ns->tx_packets++;
		ns->tx_err_packets++;
		return EOK;
	}
	return EINVAL;
}
