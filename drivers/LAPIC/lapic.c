/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2022 Diego Gallizioli
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
#include <diegos/delays.h>
#include <errno.h>
#include <ia32/ia32.h>
#include <ia32/apic.h>

#include "lapic.h"
#include "lapic_private.h"

/*
 * Reference frequency is 10 MHz
 */
#define REF_FREQ 10000000

/* Defaults to periodic */
static uint8_t mode = 0;
static uint8_t divisor = 0;
static unsigned ctrvalue = TIMER_MAX_VAL;
static unsigned frequency = 0;
static unsigned flags = DRV_IS_CHAR;
static void (*cbfn) (void) = NULL;

static BOOL lapic_int_handler()
{
	if (cbfn) {
		cbfn();
	}

	return (TRUE);
}

static void calibrate_lapic()
{
	unsigned reads[3];

	/*
	 * Start the counter
	 * */
	apic_write_counter(-1UL);
	mdelay(1);
	reads[0] = apic_read_counter();
	mdelay(1);
	reads[1] = apic_read_counter();
	mdelay(1);
	reads[2] = apic_read_counter();
	reads[2] = reads[1] - reads[2];
	reads[1] = reads[0] - reads[1];
	reads[0] = (-1UL) - reads[0];
	reads[0] = (reads[0] + reads[1] + reads[2]) / 3;
	/*
	 * We have the APIC ticks per millisecond,
	 * we can compute the frequency of its clock
	 */
	frequency = reads[0] * 1000;

	/*
	 * Stop the counter
	 */
	apic_write_counter(0);
}

static void calibrate_divisor()
{
	unsigned div = 1;

	/*
	 * Divide by 1,2,4,8,16... 
	 */
	while ((frequency / div) > REF_FREQ)
		div += div;

	if (div > 128)
		div = 128;

	switch (div) {
	case 1:
		divisor = APIC_DIV_1;
		break;
	case 2:
		divisor = APIC_DIV_2;
		break;
	case 4:
		divisor = APIC_DIV_4;
		break;
	case 8:
		divisor = APIC_DIV_8;
		break;
	case 16:
		divisor = APIC_DIV_16;
		break;
	case 32:
		divisor = APIC_DIV_32;
		break;
	case 64:
		divisor = APIC_DIV_64;
		break;
	case 128:
		divisor = APIC_DIV_128;
		break;
	}

	frequency /= div;
}

/*
 * Handle 1 device system wide
 */
static int lapic_init(unsigned unitno)
{
	struct cpuid_data data;

	if (unitno) {
		return (ENXIO);
	}

	if (!is_apic_supported()) {
		return (ENXIO);
	}

	/*
	 * Init the APIC with proper interrupt vector,
	 * CHECK INT MAPPINGS Intel has differences
	 * with 8253 hardware exceptions etc. etc.
	 */
	apic_configure(mode, 0, LAPIC_IRQ, APIC_DIV_1);

	execute_cpuid(&data, 0);
	if (data.word[EAX] > 0x14) {
		execute_cpuid(&data, 0x15);
		if (data.word[EBX])
			frequency = data.word[ECX];
	}
	if (!frequency) {
		execute_cpuid(&data, 0);
		if (data.word[EAX] > 0x15) {
			execute_cpuid(&data, 0x16);
			frequency = (data.word[ECX] & 0xFFFFUL) * 1000000;
		}
	}

	if (!frequency) {
		calibrate_lapic();
	}

	calibrate_divisor();

	if (EOK != add_int_cb(lapic_int_handler, 16)) {
		return (EPERM);
	}

	return (EOK);
}

static int lapic_start(unsigned unitno)
{
	if (unitno) {
		return (EINVAL);
	}

	if (flags & DRV_STATUS_RUN) {
		return (EOK);
	}

	flags &= ~DRV_STATUS_STOP;
	flags |= DRV_STATUS_RUN;

	apic_configure(mode, 1, LAPIC_IRQ, divisor);
	apic_write_counter(ctrvalue);

	return (EOK);
}

static int lapic_stop(unsigned unitno)
{
	if (unitno) {
		return (EINVAL);
	}

	if (flags & DRV_STATUS_STOP) {
		return (EOK);
	}

	flags &= ~DRV_STATUS_RUN;
	flags |= DRV_STATUS_STOP;

	apic_write_counter(0);

	return (EOK);
}

static int lapic_done(unsigned unitno)
{
	if (unitno) {
		return (EINVAL);
	}

	apic_configure(mode, 0, LAPIC_IRQ, divisor);
	apic_write_counter(0);

	return (del_int_cb(16));
}

static int lapic_ioctrl(void *data, unsigned opcode, unsigned unitno)
{
	unsigned *udata = (unsigned *)data;

	/* All opcodes require data */
	if (!data) {
		return (EINVAL);
	}

	switch (opcode) {
	case CLK_SET_PERIOD:
		if (!udata[0])
			udata[0]++;
		if ((udata[0] < TIMER_MIN_VAL) || (udata[0] > TIMER_MAX_VAL))
			return (EINVAL);
		ctrvalue = udata[0];
		apic_write_counter(ctrvalue);
		return (EOK);

	case CLK_GET_PARAMS:
		*udata++ = frequency;
		*udata++ = TIMER_MIN_VAL;
		*udata++ = TIMER_MAX_VAL;
		return (EOK);

	case CLK_GET_ELAPSED:
		*udata = apic_read_counter();
		return (EOK);

	case CLK_SET_MODE:
		if (udata[0] > 1) {
			return (EINVAL);
		}

		mode = (uint8_t) udata[0];
		apic_configure(mode, 1, LAPIC_IRQ, divisor);
		return (EOK);

	case CLK_SET_CB:
		cbfn = data;
		return (EOK);

	default:
		return (EINVAL);
	}
}

static unsigned lapic_status(unsigned unitno)
{
	if (!unitno) {
		return (flags);
	}

	return (0);
}

char_driver_t lapic_drv = {
	.cmn = {
		.name = "clk",
		.init_fn = lapic_init,
		.start_fn = lapic_start,
		.stop_fn = lapic_stop,
		.done_fn = lapic_done,
		.ioctrl_fn = lapic_ioctrl,
		.status_fn = lapic_status,
		.poll_fn = NULL}
	,
	.write_fn = NULL,
	.read_fn = NULL,
	.write_multi_fn = NULL,
	.read_multi_fn = NULL
};
