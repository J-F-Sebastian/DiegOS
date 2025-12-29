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

#include <stdlib.h>
#include <stdio.h>

#include <libs/red_black_tree.h>

#define DBG_MODULE_ON
#include <diegos/debug_traces.h>

enum {
	DIR_LEFT,
	DIR_RIGHT
};

/*
 * The following routines are internals, used for tree management
 */
static inline void color_node_red(rbtree_node_t *node)
{
	if (node) {
		node->flags |= NODE_IS_RED;
	}
}

static inline void color_node_black(rbtree_node_t *node)
{
	if (node) {
		node->flags &= ~NODE_IS_RED;
	}
}

static inline BOOL node_is_red(rbtree_node_t *node)
{
	return ((node && (node->flags & NODE_IS_RED)) ? TRUE : FALSE);
}

/*
 static inline BOOL node_is_black(rbtree_node_t * node)
 {
 return ((node && !(node->flags & NODE_IS_RED)) ? TRUE : FALSE);
 }
 */
static inline BOOL node_is_root(rbtree_node_t *node)
{
	return ((node && (node->flags & NODE_IS_ROOT)) ? TRUE : FALSE);
}

static inline void set_node_as_root(rbtree_node_t *node)
{
	if (node) {
		node->flags |= NODE_IS_ROOT;
	}
}

static inline void clear_node_as_root(rbtree_node_t *node)
{
	if (node) {
		node->flags &= ~NODE_IS_ROOT;
	}
}

/*
 static inline BOOL node_is_leaf(rbtree_node_t *node)
 {
 return ((node && !(node->left) && !(node->right)) ? TRUE : FALSE);
 }
 */

/*
 * common binary tree rotation
 */
static rbtree_node_t *rotate_right(rbtree_node_t *root)
{
	rbtree_node_t *save = root->left;
	root->left = save->right;
	save->right = root;
	color_node_red(root);
	color_node_black(save);
	if (node_is_root(root)) {
		set_node_as_root(save);
		clear_node_as_root(root);
	}
	return (save);
}

/*
 * common binary tree rotation
 */
static rbtree_node_t *rotate_left(rbtree_node_t *root)
{
	rbtree_node_t *save = root->right;
	root->right = save->left;
	save->left = root;
	color_node_red(root);
	color_node_black(save);
	if (node_is_root(root)) {
		set_node_as_root(save);
		clear_node_as_root(root);
	}
	return (save);
}

static inline rbtree_node_t *rotate(rbtree_node_t *root, int dir)
{
	return ((DIR_RIGHT == dir) ? rotate_right(root) : rotate_left(root));
}

/*
 * common binary tree double rotation, just a cascade of two rotations
 * in opposite directions.
 */
static rbtree_node_t *double_rotate_right(rbtree_node_t *root)
{
	root->left = rotate_left(root->left);
	return (rotate_right(root));
}

/*
 * common binary tree double rotation, just a cascade of two rotations
 * in opposite directions.
 */
static rbtree_node_t *double_rotate_left(rbtree_node_t *root)
{
	root->right = rotate_right(root->right);
	return (rotate_left(root));
}

static inline rbtree_node_t *double_rotate(rbtree_node_t *root, int dir)
{
	return ((DIR_RIGHT == dir) ? (double_rotate_right(root)) : (double_rotate_left(root)));
}

static inline rbtree_node_t *get_child(rbtree_node_t *node, int dir)
{
	return ((DIR_RIGHT == dir) ? node->right : node->left);
}

static inline void set_child(rbtree_node_t *node, int dir, rbtree_node_t *target)
{
	if (DIR_RIGHT == dir) {
		node->right = target;
	} else {
		node->left = target;
	}
}

/* static void *get_from_rbtree_internal searches for a node given the key.
 * returns a tree node.
 * INTERNAL USE ONLY
 */
static rbtree_node_t *get_from_rbtree_internal(rbtree_node_t *root,
					       rbtree_node_t *target, rbtreecmpfn cmpfn)
{
	int cmpresval;
	while (root) {
		cmpresval = cmpfn(root, target);
		switch (cmpresval) {
		case KEY_GREATER:
			root = root->left;
			break;
		case KEY_LOWER:
			root = root->right;
			break;
		case KEY_EQUAL:
			return (root);
		}
	}
	return (root);
}

