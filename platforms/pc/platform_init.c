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
#include "../../drivers/tty/vga_tty.h"
#include "../../drivers/ti16550d/16550d.h"
#include "../../drivers/i8253/i8253.h"
#include "../../drivers/i82371SB/82371SB.h"
#include "../../drivers/lo/local_loop.h"
#include "../../include/libs/pci_lib.h"
#include "../../drivers/i8042/i8042.h"

/*
 * WARNING
 * THIS MUST BE KEPT ALIGNED WITH CODE IN THE BOOT SECTOR !!!
 * THE VARIABLES AREA IS AT 0xF000
 */
struct boot_variables {
    uint32_t    free_heap_start;
    uint32_t    free_heap_size;
};

static const void *ttylist[] = { &vga_tty_drv };

static const void *drvlist[] = { &uart16550d_drv,
                                 &i8253_drv,
                                 &i82371sb_drv,
                                 &i8042_kbd_drv,
                                 &i8042_mouse_drv
                               };

static volatile unsigned long ticks = 0;

static void calib_int_handler(void)
{
    ++ticks;
}

extern STATUS malloc_init(const void *heapstart, const void *heapend);

void platform_init()
{
    uint32_t params[3];
    struct boot_variables *bootvars = (struct boot_variables *)0xF000;

    (void) malloc_init((const void *) (bootvars->free_heap_start),
                       (const void *) (bootvars->free_heap_start +
                                       bootvars->free_heap_size));

    /*
     * Init interrupts, all PIC lines are disabled
     */
    i8259_init();

    /*
     * enable interrupts
     */
    unlock();

    /*
     * Init the clock to calibrate delay loops.
     * MUST be called AFTER malloc_init.
     * It will be inited twice when added as a device, not elegant, but....
     */
    if ((EOK != i8253_drv.cmn.init_fn(0)) ||
	    (EOK != i8253_drv.cmn.ioctrl_fn(params, CLK_GET_PARAMS, 0)) ||
            (EOK != i8253_drv.cmn.ioctrl_fn(&params[1], CLK_SET_PERIOD, 0)) ||
            (EOK != i8253_drv.cmn.ioctrl_fn(calib_int_handler, CLK_SET_CB, 0)) ||
            (EOK != i8253_drv.cmn.start_fn(0))) {
        abort();
    }

    /*
     * Now we are updating ticks !!! Go calibrate.
     */
    calibrate_delay(&ticks);

    srand(ticks);

    /*
     * Stop the driver, it will be inited by driver_init
     */
    i8253_drv.cmn.stop_fn(0);
    i8253_drv.cmn.done_fn(0);

    /*
     * disable interrupts, interrupts MUST BE DISABLED after this function
     * has been called.
     */
    lock();

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

