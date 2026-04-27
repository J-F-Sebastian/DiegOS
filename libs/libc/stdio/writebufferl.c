/**
 *
 *                                                   ,----..
 *        ,---,                                     /   /   \   .--.--.
 *      .'  .' `\    ,--,                          /   .     : /  /    '.
 *    ,---.'     \ ,--.'|                         .   /   ;.  \  :  /`. /
 *    |   |  .`\  ||  |,                ,----._,..   ;   /  ` ;  |  |--`
 *    :   : |  '  |`--'_       ,---.   /   /  ' /;   |  ; \ ; |  :  ;_
 *    |   ' '  ;  :,' ,'|     /     \ |   :     ||   :  | ; | '\  \    `.
 *    '   | ;  .  |'  | |    /    /  ||   | .\  ..   |  ' ' ' : `----.   \
 *    |   | :  |  '|  | :   .    ' / |.   ; ';  |'   ;  \; /  | __ \  \  |
 *    '   : | /  ; '  : |__ '   ;   /|'   .   . | \   \  ',  / /  /`--'  /
 *    |   | '` ,/  |  | '.'|'   |  / | `---`-'| |  ;   :    / '--'.     /
 *    ;   :  .'    ;  :    ;|   :    | .'__/\_: |   \   \ .'    `--'---'
 *    |   ,.'      |  ,   /  \   \  /  |   :    :    `---`
 *    '---'         ---`-'    `----'    \   \  /
 *                                       `--`-'
 */

/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2026 Diego Gallizioli
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

#include <unistd.h>
#include "loc_incl.h"

int writebufferl(int c, FILE *stream)
{
	size_t count;

	/*
	 * Save the char in the buffer.
	 * NOTE: This function IS REQUESTED TO BE USED INTERNALLY
	 * TO LIBC ONLY.
	 * stream->count is ASSUMED TO BE not 0 at all times when
	 * entering this function.
	 */
	--stream->count;
	*stream->bufptr++ = (char)c;

	/*
	 * Is the character a newline - or the buffer
	 * is full? Flush the buffer.
	 */
	if ((c == '\n') || !stream->count) {

		if (fileno(stream) < 0) {
			stream_setflags(stream, IOBUF_ERROR);
			return (EOF);
		}

		if (stream_testflags(stream, IOBUF_APPEND)) {
			if (lseek(fileno(stream), 0L, SEEK_END) == -1) {
				stream_setflags(stream, IOBUF_ERROR);
				return (EOF);
			}
		}
		count = stream->validsize - stream->count;
		stream->count = stream->validsize;
		stream->bufptr = stream->buffer;

		if (write(fileno(stream), stream->buffer, count) != (ssize_t) count) {
			stream_setflags(stream, IOBUF_ERROR);
			return (EOF);
		}

		if ((c == '\n') && !stream_isb(stream)) {
			if (write(fileno(stream), "\r", 1) < 0) {
				stream_setflags(stream, IOBUF_ERROR);
				return (EOF);
			}
		}
	}

	return (c);
}
