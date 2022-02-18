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

#ifndef _PAKMAN_H_
#define _PAKMAN_H_

#include <types_common.h>
#include <pakman_packet.h>

typedef struct packet_buffer pakman;

int init_pakman(unsigned bytes, unsigned packets, pakman ** pakmanptr);

int delete_pakman(pakman * pakmanptr);

/*
 * bufmgr_get_slot() will return a free slot descriptor, or NULL if no more free
 * slots are available. Slot state is set to busy.
 */
struct packet *pakman_get_packet(pakman * pakmanptr, uint16_t len);

/*
 * pakman_put_packet() will drop the slot, i.e. will set it to free state.
 */
int pakman_put_packet(pakman * bufmgrptr, struct packet *pkt);

#endif
