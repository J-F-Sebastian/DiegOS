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

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "system_parser_types.h"

static const struct alternates *system_root = NULL;

const struct alt_command *system_parser_search (const struct alternates *a,
						const char *token)
{
	unsigned i = a->num_commands;
	const struct alt_command *cursor = a->commands;

	while (i--)
	{
		if (strcmp(token, cursor->item) == 0)
			return cursor;

		++cursor;
	}

	return NULL;
}

static void system_parser_print_help (const struct alternates *root, unsigned index)
{
	unsigned i = root->num_commands;
	const struct alt_command *cursor = root->commands;

	if (index == -1U)
	{
		while (i--)
		{
			fprintf(stderr, " %-15s\t%s\n", cursor->item, cursor->help);
			++cursor;
		}
	}
	else if (index < root->num_commands)
	{
		printf(" %-15s\t%s\n", cursor[index].item, cursor[index].help);
	}
}

void system_parser_init (const struct alternates *root)
{
	//FIXME ASSERT ON NULL
	system_root = root;
}

void system_parser(const char *buffer, unsigned bufsize)
{
    char buffer2[128];
    char *token = NULL, *saveptr = NULL;
    const struct alternates *cmdchain = system_root;
    const struct alt_command *cmd = NULL;

    if (!buffer || !bufsize) {
        return;
    }

    strncpy(buffer2, buffer, sizeof(buffer2));
    token = strtok_r(buffer2, " ", &saveptr);
    if (!token)
	    return;

    while (token && cmdchain) {
        if (token[0] == '\t') {
            system_parser_print_help(cmdchain, -1U);
            break;
        } else {
            cmd = system_parser_search(cmdchain, token);
            if (cmd) {
			if (cmd->next) {
			    cmdchain = cmd->next;
			    token = strtok_r(NULL, " ", &saveptr);
			} else {
				if (cmd->flags & PARSER_FUNC) {
					system_parser_func0 fn = cmd->data;
					if (fn) fn();
				}
				return;
			}
	    } else {
		    system_parser_print_help(cmdchain, -1U);
		    puts("SYNTAX ERROR !!!");
		    return;
	    }
	}
    }
}
