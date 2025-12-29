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

#include <string.h>
#include <errno.h>
#include <diegos/interrupts.h>
#include <diegos/kernel.h>
#include <diegos/delays.h>

#include "kprintf.h"
#include "platform_include.h"

extern STATUS malloc_init(const void *heapstart, const void *heapend);
extern STATUS iomalloc_init(const void *start, const void *end);

static const char DiegOS_banner[] = "DiegOS operating system release 1.0";

extern long _text_start, _text_end, _data_start, _data_end, _bss_start, _bss_end;

/*
 * This is the main entry point, loaded at a platform-specific location.
 * The linker should be instructed to produce a binary relocated at this
 * location.
 * DiegOS will be the entry point right after the boot code has finished
 * loading the system.
 * When entering DiegOS you *must* have setup the following:
 *
 * - A small stack for initial procedure calls (4K bytes should be enough)
 * - All general purpose registers set to null
 * - Flat addressing, highest execution priority, i.e. DiegOS must execute with
 *   full privileges
 * - Interrupts must be disabled.
 */

void DiegOS(void)
{
	const char *proctype;
	long *bss = &_bss_start;
	void *heap_start;
	unsigned long heap_size;
	void *iomem_start;
	unsigned long iomem_size;
	unsigned long mem_size;

	/*
	 * Complete .BSS initialization
	 */
	while (bss < &_bss_end)
		*bss++ = 0;

	/*
	 * Complete processor initialization.
	 */
	proctype = processor_init();

	/*
	 * Initialize the platform. DO NOT init
	 * drivers here, just setup basic hardware for proper running.
	 * In case you need to run some specific driver (i.e. system clocks, RTC, etc.)
	 * check carefully the driver code path, the system at this point is still a bare
	 * hardware running headless.
	 * In case you run drivers, CALL the methods stop and done before exiting the
	 * function.
	 * Remember that libraries are not initialized yet, you cannot use malloc(),
	 * printf() or any other system call.
	 */
	platform_init();

	/*
	 * Init memory allocators.
	 * Get heap and I/O memory area from platform code.
	 */
	total_memory(&mem_size);
	cacheable_memory(&heap_start, &heap_size);
	io_memory(&iomem_start, &iomem_size);

	/*
	 * Complete memory allocators initialization.
	 * Cannot print out anything at this stage.
	 */
	if (EOK != malloc_init((const void *)heap_start, (const void *)(heap_start + heap_size))) {
		kernel_done();
	}
	if (EOK != iomalloc_init((const void *)iomem_start, (const void *)(iomem_start + iomem_size))) {
		kernel_done();
	}
	/*
	 * Disable interrupts.
	 */
	lock();

	/*
	 * Init the TTY, so we can print debug traces.
	 * This is the only call where you're allowed to init a video
	 * driver, or a serial driver.
	 * Let the following call init drivers, so that DiegOS can adjust
	 * it's tables.
	 */
	tty_init();

	kmsgprintf("%s\n\n", DiegOS_banner);

#if 0
	kmsgprintf("        | Text   | Data   | BSS    | Heap   | I/O\n");
	kmsgprintf("--------+--------+--------+--------+--------+--------\n");
	kmsgprintf("Start   |%#8x|%#8x|%#8x|%#8x|%#8x\n",
		   &_text_start, &_data_start, &_bss_start, (uintptr_t) (heap_start),
		   (uintptr_t) iomem_start);
	kmsgprintf("--------+--------+--------+--------+--------+--------\n");
	kmsgprintf("End     |%#8x|%#8x|%#8x|%#8x|%#8x\n",
		   &_text_end, &_data_end, &_bss_end, (uintptr_t) (heap_start) + heap_size,
		   (uintptr_t) (iomem_start) + iomem_size);
	kmsgprintf("--------+--------+--------+--------+--------+--------\n\n");
	kmsgprintf("RAM Size: %d MBytes\n", mem_size/MBYTE);
#endif
	if (proctype) {
		kmsgprintf("processor is a %s\n", proctype);
	}
#if defined(ENABLE_SIMD)
	kmsgprintf("SIMD instructions are supported.\n");
#endif
#if defined(ENABLE_FP)
	kmsgprintf("Floating Point instructions are supported.\n");
#endif

	/*
	 * delay loop is reasonably a 2-instructions loop - a decrease
	 * and a jump - on any processor.
	 */
	kmsgprintf("%d BogoMIPS\n", loops_per_second() / 500000);
	/*
	 * Init the kernel system libraries.
	 */
	kernel_libs_init();

	/*
	 * Init all drivers and create devices and interfaces.
	 * This is the right step to init interrupts routines,
	 * power management functions, browse busses, etc.
	 * NOTE: do not enable interrupts, do not enable power save modes,
	 * although you can configure them.
	 * Here the system is still initing, it cannot use threads or system
	 * calls.
	 */
	drivers_init();

	/*
	 * Init the kernel.
	 */
	kernel_init();

	/*
	 * Init the kernel threads, running OS code
	 */
	kernel_threads_init();

	/*
	 * Enable interrupts.
	 */
	unlock();

	/*
	 * Init all applications code, remember that
	 * the scheduler (and then threads) will be running after kernel_run()
	 * is invoked.
	 */
	platform_run();

	/*
	 * Roll the ball !!!
	 */
	kernel_run();

	/*
	 * Last step: thou shalt not get in here !!!
	 */
}
