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

#ifndef _HAL_KERNEL_INIT_H_
#define _HAL_KERNEL_INIT_H_

/*
 * This is the DiegOS Initialization interface and is part of
 * the HAL (Hardware Abstraction Layer).
 * This interface MUST be implemented for every build target.
 */

/*
 * Init the processor, setup all required data to run
 * DiegOS.
 * Returns a pointer to a null-terminated string describing
 * the processor.
 * This function MUST NOT make use of libc code.
 */
extern const char *processor_init(void);

/*
 * Init the platform, i.e. setup all required hw and
 * set the platform in ready-state for drivers and kernel
 * init.
 * Init all libraries requiring a platform dependent value
 * to run - malloc, ISA, EISA, PCI, PCIe, etc.
 */
extern void platform_init(void);

/*
 * Init the TTY, this is a stand-alone pre-driver init as
 * it might be very useful to have debug traces ASAP :-)
 */
extern void tty_init(void);

/*
 * Start drivers init and tables setup
 */
extern void drivers_init(void);

/*
 * Start the applications!
 */
extern void platform_run(void);

#endif
