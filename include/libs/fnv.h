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

#ifndef FNV_H_INCLUDED
#define FNV_H_INCLUDED

/*
 * FNV-1a hash algorithm API
 */

#include <stdint.h>

uint64_t fnv_buf_64(void *buffer, unsigned size);

uint64_t fnv_str_64(char *str);

uint32_t fnv_buf_32(void *buffer, unsigned size);

uint32_t fnv_str_32(char *str);

uint16_t fnv_buf_16(void *buffer, unsigned size);

uint16_t fnv_str_16(char *str);

#endif				// FNV_H_INCLUDED
