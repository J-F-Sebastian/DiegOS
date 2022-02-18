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

#ifndef _KERNEL_EVENTS_H_
#define _KERNEL_EVENTS_H_

/*
 * This enum will address classes for a variety
 * of kernel libraries or features.
 */

enum kernel_classes {
	/*
	 * Alarms as implemented in the kernel.
	 * Eventid is used to address the alarmid.
	 */
	CLASS_ALARM = 1,
	/*
	 * Devices as implemented under kernel libraries.
	 * Events are listed in devices_events.h
	 */
	CLASS_DEV = 2,
	/*
	 * Packet events as implemented in the kernel.
	 * Events are listed in network_events.h
	 */
	CLASS_NET = 3
};

#endif
