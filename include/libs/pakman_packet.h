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

#ifndef _PAKMAN_PACKET_H_
#define _PAKMAN_PACKET_H_

/*
 * a packet is composed as follows
 *
 * +-------------------------------------------------+
 *     |                                           |
 *    o----data---------------------data_size-----o
          |                                      |
 *        o-data_payload_start+data_payload_size-o
 *             |
 *            o cursor
 * +-------------------------------------------------+
 */

struct packet {
    /*
     * This is the pointer to the data slot
     */
    void *data;

    /*
     * This is the pointer to the payload data
     * real data starts at data_payload_start and
     * ends at data_payload_start + data_payload_size
     */
    void *data_payload_start;

    /*
     * This is the temporary pointer to the payload data
     * the cursor is used to read/write data inside the slot
     * it MUST BE aligned to a protocol header boundary
     */
    void *data_payload_cursor;

    /*
     * This is the size in bytes of what
     * has been saved into the packet
     * this is the payload storage size (ranging from 0
     * to data_size)
     */
    uint16_t data_payload_size;

    /*
     * This is the size in bytes of memory allocation for data
     * this is the overall storage size
     */
    uint16_t data_size;
};

#endif