static rbtree_node_t *find_common(rbtree_node_t *root,
				  rbtree_node_t *target, rbtreecmpfn cmpfn, rbtree_node_t **parents)
{
	BOOL found = FALSE;
	int retval;
	while ((!found) && root) {
		retval = cmpfn(root, target);
		switch (retval) {
		case KEY_GREATER:
			parents[DIR_RIGHT] = root;
			root = root->left;
			break;
		case KEY_LOWER:
			parents[DIR_LEFT] = root;
			root = root->right;
			break;
		case KEY_EQUAL:

			//gotcha! break the loop
			found = TRUE;
			break;
		}
	}
	return (root);
}

/*
 * find_in_rbtree searches for a match in the tree; if none is found, returns
 * the closest lower or higher node in the tree as if the target was to be inserted
 * in the tree itself.
 * If a match is found, walk the tree to find the closes lower or higher node in the tree.
 */
static rbtree_node_t *find_in_rbtree(rbtree_node_t *root,
				     rbtree_node_t *target, rbtreecmpfn cmpfn, const BOOL lower)
{
	rbtree_node_t *lastp[] = {
		NULL, NULL
	};
	int dir;
	root = find_common(root, target, cmpfn, lastp);
	dir = (TRUE == lower) ? DIR_LEFT : DIR_RIGHT;
	if (root && get_child(root, dir)) {
		root = get_child(root, dir);
		while (root && get_child(root, !dir)) {
			root = get_child(root, !dir);
		}
		return (root);
	} else {
		return (lastp[dir]);
	}
}

/*
 * find_in_rbtree searches for a match in the tree; if none is found, returns
 * NULL.
 * If a match is found, walk the tree to find the closes lower or higher node in the tree.
 */
static rbtree_node_t *find_in_rbtree_exact(rbtree_node_t *root,
					   rbtree_node_t *target,
					   rbtreecmpfn cmpfn, const BOOL lower)
{
	rbtree_node_t *lastp[] = {
		NULL, NULL
	};
	int dir;
	root = find_common(root, target, cmpfn, lastp);
	if (!root) {
		return (NULL);
	}
	dir = (TRUE == lower) ? DIR_LEFT : DIR_RIGHT;
	if (get_child(root, dir)) {
		root = get_child(root, dir);
		while (root && get_child(root, !dir)) {
			root = get_child(root, !dir);
		}
		return (root);
	} else {
		return (lastp[dir]);
	}
}

/*
 * recursive implementation of insertion in the red black tree.
 * Courtesy of Julienne Walker and the EC team
 * From the twisted minds of Julienne Walker and The EC Team
 * http://eternallyconfuzzled.com
 */
static rbtree_node_t *insert_recursive(rbtree_node_t *root,
				       rbtree_node_t *newn, rbtreecmpfn cmpfn, BOOL *dup)
{
	int cmpval;
	int dir;
	rbtree_node_t *temp = NULL;
	if (!root) {
		root = newn;
	} else {
		cmpval = cmpfn(root, newn);
		if (KEY_EQUAL != cmpval) {
			dir = (KEY_GREATER == cmpval) ? DIR_LEFT : DIR_RIGHT;

			/*
			 * init recursion navigating the tree down to the bottom,
			 * childs are next iteration's root.
			 */
			set_child(root,
				  dir, insert_recursive(get_child(root, dir), newn, cmpfn, dup));

			/*
			 * balance subtree
			 */
			if (node_is_red(get_child(root, dir))) {
				if (node_is_red(get_child(root, !dir))) {
					color_node_red(root);
					color_node_black(root->left);
					color_node_black(root->right);
				} else {
					temp = get_child(root, dir);
					if (node_is_red(get_child(temp, dir))) {
						root = rotate(root, !dir);
					} else if (node_is_red(get_child(temp, !dir))) {
						root = double_rotate(root, !dir);
					}
				}
			}
		} else {

			/*
			 * mark a missed insertion (duplicate)
			 */
			*dup = TRUE;
		}
	}
	return (root);
}

