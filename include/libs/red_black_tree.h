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

#ifndef _RED_BLACK_TREE_
#define _RED_BLACK_TREE_

#include <types_common.h>

/*
 * return enum for compare functions, comparing is first param towards
 * second param
 */
enum {
    KEY_LOWER = -1, KEY_EQUAL = 0, KEY_GREATER = 1
};

enum {
    NODE_IS_RED = 1, NODE_IS_ROOT = 2
};

/*
 * basic node struct, you do not need to mess with this header.
 * just add it as first field of your data struct.
 *
 * typedef struct mystuff {
 *          rbtree_node_t header;
 *          int mystuff;
 *  } foo;
 */
typedef struct rbtree_node {
    struct rbtree_node *left;
    struct rbtree_node *right;
    unsigned flags;
} rbtree_node_t;

typedef int (*rbtreecmpfn)(const rbtree_node_t *, const rbtree_node_t *);

typedef void (*rbtreeprnfn)(const rbtree_node_t *);

typedef BOOL (*rbtreewlkfn)(rbtree_node_t *, void *);

/*
 * insert a new node in the tree structure.
 * Memory management is up to the caller, root and node must be pre allocated.
 * PARAM:
 *  rbtree_node_t **root -  pointer to the tree's root pointer, if the tree is void
 *                        insert will store the first node inside the root.
 *  rbtree_node_t * node -  new node pointer.
 *  rbtreecmpfn cmpfn  -  function callback to compare nodes
 * RETURNS:
 *  NULL               -  Some error was detected, or the node cannot be inserted
 *  <value>            -  node value
 */
rbtree_node_t *rbtree_insert(rbtree_node_t ** root,
                             rbtree_node_t * node,
                             rbtreecmpfn cmpfn);

/*
 * Extract a node from the tree structure.
 * Extraction is not deletion, memory management is up to the caller; this routine
 * will only unlink the node from the tree structure.
 * PARAM:
 *  rbtree_node_t **root -  pointer to the tree's root pointer.
 *  rbtree_node_t * target -  node to be removed; the node can be either linked in the tree
 *                          or not
 *  rbtreecmpfn cmpfn  -  function callback to compare nodes
 * RETURNS:
 *  NULL               -  Some error was detected, or the node cannot be extracted
 *  <value>            -  extracted node value, matches target if target was linked in the tree
 */
rbtree_node_t *rbtree_extract(rbtree_node_t ** root,
                              rbtree_node_t *target,
                              rbtreecmpfn cmpfn);

/*
 * Search for a node in the tree structure.
 * PARAM:
 *  rbtree_node_t *root   -  pointer to the tree's root.
 *  rbtree_node_t * target -  target node the node can be either linked in the tree
 *                          or not
 *  rbtreecmpfn cmpfn  -  function callback to compare nodes
 * RETURNS:
 *  NULL               -  Some error was detected, or the node cannot be found
 *  <value>            -  pointer to the node
 */
rbtree_node_t *rbtree_search(rbtree_node_t * root,
                             rbtree_node_t * target,
                             rbtreecmpfn cmpfn);

/*
 * Get the first item in tree order.
 * PARAM:
 *  rbtree_node_t *root -  pointer to the tree's root.
 * RETURNS:
 *  NULL               -  Some error was detected, or the tree is void
 *  <value>            -  pointer to the first in-order node
 */
rbtree_node_t *rbtree_first(rbtree_node_t * root);

/*
 * Get the last item in tree order.
 * PARAM:
 *  rbtree_node_t *root -  pointer to the tree's root.
 * RETURNS:
 *  NULL               -  Some error was detected, or the tree is void
 *  <value>            -  pointer to the last in-order node
 */
rbtree_node_t *rbtree_last(rbtree_node_t * root);

/*
 * Get the node preceding the target in the tree.
 * Target can be either linked in the tree or not, but it must match a node in the
 * tree, otherwise NULL will be returned.
 * PARAM:
 *  rbtree_node_t *root   -  pointer to the tree's root.
 *  rbtree_node_t *target -  node pointer, the preceding of we are interested in.
 *                         can be linked in or not, but the key must match an existing
 *                         node.
 * RETURNS:
 *  NULL               -  Some error was detected, or the target cannot be found
 *  <value>            -  pointer to the preceding in-order node
 */
rbtree_node_t *rbtree_previous(rbtree_node_t * root,
                               rbtree_node_t * target,
                               rbtreecmpfn cmpfn);

/*
 * Get the node following the target in the tree.
 * Target can be either linked in the tree or not, but it must match a node in the
 * tree, otherwise NULL will be returned.
 * PARAM:
 *  rbtree_node_t *root   -  pointer to the tree's root.
 *  rbtree_node_t *target -  node pointer, the following of we are interested in.
 *                         can be linked in or not, but the key must match an existing
 *                         node.
 * RETURNS:
 *  NULL               -  Some error was detected, or the target cannot be found
 *  <value>            -  pointer to the following in-order node
 */
