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

#ifndef IF_H_INCLUDED_
#define IF_H_INCLUDED_

enum {
	/* Interface is up */
	IFF_UP = (1 << 0),
	/* Interface has a valid broadcast address */
	IFF_BROADCAST = (1 << 1),
	/* Interface supports multicast */
	IFF_MULTICAST = (1 << 2),
	/* Interface can receive all multicast packets */
	IFF_ALLMULTI = (1 << 3),
	/* Interface can receive all packets */
	IFF_PROMISC = (1 << 4),
	/* Interface is a loopback */
	IFF_LOOPBACK = (1 << 5)
};

enum {
	NETIF_F_SG = (1 << 0),
	NETIF_F_NO_CSUM = (1 << 1)
};

#endif				// IF_H_INCLUDED_