/*
 * Global, public functions
 */
rbtree_node_t *rbtree_insert(rbtree_node_t **root, rbtree_node_t *node, rbtreecmpfn cmpfn)
{
	BOOL dup = FALSE;

	if (!root || !node || !cmpfn) {
		return (NULL);
	}

	/*
	 * Common node setup
	 */
	node->left = NULL;
	node->right = NULL;
	color_node_red(node);

	/*
	 * Case 1: the tree is void. Set and return
	 */
	if (!(*root)) {
		set_node_as_root(node);
		color_node_black(node);
		*root = node;
		return (node);
	}

	/*
	 * Case 2: walk the tree and insert the node
	 */
	*root = insert_recursive(*root, node, cmpfn, &dup);

	/*
	 * insertion failed due to a duplicated node
	 */
	if (TRUE == dup) {
		return (NULL);
	}
	color_node_black(*root);
	set_node_as_root(*root);
	return (node);
}

/*
 * recursive implementation of extraction in the red black tree.
 * Courtesy of Julienne Walker and the EC team
 * From the twisted minds of Julienne Walker and The EC Team
 * http://eternallyconfuzzled.com
 */
static rbtree_node_t *rbtree_extract_balance(rbtree_node_t *root, int dir, BOOL *done)
{
	rbtree_node_t *p = root;
	rbtree_node_t *s = get_child(root, !dir);
	BOOL save, new_root;

	/*
	 * rotate and update root, first balancing step
	 */
	if (node_is_red(s)) {
		root = rotate(root, dir);
		s = get_child(p, !dir);
	}

	/*
	 * second step, recover from removal unbalancing
	 */
	if (s) {
		if (!node_is_red(s->left) && !node_is_red(s->right)) {
			if (node_is_red(p)) {
				*done = TRUE;
			}
			color_node_black(p);
			color_node_red(s);
		} else {

			/*
			 * save data as we are going to rotate pointers
			 * root will be lost&restored
			 */
			save = node_is_red(root);
			new_root = (root == p) ? TRUE : FALSE;
			if (node_is_red(get_child(s, !dir))) {
				p = rotate(p, dir);
			} else {
				p = double_rotate(p, dir);
			}
			if (save) {
				color_node_red(p);
			} else {
				color_node_black(p);
			}
			color_node_black(p->left);
			color_node_black(p->right);

			/*
			 * if root did not change, we rotated
			 * his children and we need to restore
			 * the parent connection
			 */
			if (new_root) {
				root = p;
			} else {
				set_child(root, dir, p);
				color_node_red(p);
			}
			*done = TRUE;
		}
	}
	return (root);
}

static rbtree_node_t *rbtree_extract_recursive(rbtree_node_t *root,
					       rbtree_node_t *target,
					       rbtreecmpfn cmpfn, BOOL *done, rbtree_node_t **retv)
{
	int dir;
	int res;
	rbtree_node_t *save = NULL;
	rbtree_node_t *heir = NULL;
	if (!root) {
		*done = TRUE;
	} else {
		res = cmpfn(root, target);
		if (KEY_EQUAL == res) {
			if (!root->left || !root->right) {

				/*
				 * unlinking is performed by returning
				 * the child instead of root, this will link the parent
				 * to the 2nd order child (or link a child to his grandpa..)
				 */
				save = get_child(root, (!root->left) ? DIR_RIGHT : DIR_LEFT);
				if (node_is_red(root)) {
					*done = TRUE;
				} else if (node_is_red(save)) {
					color_node_black(save);
					*done = TRUE;
				}
				*retv = root;
				return (save);
			} else {

				/*
				 * need to find the in order next node
				 */
				heir = root->left;
				while (heir && heir->right) {
					heir = heir->right;
				}
			}
		}
		dir = (KEY_LOWER == res) ? DIR_RIGHT : DIR_LEFT;
		save = get_child(root, dir);

		/*
		 * recursively look for the node to extract
		 */
		set_child(root, dir,
			  rbtree_extract_recursive(save, (heir) ? heir : target, cmpfn, done,
						   retv));

		/*
		 * unlink root, and replace it with heir, so we just insert
		 * an existing node in another place and splice the required one.
		 */
		if (heir) {
			heir->left = root->left;
			heir->right = root->right;
			heir->flags = root->flags;
			root->left = NULL;
			root->right = NULL;
			*retv = root;
			root = heir;
		}

		/*
		 * rebalance nodes if needed
		 */
		if (!*done) {
			root = rbtree_extract_balance(root, dir, done);
		}
	}
	return (root);
}