rbtree_node_t *rbtree_next(rbtree_node_t * root,
                           rbtree_node_t * target,
                           rbtreecmpfn cmpfn);

/*
 * Get the node preceding the target.
 * Target can be either linked in the tree or not.
 * target can be a key not present in the tree, the lowest closest key in the tree will
 * be returned.
 * PARAM:
 *  rbtree_node_t *root   -  pointer to the tree's root.
 *  rbtree_node_t *target -  node pointer, the preceding of we are interested in.
 *                         can be linked in or not.
 * RETURNS:
 *  NULL               -  Some error was detected, or the target cannot be found
 *  <value>            -  pointer to the preceding in-order node
 */
rbtree_node_t *rbtree_search_previous(rbtree_node_t * root,
                                      rbtree_node_t * target,
                                      rbtreecmpfn cmpfn);

/*
 * Get the node following the target.
 * Target can be either linked in the tree or not.
 * target can be a key not present in the tree, the greatest closest key in the tree will
 * be returned.
 * PARAM:
 *  rbtree_node_t *root   -  pointer to the tree's root.
 *  rbtree_node_t *target -  node pointer, the following of we are interested in.
 *                         can be linked in or not.
 * RETURNS:
 *  NULL               -  Some error was detected, or the target cannot be found
 *  <value>            -  pointer to the following in-order node
 */
rbtree_node_t *rbtree_search_next(rbtree_node_t * root,
                                  rbtree_node_t * target,
                                  rbtreecmpfn cmpfn);

/*
 * Look for a range of items, the boundaries are low and high keys; low and high
 * can be linked in the tree or not, and their keys can be present or not
 * in the tree's nodes.
 * If low and high cannot be found, then the next and previous items respectively
 * will be used.
 * The caller must provide enough items in *list to copy all pointers.
 * The output is a sorted array of rbtree_node_t pointers, copied from the tree into
 * the list.
 * PARAM:
 *  rbtree_node_t *root   -  pointer to the tree's root.
 *  rbtree_node_t *low
 * rbtree_node_t *high    -  node pointers storing the keys' range extents. Nodes
 *                         with low and high keys are included in the returned list.
 * rbtree_node_t **list   - pointer to an array of rbtree_node_t pointers, nodes will be
 *                        copied in here.
 *                        If list is null, the routine will count nodes but not copy
 *                        pointers.
 * RETURNS:
 *  <value>            -  number of items copied in list.
 */
unsigned rbtree_enumerate_inorder_range(rbtree_node_t *root,
                                        rbtree_node_t *low,
                                        rbtree_node_t *high,
                                        rbtreecmpfn cmpfn,
                                        rbtree_node_t **list);

/*
 * Output the tree to list, in tree order.
 * The caller must provide enough items in *list to copy all pointers.
 * The output is an array of rbtree_node_t pointers, copied from the tree into
 * the list.
 * PARAM:
 *  rbtree_node_t *root   -  pointer to the tree's root.
 * rbtree_node_t **list   - pointer to an array of rbtree_node_t pointers, nodes will be
 *                        copied in here.
 * RETURNS:
 */
void rbtree_enumerate_inorder(rbtree_node_t *root, rbtree_node_t **list);
/*
 * Output the tree to list, in layer order instead of tree order, i.e. putting the root
 * and his children recursively into list.
 * The caller must provide enough items in *list to copy all pointers.
 * The output is an array of rbtree_node_t pointers, copied from the tree into
 * the list.
 * PARAM:
 *  rbtree_node_t *root   -  pointer to the tree's root.
 * rbtree_node_t **list   - pointer to an array of rbtree_node_t pointers, nodes will be
 *                        copied in here.
 * RETURNS:
 */
void rbtree_enumerate_layered(rbtree_node_t *root, rbtree_node_t **list);

/*
 * Return the number of nodes linked in the tree. This routine can be called
 * for intermediate nodes too; in this case, it will return the number of items
 * in the subtree having the node as root.
 * PARAM:
 *  const rbtree_node_t *root   -  pointer to the tree's root.
 * RETURNS:
 *  <value>            -  number of nodes
 */
unsigned rbtree_count(rbtree_node_t * root);

/*
 * This routine walks down the tree and calls wlkfn for every node; if wlkfn
 * returns FALSE the walk stops.
 * The walk function takes 2 parameters: the first is the tree's node we are
 * walking into, the second is the param passed in to rbtree_walk.
 * PARAM:
 *  const rbtree_node_t *root   -  pointer to the tree's root.
 *  rbtreewlkfn wlkfn         - walk routine, return FALSE to stop the walk
 *  void *param               - generic parameter passed to wlkfn at every call.
 * RETURNS:
 */
void rbtree_walk(rbtree_node_t *root, rbtreewlkfn wlkfn, void *param);

#ifdef DBG_MODULE_ON
void rbtree_print(rbtree_node_t * root, rbtreeprnfn prnfn);

int rbtree_verify(rbtree_node_t * root, rbtreecmpfn cmpfn);
#endif

#endif
