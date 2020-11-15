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

/* SQUARE_WAVE, ONE_SHOT */
static const uint8_t modes[] = {(COUNTER0 | RW_LSB_MSB | MODE3), (COUNTER0 | RW_LSB_MSB | MODE1)};
static uint8_t mode = 0;
static unsigned period = TIMER_MAX_VAL;
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
    if (unitno) {
        return (EINVAL);
    }

    if (flags & DRV_STATUS_RUN) {
        return (EOK);
    }

    /* Initialize channel 0 of the 8253A timer */
    out_byte(TIMER_MODE, modes[mode]);	/* set timer to run continuously or one-shot */
    out_byte(TIMER0, (uint8_t)period);	/* load timer low byte */
    out_byte(TIMER0, (uint8_t)(period >> 8));	/* load timer high byte */

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
    if (unitno) {
        return (EINVAL);
    }

    /* Reset the clock to the BIOS rate. (For rebooting) */
    out_byte(TIMER_MODE, modes[0]);
    out_byte(TIMER0, 0);
    out_byte(TIMER0, 0);

    return (del_int_cb(CLOCK_IRQ));
}


static int i8253_ioctrl(void *data, unsigned opcode, unsigned unitno)
{
    unsigned *udata = (unsigned *)data;

    /* All opcodes require data */
    if (!data) {
        return (EINVAL);
    }

    switch (opcode) {
    case CLK_SET_PERIOD:
        period = udata[0]*TIMER_FREQ/1000;
        if ((udata[0]*TIMER_FREQ)%1000 > 499) {
            period++;
        }
	if (period & 1) {
            period++;
	}
        if ((period < TIMER_MIN_VAL) || (period > TIMER_MAX_VAL)) {
            return (EINVAL);
        }
        out_byte(TIMER0, (uint8_t)period);	/* load timer low byte */
        out_byte(TIMER0, (uint8_t)(period >> 8));	/* load timer high byte */
        return (EOK);

    case CLK_GET_PERIODS:
        udata[0] = 1; /* TIMER_MIN_VAL*1000/TIMER_FREQ; */
        udata[1] = TIMER_MAX_VAL*1000/TIMER_FREQ;
        return (EOK);

    case CLK_SET_MODE:
        if (udata[0] > 1) {
            return (EINVAL);
        }

        mode = (uint8_t)udata[0];
        out_byte(TIMER_MODE, modes[mode]);	/* set timer to run continuously or one-shot */
        return (EOK);

    case CLK_SET_CB:
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
    .cmn = {
		.name = "clk",
		.init_fn = i8253_init,
		.start_fn = i8253_start,
		.stop_fn = i8253_stop,
		.done_fn = i8253_done,
		.ioctrl_fn = i8253_ioctrl,
		.status_fn = i8253_status,
		.poll_fn = NULL
	},
    .write_fn = NULL,
    .read_fn = NULL,
    .write_multi_fn = NULL,
    .read_multi_fn = NULL    
};