rbtree_node_t *rbtree_extract(rbtree_node_t **root, rbtree_node_t *target, rbtreecmpfn cmpfn)
{
	BOOL done = FALSE;
	rbtree_node_t *retval = NULL;
	if (!root || !*root || !target || !cmpfn) {
		return (NULL);
	}
	*root = rbtree_extract_recursive(*root, target, cmpfn, &done, &retval);
	if (*root) {
		color_node_black(*root);
		set_node_as_root(*root);
	}
	return (retval);
}

rbtree_node_t *rbtree_search(rbtree_node_t *root, rbtree_node_t *target, rbtreecmpfn cmpfn)
{
	if (!root || !target || !cmpfn) {
		return (NULL);
	}
	return (get_from_rbtree_internal(root, target, cmpfn));
}

rbtree_node_t *rbtree_first(rbtree_node_t *root)
{
	rbtree_node_t *first = root;
	while (first && first->left) {
		first = first->left;
	}
	return (first);
}

rbtree_node_t *rbtree_last(rbtree_node_t *root)
{
	rbtree_node_t *last = root;
	while (last && last->right) {
		last = last->right;
	}
	return (last);
}

rbtree_node_t *rbtree_previous(rbtree_node_t *root, rbtree_node_t *target, rbtreecmpfn cmpfn)
{
	if (!root || !target || !cmpfn) {
		return (NULL);
	}
	return (find_in_rbtree_exact(root, target, cmpfn, TRUE));
}

rbtree_node_t *rbtree_next(rbtree_node_t *root, rbtree_node_t *target, rbtreecmpfn cmpfn)
{
	if (!root || !target || !cmpfn) {
		return (NULL);
	}
	return (find_in_rbtree_exact(root, target, cmpfn, FALSE));
}

rbtree_node_t *rbtree_search_previous(rbtree_node_t *root, rbtree_node_t *target, rbtreecmpfn cmpfn)
{
	if (!root || !target || !cmpfn) {
		return (NULL);
	}
	return (find_in_rbtree(root, target, cmpfn, TRUE));
}

rbtree_node_t *rbtree_search_next(rbtree_node_t *root, rbtree_node_t *target, rbtreecmpfn cmpfn)
{
	if (!root || !target || !cmpfn) {
		return (NULL);
	}
	return (find_in_rbtree(root, target, cmpfn, FALSE));
}

#ifdef DBG_MODULE_ON
void rbtree_print(rbtree_node_t *root, rbtreeprnfn prnfn)
{
	if (root) {
		rbtree_print(root->left, prnfn);
		printf(" **************\n");
		printf(" Node %p\n", root);
		printf(" Node left %p right %p\n", root->left, root->right);
		printf(" Node is %s\n", (node_is_red(root)) ? "RED" : "BLACK");
		if (node_is_root(root)) {
			printf(" Node is ROOT\n");
		}
		if (prnfn) {
			prnfn(root);
		}
		printf(" **************\n\n");
		rbtree_print(root->right, prnfn);
	}
}

