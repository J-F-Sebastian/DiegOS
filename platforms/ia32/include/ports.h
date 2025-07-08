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

#ifndef _PORTS_H_
#define _PORTS_H_

extern uint8_t in_byte(const uint16_t portno);
extern uint16_t in_word(const uint16_t portno);
extern uint32_t in_dword(const uint16_t portno);

extern void out_byte(const uint16_t portno, const uint8_t val);
extern void out_word(const uint16_t portno, const uint16_t val);
extern void out_dword(const uint16_t portno, const uint32_t val);

#endif
