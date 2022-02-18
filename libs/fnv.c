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

#include "fnv.h"

static const uint64_t FNV_PRIME_64 = 1099511628211ULL;
static const uint64_t FNV_OFS_64 = 14695981039346656037ULL;

static const uint32_t FNV_PRIME_32 = 16777619UL;
static const uint32_t FNV_OFS_32 = 2166136261UL;

uint64_t fnv_buf_64(void *buffer, unsigned size)
{
	uint64_t hash = FNV_OFS_64;
	uint8_t *cur = (uint8_t *) buffer;
	uint8_t *end = (uint8_t *) (buffer + size);

	while (cur < end) {
		hash ^= (uint64_t) * cur++;
		hash *= FNV_PRIME_64;
	}

	return (hash);
}

uint64_t fnv_str_64(char *str)
{
	uint64_t hash = FNV_OFS_64;
	uint8_t *cur = (uint8_t *) str;

	while (*cur) {
		hash ^= (uint64_t) * cur++;
		hash *= FNV_PRIME_64;
	}

	return (hash);
}

uint32_t fnv_buf_32(void *buffer, unsigned size)
{
	uint32_t hash = FNV_OFS_32;
	uint8_t *cur = (uint8_t *) buffer;
	uint8_t *end = (uint8_t *) (buffer + size);

	while (cur < end) {
		hash ^= (uint32_t) * cur++;
		hash *= FNV_PRIME_32;
	}

	return (hash);
}

uint32_t fnv_str_32(char *str)
{
	uint32_t hash = FNV_OFS_32;
	uint8_t *cur = (uint8_t *) str;

	while (*cur) {
		hash ^= (uint32_t) * cur++;
		hash *= FNV_PRIME_32;
	}

	return (hash);
}

uint16_t fnv_buf_16(void *buffer, unsigned size)
{
	uint32_t hash = fnv_buf_32(buffer, size);

	return ((hash >> 16) ^ (hash & 0xffff));
}

uint16_t fnv_str_16(char *str)
{
	uint32_t hash = fnv_str_32(str);

	return ((hash >> 16) ^ (hash & 0xffff));
}
