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
#include <diegos/net_interfaces.h>
#include <libs/chunks.h>
#include <libs/red_black_tree.h>
#include <string.h>

#include "kprintf.h"

typedef struct tree_node {
    rbtree_node_t header;
    net_interface_t *ni;
} tree_node_t;

/*
 * interface_tree is sorted by name
 */
static rbtree_node_t *interface_tree = NULL;
/*
 * interface_tree_if is sorted by ifindex
 */
static rbtree_node_t *interface_tree_if = NULL;
/*
 * Chunks pool for tree nodes and interfaces
 */
static chunks_pool_t *tree_node_pool = NULL;
static chunks_pool_t *interfaces_pool = NULL;

/*
 * run-time counter to assign ifindexes
 */
static int ifindex_counter = 0;

static int cmpfn(const rbtree_node_t *a, const rbtree_node_t *b)
{
    tree_node_t *aa = (tree_node_t *) a;
    tree_node_t *bb = (tree_node_t *) b;
    int retcode;

    retcode = strncmp(aa->ni->name, bb->ni->name, sizeof(aa->ni->name));

    if (retcode > 0) {
        return (KEY_GREATER);
    }
    if (retcode < 0) {
        return (KEY_LOWER);
    }
    return (KEY_EQUAL);
}

static int cmpfnif(const rbtree_node_t *a, const rbtree_node_t *b)
{
    tree_node_t *aa = (tree_node_t *) a;
    tree_node_t *bb = (tree_node_t *) b;
    int retcode = (aa->ni->ifindex - bb->ni->ifindex);

    if (retcode > 0) {
        return (KEY_GREATER);
    }
    if (retcode < 0) {
        return (KEY_LOWER);
    }
    return (KEY_EQUAL);
}

BOOL init_net_interfaces_lib()
{
    tree_node_pool = chunks_pool_create("Intf tree",
                                        CACHE_ALN,
                                        sizeof(tree_node_t),
                                        2 * 8,
                                        2 * 8);

    if (!tree_node_pool) {
        return (FALSE);
    }

    interfaces_pool = chunks_pool_create("Intf",
                                         CACHE_ALN,
                                         sizeof(net_interface_t),
                                         8,
                                         8);

    if (!interfaces_pool) {
        /*
         * Not handled memory leak as the system will shut down on failure
         */
        return (FALSE);
    }

    return (TRUE);
}

void start_net_interfaces_lib(void)
{
    tree_node_t *ptr = (tree_node_t *) rbtree_first(interface_tree);

    /*
     * Now loop and start drivers.
     */
    while (ptr) {
        if (ptr->ni && ptr->ni->drv && ptr->ni->drv->start_fn(ptr->ni->unit)) {
            kerrprintf("Failed starting %s\n", ptr->ni->name);
        } else {
            kmsgprintf("Interface %s started, MTU %u\n",
                       ptr->ni->name,
                       ptr->ni->mtu);
        }
        ptr = (tree_node_t *) rbtree_next(interface_tree, &ptr->header, cmpfn);
    }
}

/*
 * Public section
 */

net_interface_t *net_interface_create(net_driver_t *inst)
{
    net_interface_t *temp;
    tree_node_t *node, *node2;
    unsigned temp2 = 0;

    if (!inst) {
        return (NULL);
    }

    temp = chunks_pool_malloc(interfaces_pool);
    node = chunks_pool_malloc(tree_node_pool);
    node2 = chunks_pool_malloc(tree_node_pool);

    if (!(temp && node && node2)) {
        if (temp) {
            chunks_pool_free(interfaces_pool, temp);
        }
        if (node) {
            chunks_pool_free(tree_node_pool, node);
        }
        if (node2) {
            chunks_pool_free(tree_node_pool, node2);
        }
        return (NULL);
    }

    node->ni = node2->ni = temp;
    /*node->ni = temp;*/
    temp->drv = inst;
    temp->mtu = temp->drv->mtu;
    temp->ifindex = ifindex_counter++;
    snprintf(temp->name, sizeof(temp->name),"%s%u",temp->drv->ifname, temp2);

    if (!rbtree_insert(&interface_tree, &node->header, cmpfn) ||
        !rbtree_insert(&interface_tree_if, &node2->header, cmpfnif)) {
        chunks_pool_free(interfaces_pool, temp);
        chunks_pool_free(tree_node_pool, node);
        chunks_pool_free(tree_node_pool, node2);
        return (NULL);
    }

    return (temp);
}

net_interface_t *net_interface_lookup_name(const char *ifname)
{
    tree_node_t temp;
    net_interface_t itemp;
    rbtree_node_t *retval;

    if (!ifname) {
        return (NULL);
    }

    strncpy(itemp.name, ifname, sizeof(itemp.name));
    temp.ni = &itemp;

    retval = rbtree_search(interface_tree, &temp.header, cmpfn);

    return ((retval) ? (((tree_node_t *) retval)->ni) : NULL);
}

net_interface_t *net_interface_lookup_index(const int ifindex)
{
    tree_node_t temp;
    net_interface_t itemp;
    rbtree_node_t *retval;

    itemp.ifindex = ifindex;
    temp.ni = &itemp;

    retval = rbtree_search(interface_tree, &temp.header, cmpfnif);

    return ((retval) ? (((tree_node_t *) retval)->ni) : NULL);
}

net_interface_t *net_interface_first()
{
    rbtree_node_t *retval;

    retval = rbtree_first(interface_tree);

    return ((retval) ? (((tree_node_t *) retval)->ni) : NULL);
}

net_interface_t *net_interface_next(net_interface_t *intp)
{
    rbtree_node_t *retval;
    tree_node_t temp;

    temp.ni = intp;

    retval = rbtree_next(interface_tree, &temp.header, cmpfn);

    return ((retval) ? (((tree_node_t *) retval)->ni) : NULL);
}
