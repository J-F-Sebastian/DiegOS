/**
 *
 *                                                   ,----..
 *        ,---,                                     /   /   \   .--.--.
 *      .'  .' `\    ,--,                          /   .     : /  /    '.
 *    ,---.'     \ ,--.'|                         .   /   ;.  \  :  /`. /
 *    |   |  .`\  ||  |,                ,----._,..   ;   /  ` ;  |  |--`
 *    :   : |  '  |`--'_       ,---.   /   /  ' /;   |  ; \ ; |  :  ;_
 *    |   ' '  ;  :,' ,'|     /     \ |   :     ||   :  | ; | '\  \    `.
 *    '   | ;  .  |'  | |    /    /  ||   | .\  ..   |  ' ' ' : `----.   \
 *    |   | :  |  '|  | :   .    ' / |.   ; ';  |'   ;  \; /  | __ \  \  |
 *    '   : | /  ; '  : |__ '   ;   /|'   .   . | \   \  ',  / /  /`--'  /
 *    |   | '` ,/  |  | '.'|'   |  / | `---`-'| |  ;   :    / '--'.     /
 *    ;   :  .'    ;  :    ;|   :    | .'__/\_: |   \   \ .'    `--'---'
 *    |   ,.'      |  ,   /  \   \  /  |   :    :    `---`
 *    '---'         ---`-'    `----'    \   \  /
 *                                       `--`-'
 */

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

#ifndef _NET_BUFFERS_H_
#define _NET_BUFFERS_H_

/*
 * The netbuf structure
 *
 *                        netbuf_in
 * +------------------+ - - - - - - - - - - - - - - - - - - - - - - - - \
 * |                  |                                                 |
 * |     DRIVERS      |   netbuf_get                                    |
 * |                  |   netbuf_put                                    |
 * +------------------+ < - - - - - > +-----------------+               |
 *                    ^               |                 |               |
 *                    |               |      PAKMAN     |               |
 *                    |               |                 |               V                     netbuf_process_in
 *                    |               +-----------------+               +-----------------+ - - - - - >
 *                    |                                                 |                 |
 *                    |                                                 |      IN/OUT     |             sockets, IPv4, IPv6, Applications
 *                    |                                                 |      QUEUES     |
 *                    \ - - - - - - - - - - - - - - - - - - - - - - - - +-----------------+ < - - - - -
 *                        netbuf_process_out                                                    netbuf_out
 *
 *
 * The drivers use netbuf_in to send packets into the network stack, IN queue.
 * The drivers must allocate packets using nebuf_get, the packet is consumed by
 * the stack and need not to be released.
 * The stack consume the packet by calling netbuf_process_in and pass it to sockets, IPv4,
 * IPv6, routing, bridging, etc.
 * The stack is responsible for releasing packets once the payload has been consumed.
 * Application go the same way around by sending packets to the stack using
 * netbuf_out, OUT queue.
 * The stack consumes the packet by calling netbuf_process_out and pass the packet
 * to a driver, calling its send function.
 * The driver is responsible for releasing packets once the packet has been sent.
 */

#include <diegos/net_interfaces.h>
#include <libs/pakman_packet.h>

/*
 * Initialize the network buffering library.
 * Must be called at the kernel initialization stage.
 *
 * PARAMETERS IN
 * unsigned bytes - amount of bytes to be used for packet buffers
 * unsigned packets - maximum number of packets in the queue
 *
 * RETURNS
 * EOK success
 * EINVAL bytes is less than CACHE_ALN or packets is less than 8
 * ENOMEM is the buffer cannot be allocated
 */
int netbuf_init(unsigned bytes, unsigned packets);

/*
 * Get a free packet from the network buffer.
 *
 * PARAMETERS IN
 * unsigned bytes - size of the packet in bytes, size will be rounded
 *                  to a multiple of a cache line (so it is CPU dependant)
 *
 * PARAMETERS OUT
 * struct packet **pkt - a reference to a pointer to a free packet
 *
 * RETURNS
 * ENOBUFS if no free packet is available
 * ENOMEM if no free packet of the requested size is available
 * EINVAL if pkt is NULL or bytes is zero
 * EOK success
 */
