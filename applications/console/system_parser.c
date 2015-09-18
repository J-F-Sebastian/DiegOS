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

#include <libs/chunks.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "system_parser.h"
#include "system_parser_api.h"

static chunks_pool_t *system_parser_chunks_pool = NULL;

static struct parser_tree *system_root = NULL;

static int system_parser_insert (struct parser_tree *root,
                                 struct parser_tree *tree)
{
    int retcode;

    do {
        retcode = strncmp(root->chunk, tree->chunk, sizeof(root->chunk));

        if (0 == retcode) {
            return (EINVAL);
        } else if (retcode < 0) {
            root = root->alternate;
        } else {
            tree->alternate = root->alternate;
            root->alternate = tree;
            return (EOK);
        }
    } while (root != NULL);

    return (EINVAL);
}

static struct parser_tree *system_parser_search (struct parser_tree *root,
                                                 const char *name)
{
    int retcode;

    do {
        retcode = strncmp(root->chunk, name, sizeof(root->chunk));

        if (0 == retcode) {
            return (root);
        } else if (retcode < 0) {
            root = root->alternate;
        } else {
            return (NULL);
        }
    } while (root != NULL);

    return (NULL);
}

static void system_parser_print_help (struct parser_tree *root, BOOL printchain)
{
    if (printchain) {
        do {
            printf(" %-15s\t%s\n", root->chunk, root->help);
            root = root->alternate;
        } while (root);
    } else {
        printf(" %-15s\t%s\n", root->chunk, root->help);
    }
}

void system_parser_new_tree (struct parser_tree *tree)
{
    assert(tree != NULL);

    if (system_root) {
        assert(system_parser_insert(system_root, tree) == EOK);
    } else {
        system_root = tree;
    }
}

struct parser_tree *system_parser_add_cmd (const char *name,
                                           const char *help,
                                           unsigned flag,
                                           struct parser_tree *next,
                                           struct parser_tree *alternate)
{
    struct parser_tree *newnode;

    /* FIXME flag checks!!! */
    if (!name || !help) {
        return (NULL);
    }

    if (!system_parser_chunks_pool) {
        system_parser_chunks_pool = chunks_pool_create("SystemParser",
                                                       0,
                                                       sizeof(struct parser_tree),
                                                       16,
                                                       16);
        assert(system_parser_chunks_pool != NULL);
    }

    newnode = chunks_pool_malloc(system_parser_chunks_pool);
    assert(newnode != NULL);

    strncpy(newnode->chunk, name, sizeof(newnode->chunk));
    newnode->chunk[SYSTEM_PARSER_CMD_MAX] = 0;
    strncpy(newnode->help, help, sizeof(newnode->help));
    newnode->help[SYSTEM_PARSER_HELP_MAX] = 0;
    newnode->flag = flag;
    newnode->next = next;
    newnode->alternate = alternate;

    return (newnode);
}


void system_parser(const char *buffer, unsigned bufsize)
{
    char buffer2[128] = {0};
    char *token = NULL;
    struct parser_tree *cmdchain = system_root;

    if (!buffer || !bufsize) {
        return;
    }

    strncpy(buffer2, buffer, sizeof(buffer2));
    token = strtok(buffer2," ");

    while (token && cmdchain) {
        if (token[0] == '\t') {
            system_parser_print_help(cmdchain, TRUE);
            break;
        } else {
            cmdchain = system_parser_search(cmdchain, token);
            if (cmdchain) {
                cmdchain = cmdchain->next;
                token = strtok(NULL," ");
            } else {
                puts("SYNTAX ERROR");
                return;
            }
        }
    }
}
