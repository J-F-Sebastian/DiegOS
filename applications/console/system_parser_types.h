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

#ifndef _SYSTEM_PARSER_TYPES_H_
#define _SYSTEM_PARSER_TYPES_H_

enum PARSER_TYPES {
	PARSER_VOID	= 1 << 0,
	PARSER_STRING	= 1 << 1,
	PARSER_INT	= 1 << 2,
	PARSER_FLOAT	= 1 << 3,
	PARSER_FUNC	= 1 << 4,

	PARSER_DEC	= 1 << 8,
	PARSER_HEX	= 1 << 9,
	PARSER_MAC	= 1 << 10,
	PARSER_IPv4	= 1 << 11,
	PARSER_IPv6	= 1 << 12,

	PARSER_PARAMS	= 0xFF << 24
};

typedef void (*system_parser_func0) (void);

struct alternates;

struct alt_command {
	/* enum PARSER_TYPES */
	const int flags;
	/* command to be typed in */
	const char *item;
	/* help */
	const char *help;
	/* flexible operand */
	const void *data;
	/* next alternate chain */
	const struct alternates *next;
};

struct alternates {
	const struct alt_command *commands;
	unsigned num_commands;
};

#endif
