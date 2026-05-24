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
#include <stdlib.h>

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

int FAT_parse_directory(const char *directory, size_t dirlen, struct FAT_split_directory *split_dir)
{
	size_t startpos = 0, endpos = 0;
	unsigned split_num = 0;
	unsigned split_max = 8;

	/*
	 * CAUTION !!! If you forgot to release memory allocated to splits, you are leaking memory.
	 * You MUST release memory only if the return value is 0.
	 */
	split_dir->splits = malloc(split_max * DIR_NAMELEN);
	if (!split_dir->splits)
		return -1;

	while (startpos < dirlen) {
		while ((startpos < dirlen) && (directory[startpos] == '\\'))
			startpos++;

		if (startpos == dirlen)
			break;

		endpos = startpos + 1;
		while ((endpos < dirlen) && (directory[endpos] != '\\'))
			endpos++;

		if ((endpos - startpos) > DIR_NAMELEN)
		{
			free(split_dir->splits);
			split_dir->splits = NULL;
			split_dir->numsplits = 0;
			return -1;
		}

		if (FAT_build_DIR_name(directory + startpos, endpos - startpos, &split_dir->splits[split_num*DIR_NAMELEN]))
		{
			free(split_dir->splits);
			split_dir->splits = NULL;
			split_dir->numsplits = 0;
			return -1;
		}

		if (++split_num == split_max) {
			split_max += 8;
			char *new_splits = realloc(split_dir->splits, split_max * DIR_NAMELEN);
			if (!new_splits) {
				free(split_dir->splits);
				split_dir->splits = NULL;
				split_dir->numsplits = 0;
				return -1;
			}
			split_dir->splits = new_splits;
		}
		startpos = endpos + 1;
	}

	split_dir->numsplits = split_num;
	return 0;
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
		output[pos] = (char)toupper(name[pos]);
		pos++;
	}

	/* Pad with trailing spaces */
	while (pos < DIR_NAMELEN)
		output[pos++] = ' ';

	return 0;
}

int FAT_make_path_relative(const char *filename, const char *relto, char *output)
{
	size_t reltolen;
	struct FAT_split_directory splitdir;

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

	/*
	 * add a step back '..' for each not-common folder, and break at the end of
	 * the relto path.
	 */
	if (FAT_parse_directory(relto, reltolen, &splitdir))
		return -1;

	while (splitdir.numsplits--) {
		strcpy(output, "..\\");
		output += 3;
	}

	free(splitdir.splits);
	strcat(output, filename);
	return 0;
}

int FAT_check_character(char chk)
{
	return (check(chk)) ? -1 : 0;
}
