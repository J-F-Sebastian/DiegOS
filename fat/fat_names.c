/*
 * FAT File System
 *
 * Copyright (C) 2019 Diego Gallizioli
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

#include <ctype.h>
#include <string.h>

#include "fat_names.h"

/* Update this define if you change the definition of struct FAT in fat_data.h !!! */
#define DIR_NAMELEN 39

static inline int check(const char chk)
{
	return ((chk < 0x20) || (chk == 0x22) ||
		((chk >= 0x2A) && (chk <= 0x2C)) ||
		((chk == 0x2F)) ||
		((chk >= 0x3A) && (chk <= 0x3F)) ||
		((chk >= 0x5B) && (chk <= 0x5D)) || (chk == 0x7C));
}

size_t FAT_parse_directory(const char *directory, size_t dirlen, size_t *startpos, size_t *endpos)
{
	while (*startpos < dirlen) {
		if (directory[*startpos] == '\\')
			break;
		(*startpos)++;
	}

	if (*startpos == dirlen)
		return 0;

	*startpos += 1;
	*endpos = *startpos + 1;
	while (*endpos < dirlen) {
		if (directory[*endpos] == '\\')
			break;
		(*endpos)++;
	}
	return (*endpos - *startpos);
}

size_t FAT_parse_directory_last(const char *directory, size_t dirlen)
{
	while (directory[--dirlen] != '\\') {
		if (!dirlen)
			break;
	}

	return dirlen;
}

int FAT_build_DIR_name(const char *name, size_t len, char *output)
{
	size_t pos = 0;

	if (len > DIR_NAMELEN)
		return -1;

	while (pos < len) {
		if (check(name[pos])) {
			output[0] = '\0';
			return -1;
		}
		output[pos] = name[pos];
		pos++;
	}

	/* Pad with trailing spaces */
	while (pos < DIR_NAMELEN)
		output[pos++] = ' ';

	return 0;
}

int FAT_make_path_relative(const char *filename, const char *relto, char *output)
{
	size_t startrelto, endrelto, reltolen, reltochunk;

	if (!filename || !relto || !output)
		return -1;

	/* skip over common root path */
	while (*filename == *relto) {
		if (*filename == '\0')
			return -1;
		++filename;
		++relto;
	}

	reltolen = strlen(relto);
	startrelto = 0;

	/*
	 * add a step back '..' for each not-common folder, and break at the end of
	 * the relto path.
	 */
	do {
		reltochunk = FAT_parse_directory(relto, reltolen, &startrelto, &endrelto);
		if (reltochunk) {
			strcpy(output, "..\\");
			output += 3;
		} else
			break;

		endrelto = startrelto;

	} while (1);

	strcat(output, filename);
	return 0;
}

int FAT_check_character(char chk)
{
	return (check(chk)) ? -1 : 0;
}
