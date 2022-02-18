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

#ifndef FAT_NAMES_H
#define FAT_NAMES_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief FAT_parse_directory parse a standard path string and split it in chunks, returning
 * in startpos and endpos the limits of the chunk.
 *
 * @param directory a string representing a path name
 * @param dirlen length of the string pointed by directory
 * @param startpos starting position of the chunk
 * @param endpos ending position of the chunk
 *
 * @return length of the chunk in bytes
 */
size_t FAT_parse_directory(const char *directory, size_t dirlen, size_t * startpos,
			   size_t * endpos);

/**
 * @brief FAT_parse_directory_last parse a standard path string and returns the position
 * of the last container, i.e. the last directory containing the final file or directory.
 * The returned position is taht of the '\\' folder separator.
 *
 * @param directory a string representing a path name
 * @param dirlen length of the string pointed by directory
 *
 * @return the ending position of the last containing folder of the pathname
 */
size_t FAT_parse_directory_last(const char *directory, size_t dirlen);

/**
 * @brief FAT_build_DIR_name converts a file name or directory name to a FAT name.
 * Name is converted to upper case. The returned string is null on failure.
 *
 * @param name directory entry to be converted
 * @param len length of the string pinted by name
 * @param output buffer storing the result, null string "" on failure
 *
 * @return 0 on success
 * @return -1 on failure
 */
int FAT_build_DIR_name(const char *name, size_t len, char *output);

int FAT_make_path_relative(const char *directory, const char *relto, char *output);

int FAT_check_character(char chk);

#endif				// FAT_NAMES_H
