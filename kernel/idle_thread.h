#ifndef IDLE_THREAD_H_INCLUDED
#define IDLE_THREAD_H_INCLUDED

/*******************************************
 *********** HEADER FILE *******************
 *******************************************
 *
 * Name:
 * Author:
 * Date:
 *******************************************
 * Description:
 *
 *******************************************/

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
