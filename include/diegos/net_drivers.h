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

#ifndef NET_DRIVERS_H_INCLUDED
#define NET_DRIVERS_H_INCLUDED

#include <libs/pakman_packet.h>

typedef struct net_driver {
	/*
	 * Common functionalities
	 */
	driver_header_t cmn;
	/*
	 * Interface name
	 */
	char ifname[DRV_NAME_LEN + 1];
	/*
	 * Maximum Transmission Unit of the physical device, i.e. the maximum
	 * payload per frame.
	 */
	unsigned mtu;
	/*
	 * Interface type, see if.h
	 */
	unsigned iftype;
	/*
	 * The physical device's address proper, followed by the broadcast
	 * address
	 */
	uint8_t addr[DRV_MAX_ADDR_LEN];
	uint8_t broadcast_addr[DRV_MAX_ADDR_LEN];
	/*
	 * Flags are documented in if.h
	 */
	unsigned short ifflags;
	/*
	 * Features are documented in if.h
	 */
	unsigned short ifcaps;
	/*
	 * Write function, the data buffer pointed by buf will be output to the device
	 */
	int (*tx_fn)(const struct packet * buf, unsigned unitno);
	/*
	 * Read function, the data buffer pointed by buf will be written with data
	 */
	int (*rx_fn)(struct packet * buf, unsigned unitno);
	/*
	 * Multi Write function, the data buffer list pointed by buf
	 * will be output to the device
	 */
	int (*tx_multi_fn)(const struct packet ** buf, unsigned items, unsigned unitno);
	/*
	 * Multi Read function, the data buffer list pointed by buf
	 * will be written with data
	 */
	int (*rx_multi_fn)(struct packet ** buf, unsigned items, unsigned unitno);
	/*
	 * Peak buffer size of the next rx operation; can be used to size
	 * the receiving buffer
	 */
	int (*rx_peak_fn)(unsigned *bsize, unsigned items, unsigned unitno);

} net_driver_t;

#endif				// NET_DRIVERS_H_INCLUDED
