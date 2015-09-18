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

#ifndef SYSTEM_PARSER_API_H_INCLUDED
#define SYSTEM_PARSER_API_H_INCLUDED

#define SYSTEM_PARSER_CMD_MAX (15)
#define SYSTEM_PARSER_HELP_MAX (31)

enum {
    SYSTEM_PARSER_FLAG_CMD  = (1 << 0),
    SYSTEM_PARSER_FLAG_INT  = (1 << 1),
    SYSTEM_PARSER_FLAG_UINT = (1 << 2),
    SYSTEM_PARSER_FLAG_HEX  = (1 << 3),
    SYSTEM_PARSER_FLAG_STR  = (1 << 4)
};

/*
 * Structure defining a parser tree node, carrying chunk name,
 * help, next node (if this one is selected), alternate (if more than
 * one can be selected).
 *
 *   <node1> - next - <node2> - next - <node4>
 *      |                             |
 *   alternate                        |
 *      |                             |
 *   <node11> - next -<node3> - next -+
 *      |
 *   alternate
 *      |
 *   <node12> - next - NULL
 *      |
 *   alternate
 *      |
 *    NULL
 */
struct parser_tree {
    char chunk[SYSTEM_PARSER_CMD_MAX + 1];
    char help[SYSTEM_PARSER_HELP_MAX + 1];
    unsigned flag;
    struct parser_tree *next;
    struct parser_tree *alternate;
};

void system_parser_new_tree (struct parser_tree *tree);

struct parser_tree *system_parser_add_cmd (const char *name,
                                           const char *help,
                                           unsigned flag,
                                           struct parser_tree *next,
                                           struct parser_tree *alternate);

#endif // SYSTEM_PARSER_MACROS_H_INCLUDED
