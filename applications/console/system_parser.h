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

#ifndef SYSTEM_PARSER_H_INCLUDED
#define SYSTEM_PARSER_H_INCLUDED

#include "system_parser_types.h"

void system_parser_init(const struct alternates *root);

/*
 * Parse the input buffer and execute the corresponding command, if any.
 * The buffer is not null-terminated, but the size is given as a parameter.
 *
 * PARAMETERS IN
 * const char *buffer: the input buffer to parse
 * unsigned bufsize: the size of the input buffer
 *
 * RETURNS
 *
 * EOK on success, the parsed string matches a true command and the command was executed successfully.
 * EINVAL if the input buffer is invalid (e.g., null pointer, zero size).
 * EILSEQ if the input buffer contains an unrecognized command or an invalid command format.
 */
int system_parser(const char *buffer, unsigned bufsize);

#endif				// SYSTEM_PARSER_H_INCLUDED
