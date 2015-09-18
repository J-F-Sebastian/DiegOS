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

#ifndef _VGA_PRIVATE_H_
#define _VGA_PRIVATE_H_

#define VGA_AC_INDEX        (0x3C0)
#define VGA_AC_WRITE        (0x3C0)
#define VGA_AC_READ         (0x3C1)
#define VGA_MISC_WRITE      (0x3C2)
#define VGA_SEQ_INDEX       (0x3C4)
#define VGA_SEQ_DATA        (0x3C5)
#define VGA_DAC_READ_INDEX  (0x3C7)
#define VGA_DAC_WRITE_INDEX (0x3C8)
#define VGA_DAC_DATA        (0x3C9)
#define VGA_MISC_READ       (0x3CC)
#define VGA_GC_INDEX        (0x3CE)
#define VGA_GC_DATA         (0x3CF)

/* COLOR emulation MONO emulation */

#define VGA_CRTC_INDEX      (0x3D4) /* 0x3B4 */
#define VGA_CRTC_DATA       (0x3D5) /* 0x3B5 */
#define VGA_INSTAT_READ     (0x3DA)

#define VGA_VIDEO_BUFFER1   (0xA0000)
#define VGA_VIDEO_BUFFER2   (0xB0000)
#define VGA_VIDEO_BUFFER3   (0xB8000)

#endif