int netbuf_get(struct packet **pkt, unsigned bytes);

/*
 * Returns a packet to the network buffer, making it free.
 * This function must be called after using the packet, for instance after
 * a successful transmission.
 * The packet must have been retrieved by calling netbuf_get().
 *
 * PARAMETERS IN
 * struct packet *pkt - a pointer to the packet to be released
 *
 * RETURNS
 * EINVAL if pkt is NULL
 * EPERM if pkt does not belong to the network buffer
 * EINVAL is pkt is NULL
 * EOK success
 */
int netbuf_put(struct packet *pkt);

/*
 * Sends a packet into the network stack (IN queue).
 * The packet must have been retrieved by calling netbuf_get().
 * The packet must not be released by calling netbuf_put().
 * The network stack will release the packet after using it.
 * In case of failure - the queue is full - the packet is
 * rejected and the driver must stop receiving data.
 * The packet enters the network stack to be routed or bridged
 * or consumed.
 *
 * PARAMETERS IN
 * struct packet *pkt - a pointer to the packet to be sent into the network stack
 *
 * RETURNS
 * EOK success
 * ENOBUFS if the queue is full
 * EINVAL if pkt is NULL
 */
int netbuf_in(struct packet *pkt);

/*
 * Sends a packet to the network stack (OUT queue).
 * The packet must have been retrieved by calling netbuf_get().
 * The packet must not be released by calling netbuf_put().
 * The network stack will release the packet after using it.
 * In case of failure - the queue is full - the packet is
 * rejected and the application must suspend sending data.
 * The packet enters the network stack to be sent to a network
 * interface (physical).
 *
 * PARAMETERS IN
 * struct packet *pkt - a pointer to the packet to be sent to the network stack
 *
 * RETURNS
 * EOK success
 * ENOBUFS if the queue is full
 * EINVAL if pkt is NULL or intf is NULL
 */
int netbuf_out(struct packet *pkt, net_interface_t * intf);

/*
 * Retrieve a packet from the IN queue.
 *
 * PARAMETERS IN
 * struct packet **pkt - a reference to a pointer to a packet
 *
 * PARAMETERS OUT
 * struct packet **pkt - a reference to a pointer to a packet removed from the IN queue
 *
 * RETURNS
 * EOK success
 * EAGAIN the IN queue is empty
 * EINVAL pkt is NULL
 */
int netbuf_process_in(struct packet **pkt);

/*
 * Retrieve a packet from the OUT queue.
 *
 * PARAMETERS IN
 * struct packet **pkt - a reference to a pointer to a packet
 *
 * PARAMETERS OUT
 * struct packet **pkt - a reference to a pointer to a packet removed from the OUT queue
 *
 * RETURNS
 * EOK success
 * EAGAIN the IN queue is empty
 * EINVAL pkt is NULL
 */
int netbuf_process_out(struct packet **pkt, net_interface_t ** intf);

/*
 * Wait for processing.
 * The function will suspend any calling thread until a driver or an application
 * add a packet to IN or OUT queues by means of netbuf_in() or netbuf_out().
 * The function will not suspend if IN or OUT queues have packets available for
 * processing.
 */

void netbuf_wait(void);

/*
 * Copy data from an ethernet frame stored in a buffer to a packet structure.
 * Pointers in packet structure are updated accordingly to the type of packet.
 *
 * PARAMETERS IN
 * const void *src - the source buffer carrying the ethernet frame
 * struct packet *pkt - the packet structure to be updated
 * unsigned bytes - the amount of data in the source buffer
 *
 * RETURNS
 * EOK success
 * EINVAL src or pkt or bytes are null
 */
int netbuf_copy_eth(const void *src, struct packet *pkt, unsigned bytes);

#endif
