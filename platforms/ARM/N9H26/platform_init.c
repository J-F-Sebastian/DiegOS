/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2023 Diego Gallizioli
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <diegos/interrupts.h>
#include <diegos/delays.h>
#include <diegos/devices.h>
#include <diegos/drivers.h>
#include <N9H26_drivers.h>
#include <platform/N9H26.h>
#include "../../kernel/platform_include.h"

#define TVAL   12000000
#define TTHRES (TVAL/1000)

#if 0
static unsigned long tickfn(void)
{
	static unsigned long ticks = 0;
	unsigned rd;

	rd = N9H26r(N9H26TMR_BASE0, TDR0);

	if (rd > TTHRES) {
		/*
		 * Faster than a division
		 */
		while (rd > TTHRES) {
			rd -= TTHRES;
			++ticks;
		}
		/* Load timer counter */
		N9H26w(N9H26TMR_BASE0, TICR0, TVAL);
	}

	return ticks;
}
#endif

static int drivers_list_init(void)
{
	device_t *dev;

	dev = device_create(&n9h26_uart_drv, 1);
	if (!dev) return (ENODEV);

	dev = device_create(&n9h26_timer_drv, 0);
	if (!dev) return (ENODEV);

	return (EOK);
}

extern STATUS malloc_init(const void *heapstart, const void *heapend);
extern STATUS iomalloc_init(const void *start, const void *end);

void platform_init()
{
	void *heap_base;
	unsigned long heap_size;
	void *io_base;
	unsigned long io_size;

	cacheable_memory(&heap_base, &heap_size);
	io_memory(&io_base, &io_size);
	(void)malloc_init((const void *)heap_base, (const void *)(heap_base + heap_size));
	(void)iomalloc_init((const void *)io_base, (const void *)(io_base + io_size));

#if 0
	/*
	 * Perform hardcoded initialization of TIMER0 to calibrate delays
	 */
	N9H26u(N9H26CLK_BASE, APBCLK, TMR0_CKE);
	N9H26u(N9H26CLK_BASE, APBIPRST, TMR0RST);
	/*
	 * There should be some no-op here is it ?
	 */
	N9H26c(N9H26CLK_BASE, APBIPRST, TMR0RST);
	N9H26w(N9H26TMR_BASE0, TCSR0, CRST);
	N9H26cu(N9H26TMR_BASE0, TCSR0, TCSR0, TCSR_PRESCALE, 0);

	/* enable the timer, no interrupts */
	N9H26u(N9H26TMR_BASE0, TCSR0, CEN | TDR_EN);
	/* load timer counter, the timer runs as soon as the value is written */
	N9H26w(N9H26TMR_BASE0, TICR0, TVAL);
	/* one shot timer 0 */

	/*
	 * Now we are updating ticks !!! Go calibrate.
	 */
	calibrate_delay(tickfn);

	/* disable the timer */
	N9H26c(N9H26TMR_BASE0, TCSR0, CEN | TDR_EN);

	/*
	 * Disable the clock, reset the device
	 */
	N9H26c(N9H26CLK_BASE, APBCLK, TMR0_CKE);
	N9H26c(N9H26CLK_BASE, APBIPRST, TMR0RST);
	N9H26u(N9H26CLK_BASE, APBIPRST, TMR0RST);
	N9H26w(N9H26CLK_BASE, TCSR0, CRST);
#else
	set_delay(120000);
#endif

	srand(loops_per_second() / 123);
}

void tty_init()
{
}

void drivers_init()
{
	int retcode;

	retcode = drivers_list_init();

	if (EOK != retcode) {
		abort();
	}

	if ((STDIN_FILENO != open(DEFAULT_STDIN, O_RDONLY, 0)) ||
	    (STDOUT_FILENO != open(DEFAULT_STDOUT, O_WRONLY, 0)) ||
	    (STDERR_FILENO != open(DEFAULT_STDERR, O_WRONLY, 0))) {
		abort();
	}
}
