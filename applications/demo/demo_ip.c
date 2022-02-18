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

#include <networking/networking.h>
#include <diegos/net_interfaces.h>
#include <diegos/kernel.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ip.h>

static void demo_thread_entry(void)
{
	net_interface_t *eth;

	if (EOK != netw_init()) {
		exit(-1);
	}

	eth = net_interface_lookup_name("eth0");
	if (eth) {
		netw_add_protocol(eth, &ip_proto);
	}

	eth = net_interface_lookup_name("lo0");
	if (eth) {
		netw_add_protocol(eth, &ip_proto);
	}

	while (TRUE) {
		thread_suspend();
	}
	thread_terminate();
}

void platform_run()
{
	uint8_t pid;

	thread_create("IPDemo", THREAD_PRIO_NORMAL, demo_thread_entry, 0, 8192, &pid);
}
