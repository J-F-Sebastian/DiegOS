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

#ifndef _I8253_PRIVATE_H_
#define _I8253_PRIVATE_H_

enum i8253_ports {
    /* I/O port for timer channel 0 */
    TIMER0 = 0x40,
    /* I/O port for timer channel 1 */
    TIMER1 = 0x41,
    /* I/O port for timer channel 2 */
    TIMER2 = 0x42,
    /* I/O port for timer mode control */
    TIMER_MODE = 0x43
};

enum i8253_control {
    COUNTER0   = (0 << 6),
    COUNTER1   = (1 << 6),
    COUNTER2   = (2 << 6),
    READ_BACK  = (3 << 6),

    COUNTER_LATCH  = (0 << 4),
    RW_LSB         = (1 << 4),
    RW_MSB         = (2 << 4),
    RW_LSB_MSB     = (3 << 4),

    MODE0  = (0 << 1),
    MODE1  = (1 << 1),
    MODE2  = (2 << 1),
    MODE3  = (3 << 1),
    MODE4  = (4 << 1),
    MODE5  = (5 << 1),
    BCD    = (1 << 0)
};

/*
 * All values in Hz
 */
#define TIMER_FREQ      (1193182UL) /* clock frequency for timer in PC and AT */
#define TIMER_MAX_VAL   (65534)
#define TIMER_MIN_VAL   (256)

#endif

