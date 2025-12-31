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

#ifndef MUTEX_DATA_H_INCLUDED
#define MUTEX_DATA_H_INCLUDED

#include <libs/cbuffers.h>

struct mutex {
	list_node header;
	struct cbuffer thread_ids;
	uint8_t ids_buffer[DIEGOS_MAX_THREADS];
	uint8_t locker_tid;
	char name[16];
};

#endif				// MUTEX_DATA_H_INCLUDED
