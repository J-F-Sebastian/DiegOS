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

#include "../../kernel/platform_include.h"
#include "i8259.h"
#include "../ia32/ints_private.h"
#include "../ia32/ports.h"
#include "../ia32/apic.h"
#include "../../drivers/tty/vga_tty.h"
#include "../../drivers/ti16550d/16550d.h"
#include "../../drivers/i8253/i8253.h"
#include "../../drivers/i82371SB/82371SB.h"
#include "../../drivers/lo/local_loop.h"
#include "../../include/libs/pci_lib.h"
#include "../../drivers/i8042/i8042.h"
#include "../../drivers/LAPIC/lapic.h"

/*
 * Hardcoded values for calibration
 */
#define TIMER0 0x40
#define TMRMOD 0x43

#define TVAL   65534
#define TDELTA 1193
#define TTHRES (TVAL - TDELTA)

/*
 * WARNING
 * THIS MUST BE KEPT ALIGNED WITH CODE IN THE BOOT SECTOR !!!
 * THE VARIABLES AREA IS AT 0xF000
 */
struct boot_variables {
	uint32_t free_ram_start;
	uint32_t free_ram_size;
};

static const void *ttylist[] = { &vga_tty_drv };

static const void *drvlist[] = { &uart16550d_drv,
	&i8253_drv,
	&i82371sb_drv,
	&i8042_kbd_drv,
	&i8042_mouse_drv
};

static unsigned long tickfn(void)
{
	static unsigned long ticks = 0;
	unsigned rval = 0;
	uint8_t *cval = (uint8_t *) & rval;

	/* Latch values */
	out_byte(TMRMOD, 0);
	cval[0] = in_byte(TIMER0);
	cval[1] = in_byte(TIMER0);

	if (rval < TTHRES) {
		ticks += (TVAL - rval) / TDELTA;
		/* Start counter value */
		out_byte(TIMER0, (uint8_t) TVAL);
		out_byte(TIMER0, (uint8_t) (TVAL >> 8));
	}

	return ticks;
}

extern STATUS malloc_init(const void *heapstart, const void *heapend);

void platform_init()
{
	struct boot_variables *bootvars = (struct boot_variables *)0xF000;

	(void)malloc_init((const void *)(bootvars->free_ram_start),
			  (const void *)(bootvars->free_ram_start + bootvars->free_ram_size));

	/*
	 * Init interrupts, all PIC lines are disabled
	 */
	i8259_init();

	/*
	 * Perform hardcoded initialization of i8253 PIT to calibrate delays
	 */
	/* one shot timer 0 */
	out_byte(TMRMOD, 0x38);

	/* Start counter value */
	out_byte(TIMER0, (uint8_t) TVAL);
	out_byte(TIMER0, (uint8_t) (TVAL >> 8));

	/*
	 * Now we are updating ticks !!! Go calibrate.
	 */
	calibrate_delay(tickfn);

	/*
	 * Stop the i8253 PIT
	 */
	out_byte(TIMER0, 0);

	srand(loops_per_second() / 123);
}

void tty_init()
{
	int retcode;

	retcode = drivers_list_init(ttylist, NELEMENTS(ttylist));
	if (EOK != retcode) {
		abort();
	}
	vga_tty_drv.cmn.start_fn(0);
}

void drivers_init()
{
	unsigned i = BPS_115200;
	unsigned j = CHAR_8BITS | STOP_BIT;
	int retcode;

	if (EOK != pci_bus_init()) {
		return;
	}

	/*
	 * Alas, we need to patch the list if we support LAPIC
	 */
	if (is_apic_supported()) {
		drvlist[1] = &lapic_drv;
	}

	retcode = drivers_list_init(drvlist, NELEMENTS(drvlist));

	if (EOK != retcode) {
		abort();
	}

	uart16550d_drv.cmn.ioctrl_fn(&i, UART_SET_SPEED, 0);
	uart16550d_drv.cmn.ioctrl_fn(&j, UART_SET_BITS, 0);

	if ((STDIN_FILENO != open(DEFAULT_STDIN, O_RDONLY, 0)) ||
	    (STDOUT_FILENO != open(DEFAULT_STDOUT, O_WRONLY, 0)) ||
	    (STDERR_FILENO != open(DEFAULT_STDERR, O_WRONLY, 0))) {
		abort();
	}
}
