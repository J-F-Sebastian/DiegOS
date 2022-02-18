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

#ifndef _PLATFORM_INCLUDE_H_
#define _PLATFORM_INCLUDE_H_

/*
 * The following externs *must* be defined for
 * every processor in use. the platform makefile
 * *must* link these symbols.
 */

/*
 * Thread context setup
 *
 * The fail safe pointer should be stored onto the stack as a
 * return address in case the thread entry function terminates
 * without calling the proper terminating function. 
 * NOTE: when adding data onto the stack, simulate pushes by changing
 * the stack pointer accordingly.
 */
extern void setup_context(void *stack_ptr, void *fail_safe, void *entry_point, void **ctx);
/*
 * Context switching: save registers and additional data
 * into <from>, load the same data from <to>
 * You MUST ensure that from and to are properly sized
 * and aligned.
 * Interrupts MUST BE DISABLED entering the function, and REENABLED
 * exiting the function.
 */
extern void switch_context(void **from, const void *to);

/*
 * Context loading: restore registers from a context
 * location in memory. This will lead to a context load,
 * not a switch - the older running thread context is not
 * saved to memory.
 * This is useful when spawning the first thread, as well as
 * when killing or exiting one.
 * Interrupts MUST BE DISABLED entering the function, and REENABLED
 * exiting the function.
 */
extern void load_context(const void *to);

/*
 * Context cleaning: release or re-init context memory.
 * Interrupts MUST BE DISABLED entering the function, and REENABLED
 * exiting the function.
 */
extern void cleanup_context(void *ctx, unsigned tid);

/*
 * Init the processor, setup all required data to run
 * DiegOS.
 * Returns a pointer to a null-terminated string describing
 * the processor.
 */
extern const char *processor_init(void);

/*
 * Init the platform, i.e. setup all required hw and
 * set the platfrom in ready-state for drivers and kernel
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

/*
 * Enable the interrupts for this platform; please note,
 * this is not as calling unlock, this call MUST enable
 * hw and sw interrupts at hardware level, i.e. the cpu
 * must be able to receive interrupts from devices or
 * software.
 */
extern void enable_interrupts(void);

/*
 * Power save function; this is invoked by the idle loop
 * in the idle thread.
 */
extern void power_save(void);

/*
 * Loop delay function; this function is a "waste loop" to implement
 * nsleep and/or usleep. It must tune the variable
 * extern unsigned long delay_loops_per_tick
 * declared in delays.h.
 */
extern void delay_loop(unsigned long loops);

#endif