int rbtree_verify(rbtree_node_t *root, rbtreecmpfn cmpfn)
{
	int lh, rh;
	rbtree_node_t *ln, *rn;
	if (!root) {
		return 1;
	} else {
		ln = root->left;
		rn = root->right;
		if (node_is_red(root)) {
			if (node_is_red(ln) || node_is_red(rn)) {
				printf("RED VIOLATION at node %p!\n", root);
				printf(" Node %p is %s\n", root,
				       (node_is_red(root)) ? "RED" : "BLACK");
				printf(" Node %p is %s\n", ln, (node_is_red(ln)) ? "RED" : "BLACK");
				printf(" Node %p is %s\n", rn, (node_is_red(rn)) ? "RED" : "BLACK");
				return 0;
			}
		}
		lh = rbtree_verify(ln, cmpfn);
		rh = rbtree_verify(rn, cmpfn);
		if ((ln && (cmpfn(ln, root) != KEY_LOWER))
		    || (rn && (cmpfn(rn, root) != KEY_GREATER))) {
			printf("BINARY TREE VIOLATION at node %p!\n", root);
			return 0;
		}
		if (lh && rh && (lh != rh)) {
			printf("BLACK VIOLATION at node %p!\n", root);
			return 0;
		}
		if (lh && rh) {
			return (node_is_red(root)) ? lh : lh + 1;
		} else {
			return 0;
		}
	}
}

#endif
uint32_t rbtree_count(rbtree_node_t *root)
{
	if (root) {
		return (1 + rbtree_count(root->left) + rbtree_count(root->right));
	}
	return (0);
}

uint32_t rbtree_enumerate_inorder_range(rbtree_node_t *root, rbtree_node_t *low,
					rbtree_node_t *high, rbtreecmpfn cmpfn,
					rbtree_node_t **list)
{
	rbtree_node_t *start, *stop, *cursor;
	uint32_t counter = 0;
	if (!root || !low || !high || !cmpfn) {
		return (0);
	}
	start = rbtree_search(root, low, cmpfn);
	if (!start) {
		start = rbtree_search_next(root, low, cmpfn);
	}
	stop = rbtree_search(root, high, cmpfn);
	if (!stop) {
		stop = rbtree_search_previous(root, high, cmpfn);
	}
	if (!(start && stop)) {
		return (0);
	}
	if (KEY_LOWER != cmpfn(start, stop)) {
		return (0);
	}
	for (cursor = start; cursor != stop; cursor = rbtree_next(root, cursor, cmpfn)) {
		if (list) {
			list[counter] = cursor;
		}
		++counter;
	}

	/*
	 * last one
	 */
	if (list) {
		list[counter] = cursor;
	}
	return (counter + 1);
}

static
void rbtree_enum_recursive(rbtree_node_t *root, rbtree_node_t **item, uint32_t *pos)
{
	if (root->left) {
		rbtree_enum_recursive(root->left, item, pos);
	}
	item[*pos] = root;
	*pos = *pos + 1;
	if (root->right) {
		rbtree_enum_recursive(root->right, item, pos);
	}
}

void rbtree_enumerate_inorder(rbtree_node_t *root, rbtree_node_t **list)
{
	uint32_t pos = 0;
	rbtree_enum_recursive(root, list, &pos);
}

void rbtree_enumerate_layered(rbtree_node_t *root, rbtree_node_t **list)
{
	uint32_t r1, r2, c;
	if (!root || !list) {
		return;
	}
	r1 = 0;
	r2 = c = 1;
	list[r1] = root;

	do {
		r2 = c;
		while (r1 != r2) {
			if (list[r1]->left) {
				list[c++] = list[r1]->left;
			}
			if (list[r1]->right) {
				list[c++] = list[r1]->right;
			}
			++r1;
		}
	} while (r2 != c);
}

static BOOL rbtree_walk_recursive(rbtree_node_t *root, rbtreewlkfn wlkfn, void *param)
{
	BOOL retval;
	if (!root) {
		return (TRUE);
	}
	retval = rbtree_walk_recursive(root->left, wlkfn, param);
	if (FALSE == retval) {
		return (FALSE);
	}
	retval = wlkfn(root, param);
	if (FALSE == retval) {
		return (FALSE);
	}
	retval = rbtree_walk_recursive(root->right, wlkfn, param);
	return (retval);
}

void rbtree_walk(rbtree_node_t *root, rbtreewlkfn wlkfn, void *param)
{
	if (!root || !wlkfn) {
		return;
	}
	(void)rbtree_walk_recursive(root, wlkfn, param);
}
