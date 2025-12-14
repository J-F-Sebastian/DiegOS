/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2025 Diego Gallizioli
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
#include <errno.h>

#include <platform/N9H26.h>
#include <platform/interrupts.h>
#include "N9H26_TIMER.h"
#include "N9H26_TIMER_private.h"

static unsigned ctrvalue = TIMER_MAX_VAL;
static unsigned flags[] = {DRV_IS_CHAR, DRV_IS_CHAR, DRV_IS_CHAR, DRV_IS_CHAR};
static unsigned regunit = 0;

typedef void (*cbfunc)(void);
static cbfunc cbfn[] = {NULL, NULL, NULL, NULL};

static const struct tmrvalues {
	uint32_t interrupt;
	uint32_t clk;
	uint32_t clkrst;
	uint32_t tmrbase;
	uint32_t tmrcsr;
	uint32_t tmricr;
	uint32_t tmrtdr;
} tmrval[] = {
	{ TMR0_INT, TMR0_CKE, TMR0RST, N9H26TMR_BASE0, TCSR0, TICR0, TDR0},
	{ TMR1_INT, TMR1_CKE, TMR1RST, N9H26TMR_BASE0, TCSR1, TICR1, TDR1},
	{ TMR2_INT, TMR2_CKE, TMR2RST, N9H26TMR_BASE1, TCSR2, TICR2, TDR2},
	{ TMR3_INT, TMR3_CKE, TMR3RST, N9H26TMR_BASE1, TCSR3, TICR3, TDR3}
};

static BOOL int_handler0()
{
	if (cbfn[0]) {
		cbfn[0]();
	}

	return (TRUE);
}

static BOOL int_handler1()
{
	if (cbfn[1]) {
		cbfn[1]();
	}

	return (TRUE);
}

static BOOL int_handler2()
{
	if (cbfn[2]) {
		cbfn[2]();
	}

	return (TRUE);
}

static BOOL int_handler3()
{
	if (cbfn[3]) {
		cbfn[3]();
	}

	return (TRUE);
}

/*
 * Handle 4 devices system wide
 */
static int timer_init(unsigned unitno)
{
	if (unitno > 3) {
		return (ENXIO);
	}

	/*
        * Check if already initialized
        */
	if (regunit & (1 << unitno)) {
		return (EPERM);
	}

	regunit |= (1 << unitno);

	/*
	 * Enable the clock, reset the device
	 */
	N9H26u(N9H26CLK_BASE, APBCLK, tmrval[unitno].clk);
	N9H26u(N9H26CLK_BASE, APBIPRST, tmrval[unitno].clkrst);
	N9H26c(N9H26CLK_BASE, APBIPRST, tmrval[unitno].clkrst);
	N9H26w(tmrval[unitno].tmrbase, tmrval[unitno].tmrcsr, CRST);
	N9H26cu(tmrval[unitno].tmrbase,
		tmrval[unitno].tmrcsr,
		tmrval[unitno].tmrcsr,
		TCSR_PRESCALE,
		TIMER_PRESCALE);
	
	switch (unitno)
	{
		case 0:
			if (EOK != add_int_cb(int_handler0, TMR0_INT)) {
				return (EPERM);
			}
		break;

		case 1:
			if (EOK != add_int_cb(int_handler1, TMR1_INT)) {
				return (EPERM);
			}
		break;

		case 2:
			if (EOK != add_int_cb(int_handler2, TMR2_INT)) {
				return (EPERM);
			}
		break;

		case 3:
			if (EOK != add_int_cb(int_handler3, TMR3_INT)) {
				return (EPERM);
			}
		break;
	}

	return (EOK);
}

