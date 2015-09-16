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

#ifndef IDLE_THREAD_H_INCLUDED
#define IDLE_THREAD_H_INCLUDED

/*
 * Idle thread entry point.
 * The idle thread will enter an infinite loop performing a
 * call to a power save function and then suspending.
 * The idle thread PID value is 0.
 * The idle thread CANNOT be killed, CANNOT be terminated,
 * CANNOT wait.
 * The idle priority is the lowest possible, so the idle thread
 * will run only when no other thread is running or ready.
 * Please note, it is possible to spawn threads with the same priority
 * as the idle thread; keep in mind that power save functions usually
 * halt the CPU until an interrupt is detected, so it might not be
 * very proficient running a thread with idle priority.
 */
void idle_thread_entry(void);

#endif // IDLE_THREAD_H_INCLUDED
