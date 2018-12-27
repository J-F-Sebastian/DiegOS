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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pakman.h>
#include <errno.h>

struct packet_int {
    struct packet pkt;
    uint16_t chunks;
};

/*
 * A packet buffer is a complex data structure composed by the following
 * items:
 *
 * 1) a packet_int circular buffer, with pointers to storage data area.
 * 2) a storage data area, aligned to and multiple of a cache line.
 *
 * The rationale behind those structures is, packet_int points to data inside
 * storage, consuming a variable amount of bytes; several packet_int can be
 * allocated consuming storage, and they are stored as a circular buffer.
 * Storage is logically divided into cache lines and assigned to several
 * packet_int; space is consumed in chunks. The size of the circular buffer
 * is dictated by the size of the packet_int circual buffer.
 *
 *     Storage                              packet_int
 *  +--------------------+ <--------------- Item1, 0, 2     [2-0+1 = 3 chunks]
 *  |                    |      +---------- Item2, 3, 3     [3-3+1 = 1 chunk]
 *  |         0          |      |   +------ Item3, 4, 4     [4-4+1 = 1 chunk]
 *  |                    |      |   |
 *  |                    |      |   |
 *  +--------------------+      |   |
 *  |                    |      |   |
 *  |         1          |      |   |
 *  |                    |      |   |
 *  |                    |      |   |
 *  +--------------------+      |   |
 *  |                    |      |   |
 *  |         2          |      |   |
 *  |                    |      |   |
 *  |                    |      |   |
 *  +--------------------+ <----+   |
 *  |                    |          |
 *  |         3          |          |
 *  |                    |          |
 *  |                    |          |
 *  +--------------------+ <--------+
 *  |                    |
 *  |         4          |
 *  |                    |
 *  |                    |
 *  +--------------------+
 */

typedef struct packet_buffer {
    /*
     * flags for this buffer
     */
    uint16_t flags;
    /*
     * number of packets in the circular buffer of packet descriptors
     */
    uint16_t packets_number;
    /*
     * number of free chunks in the buffer
     */
    unsigned storage_free_chunks;
    /*
     * circular buffer of packet descriptors
     */
    struct packet_int *pkts;
    unsigned pkts_head, pkts_tail;
    /*
     * pointer to the overall allocated memory for slots
     */
    void *storage;
    /*
     * pointer to the first chunk in storage.
     */
    void *storage_first_chunk;
    /*
     * pointer to the next available chunk in storage.
     */
    void *storage_next_chunk;
    /*
     * pointer to the last chunk used as a sentinel.
     */
    void *storage_last_chunk;
} pakman;

static unsigned space_in_packet (pakman *pk)
{
    if (pk->pkts_head == pk->pkts_tail) return (pk->packets_number - 1);
    if (pk->pkts_head < pk->pkts_tail) {
        return (pk->pkts_head + pk->packets_number - pk->pkts_tail - 1);
    } else {
        return (pk->pkts_head - pk->pkts_tail - 1);
    }
}

static unsigned space_in_storage (pakman *pk)
{
    return (pk->storage_free_chunks);
}

int init_pakman (unsigned bytes, unsigned packets, pakman **pakmanptr)
{
    pakman *pk;
    unsigned slot_num;

    if (!bytes || !packets || !pakmanptr) {
        return EINVAL;
    }

    *pakmanptr = NULL;
    slot_num = ALNC(bytes)/CACHE_ALN;
    if (slot_num < packets) {
        fprintf(stderr,
                "%s: packets number was %u, set to %u\n",
                __FUNCTION__,
                packets,
                slot_num);
        packets = slot_num;
    }

    pk = calloc(1, sizeof(pakman));
    if (!pk) {
        return ENOMEM;
    }

    pk->pkts = calloc(packets, sizeof(struct packet_int));
    if (!pk->pkts) {
        free(pk);
        return ENOMEM;
    }

    pk->storage = calloc(slot_num, CACHE_ALN);
    if (!pk->storage) {
        free(pk->pkts);
        free(pk);
        return ENOMEM;
    }

    pk->storage_first_chunk = (void *)ALNC((intptr_t)pk->storage);
    pk->storage_next_chunk = pk->storage_first_chunk;
    pk->packets_number = packets;
    pk->storage_free_chunks = slot_num;
    pk->storage_last_chunk = pk->storage_first_chunk+CACHE_ALN*slot_num;
    *pakmanptr = pk;

    return EOK;
}

int delete_pakman (pakman *pakmanptr)
{
    if (!pakmanptr) {
        return EINVAL;
    }

    free(pakmanptr->storage);
    free(pakmanptr->pkts);
    free(pakmanptr);

    return EOK;
}

struct packet *pakman_get_packet (pakman *pakmanptr, uint16_t len)
{
    struct packet_int *ptr;

    if (!pakmanptr ||
            !space_in_packet(pakmanptr) ||
            !space_in_storage(pakmanptr)) {
        return (NULL);
    }

    len = ALNC(len);
    if (len > CACHE_ALN*space_in_storage(pakmanptr)) {
        return (NULL);
    }
    /*
     * Grab an available packet in the circular queue, it will be stored in ptr
     */
    ptr = &pakmanptr->pkts[pakmanptr->pkts_tail++];
    ptr->pkt.data = pakmanptr->storage_next_chunk;
    ptr->pkt.data_payload_start = ptr->pkt.data;
    ptr->pkt.data_payload_cursor = ptr->pkt.data_payload_start;
    ptr->pkt.data_size = len;
    ptr->pkt.data_payload_size = len;
    ptr->chunks = len/CACHE_ALN;

    pakmanptr->pkts_tail %= pakmanptr->packets_number;
    pakmanptr->storage_free_chunks -= ptr->chunks;
    pakmanptr->storage_next_chunk += len;
    if (pakmanptr->storage_next_chunk == pakmanptr->storage_last_chunk) {
        pakmanptr->storage_next_chunk = pakmanptr->storage_first_chunk;
    }

    return (&ptr->pkt);
}

int pakman_put_packet (pakman *pakmanptr, struct packet *pkt)
{
    struct packet_int *ptr;

    if (!pakmanptr || !pkt) {
        return (EINVAL);
    }

    ptr = (struct packet_int *)pkt;
    if (ptr != pakmanptr->pkts + pakmanptr->pkts_head) {
        return (EPERM);
    }

    pakmanptr->pkts_head++;
    pakmanptr->pkts_head %= pakmanptr->packets_number;
    pakmanptr->storage_free_chunks += ptr->chunks;
    memset(ptr, 0, sizeof(*ptr));

    return (EOK);
}

