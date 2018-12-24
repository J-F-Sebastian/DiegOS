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

#ifndef _82371SB_H_INCLUDED
#define _82371SB_H_INCLUDED

#include <diegos/drivers.h>

/*
 * Intel 82371 South Bridge chipset
 */

#define i82371SB_VID        (0x8086)
#define i82371SB_BRIDGE_DID (0x7000)
#define i82371SB_IDE_DID    (0x7010)
#define i82371AB_PWR_DID    (0x7113)
#define i82371SB_USB_DID    (0x7020)

extern char_driver_t i82371sb_drv;

#endif // 82371SB_H_INCLUDED
