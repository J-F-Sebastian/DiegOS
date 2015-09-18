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

#include <types_common.h>
#include <diegos/interrupts.h>
#include <diegos/devices.h>
#include <ia32/ports.h>
#include <pc/i8259.h>
#include <errno.h>

#include "i8253_private.h"
#include "i8253.h"

static unsigned resolution = TIMER_MIN_RES;
static unsigned flags = DRV_IS_CHAR;
static void (*cbfn)(void) = NULL;

static BOOL i8253_int_handler()
{
    if (cbfn) {
        cbfn();
    }

    return (TRUE);
}

/*
 * Handle 1 device system wide
 */
static int i8253_init(unsigned unitno)
{
    if (unitno) {
        return (ENXIO);
    }

    if (EOK != add_int_cb(i8253_int_handler, CLOCK_IRQ)) {
        return (EPERM);
    }

    return (EOK);
}

static int i8253_start(unsigned unitno)
{
    const uint8_t SQUARE_WAVE = (COUNTER0 | RW_LSB_MSB | MODE3);
    unsigned tc = TIMER_FREQ / resolution;

    if (unitno) {
        return (EINVAL);
    }

    if (flags & DRV_STATUS_RUN) {
        return (EOK);
    }

    if ((TIMER_FREQ % resolution) > resolution/2) {
        if (!(tc & 1)) {
            tc++;
        }
    }

    /*
     * Even values are preferrable
     */
    if (tc & 1) {
        tc++;
    }

    /* Initialize channel 0 of the 8253A timer */
    out_byte(TIMER_MODE, SQUARE_WAVE);	/* set timer to run continuously */
    out_byte(TIMER0, (uint8_t)tc);	/* load timer low byte */
    out_byte(TIMER0, (uint8_t)(tc >> 8));	/* load timer high byte */

    flags &= ~DRV_STATUS_STOP;
    flags |= DRV_STATUS_RUN;

    enable_int(CLOCK_IRQ);

    return (EOK);
}

static int i8253_stop(unsigned unitno)
{
    if (unitno) {
        return (EINVAL);
    }

    if (flags & DRV_STATUS_STOP) {
        return (EOK);
    }

    flags &= ~DRV_STATUS_RUN;
    flags |= DRV_STATUS_STOP;

    disable_int(CLOCK_IRQ);

    return (EOK);
}

static int i8253_done(unsigned unitno)
{
    const uint8_t SQUARE_WAVE = (COUNTER0 | RW_LSB_MSB | MODE3);

    if (unitno) {
        return (EINVAL);
    }

    /* Reset the clock to the BIOS rate. (For rebooting) */
    out_byte(TIMER_MODE, SQUARE_WAVE);
    out_byte(TIMER0, 0);
    out_byte(TIMER0, 0);

    return (del_int_cb(CLOCK_IRQ));
}


static int i8253_ioctrl(void *data, unsigned opcode, unsigned unitno)
{
    /* All opcodes require data */
    if (!data) {
        return (EINVAL);
    }

    switch (opcode) {
    case RTC_SET_FREQ:
        resolution = *(unsigned *)data & 0xFFFFUL;
        if (resolution < TIMER_MIN_RES) {
            resolution = TIMER_MIN_RES;
        } else if (resolution > TIMER_MAX_RES) {
            resolution = TIMER_MAX_RES;
        }
        i8253_stop(unitno);
        i8253_start(unitno);
        return (EOK);

    case RTC_SET_CB:
        cbfn = data;
        return (EOK);

    default:
        return (EINVAL);
    }
}

static unsigned i8253_status(unsigned unitno)
{
    if (!unitno) {
        return (flags);
    }

    return (0);
}

char_driver_t i8253_drv = {
    .name = "rtc",
    .init_fn = i8253_init,
    .start_fn = i8253_start,
    .stop_fn = i8253_stop,
    .done_fn = i8253_done,
    .write_fn = NULL,
    .read_fn = NULL,
    .write_multi_fn = NULL,
    .read_multi_fn = NULL,
    .ioctrl_fn = i8253_ioctrl,
    .status_fn = i8253_status
};