static int timer_start(unsigned unitno)
{
	if (unitno > 3) {
		return (ENXIO);
	}

	if (flags[unitno] & DRV_STATUS_RUN) {
		return (EOK);
	}

	flags[unitno] &= ~DRV_STATUS_STOP;
	flags[unitno] |= DRV_STATUS_RUN;

	enable_int(tmrval[unitno].interrupt);

	/* enable the timer */
	N9H26u(tmrval[unitno].tmrbase, tmrval[unitno].tmrcsr, CEN | TCSR_IE | TDR_EN);
	/* load timer counter, the timer runs as soon as the value is written */
	N9H26w(tmrval[unitno].tmrbase, tmrval[unitno].tmricr, ctrvalue);

	return (EOK);
}

static int timer_stop(unsigned unitno)
{
	if (unitno > 3) {
		return (ENXIO);
	}

	if (flags[unitno] & DRV_STATUS_STOP) {
		return (EOK);
	}

	flags[unitno] &= ~DRV_STATUS_RUN;
	flags[unitno] |= DRV_STATUS_STOP;

	/* disable the timer */
	N9H26c(tmrval[unitno].tmrbase, tmrval[unitno].tmrcsr, CEN | TCSR_IE | TDR_EN);

	disable_int(tmrval[unitno].interrupt);

	return (EOK);
}

static int timer_done(unsigned unitno)
{
	if (unitno > 3) {
		return (ENXIO);
	}

	/*
	 * Disable the clock, reset the device
	 */
	N9H26c(N9H26CLK_BASE, APBCLK, tmrval[unitno].clk);
	N9H26c(N9H26CLK_BASE, APBIPRST, tmrval[unitno].clkrst);
	N9H26u(N9H26CLK_BASE, APBIPRST, tmrval[unitno].clkrst);
	N9H26w(tmrval[unitno].tmrbase, tmrval[unitno].tmrcsr, CRST);
	
	return (del_int_cb(tmrval[unitno].interrupt));
}

static int timer_ioctrl(void *data, unsigned opcode, unsigned unitno)
{
	unsigned *udata = (unsigned *)data;

	/* All opcodes require data */
	if (!data) {
		return (EINVAL);
	}

	if (unitno > 3) {
		return (ENXIO);
	}

	switch (opcode) {
	case CLK_SET_PERIOD:
		if (!udata[0])
			udata[0]++;
		if ((udata[0] < TIMER_MIN_VAL) || (udata[0] > TIMER_MAX_VAL))
			return (EINVAL);
		ctrvalue = udata[0];
		return (EOK);

	case CLK_GET_PARAMS:
		*udata++ = TIMER_FREQ;
		*udata++ = TIMER_MIN_VAL;
		*udata++ = TIMER_MAX_VAL;
		return (EOK);

	case CLK_GET_ELAPSED:
		*udata = N9H26r(tmrval[unitno].tmrbase, tmrval[unitno].tmrtdr);
		*udata = ctrvalue - *udata;
		return (EOK);

	case CLK_SET_MODE:
		if (udata[0] > 1) {
			return (EINVAL);
		}

		/* set timer to run continuously or one-shot */
		if ((udata[0] == 0) || (udata[0] == 1)) {
			N9H26cu(tmrval[unitno].tmrbase,
				tmrval[unitno].tmrcsr,
				tmrval[unitno].tmrcsr,
				TCSR_MODE,
				(udata[0]) ? 0 : (1 << 27));
			return (EOK);
		} else {
			return (EINVAL);
		}

	case CLK_SET_CB:
		cbfn[unitno] = data;
		return (EOK);

	default:
		return (EINVAL);
	}
}

static unsigned timer_status(unsigned unitno)
{
	if (unitno < 4) {
		return (flags[unitno]);
	}

	return (0);
}

char_driver_t n9h26_timer_drv = {
	.cmn = {
		.name = "clk",
		.init_fn = timer_init,
		.start_fn = timer_start,
		.stop_fn = timer_stop,
		.done_fn = timer_done,
		.ioctrl_fn = timer_ioctrl,
		.status_fn = timer_status,
		.poll_fn = NULL}
	,
	.write_fn = NULL,
	.read_fn = NULL,
	.write_multi_fn = NULL,
	.read_multi_fn = NULL
};
