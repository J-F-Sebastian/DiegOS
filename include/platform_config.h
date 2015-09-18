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

/*
 * Supported platforms (set for DIEGOS_PLAT):
 *
 * PC_IA32
 *
 */

#ifndef _PLATFORM_CONFIG_H_
#define _PLATFORM_CONFIG_H_

#if (DIEGOS_CPU == ia32)
#include "ia32.h"
#else
#error no cpu was defined!
#endif

#if (DIEGOS_PLAT == pc)
#include "pc.h"
#else
#error no platform was defined!
#endif

#endif

