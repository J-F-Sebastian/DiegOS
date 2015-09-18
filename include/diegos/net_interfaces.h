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

#ifndef _NET_INTERFACES_H_
#define _NET_INTERFACES_H_

#include <types_common.h>
#include <stddef.h>
#include <diegos/drivers.h>

#define NETIF_NAME_LEN  (5)
#define MAX_ADDR_LEN	(8)

typedef struct net_interface {
	char name[NETIF_NAME_LEN + 1];
	unsigned unit;
	unsigned flags;

	int ifindex;
	int iflink;

//	struct net_device_stats stats;

	unsigned char operstate;
	unsigned char link_mode;

	unsigned int mtu;
	unsigned short type;
	unsigned short hard_header_len;

	/* Interface address info. */
	unsigned char perm_addr[MAX_ADDR_LEN];
	unsigned char addr_assign_type;
	unsigned char addr_len;
	unsigned char broadcast[MAX_ADDR_LEN];

	net_driver_t *drv;
} net_interface_t;

/* Called by drivers' init routines, net_interface_create() return a pointer to
 * the newly created device.
 *
 * PARAMETERS IN
 * net_driver_t *inst - a pointer to a driver interface; the driver must be capable
 *                  of handling the unit number found in the parameter unit.
 *
 * RETURNS
 * A valid pointer to a new interface object in case of success, NULL in any other
 * case.
 */
net_interface_t *net_interface_create(net_driver_t *inst);

/*
 * net_interface_lookup_name() retrieves a pointer to an interface.
 * The search key is the full interface name.
 *
 * PARAMETERS IN
 * const char *ifname - the interface's name.
 *
 * RETURNS
 * A valid pointer to an existing device object in case of success,
 * NULL in any other case.
 */
net_interface_t *net_interface_lookup_name(const char *ifname);

/*
 * net_interface_lookup_index() retrieves a pointer to an interface.
 * The search key is the interface ifindex.
 *
 * PARAMETERS IN
 * const int ifindex - the interface's index.
 *
 * RETURNS
 * A valid pointer to an existing device object in case of success,
 * NULL in any other case.
 */
net_interface_t *net_interface_lookup_index(const int ifindex);

/*
 * net_interface_first() retrieves a pointer to the first interface
 * in the system.
 *
 * RETURNS
 * A valid pointer to an existing device object in case of success,
 * NULL in any other case.
 */
net_interface_t *net_interface_first(void);

/*
 * net_interface_next() retrieves a pointer to the interface next to
 * the one pointed by intp.
 *
 * PARAMETERS IN
 * net_interface_t *intp - pointer to an interface.
 *
 * RETURNS
 * A valid pointer to an existing device object in case of success,
 * NULL in any other case.
 */
net_interface_t *net_interface_next(net_interface_t *intp);

#endif /* _NET_INTERFACES_H_ */
