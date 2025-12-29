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

#include <errno.h>
#include <libs/kernel_time.h>

int kernel_time_init(uint32_t base_val, uint32_t min_val, uint32_t max_val, struct time_util *tu)
{
	if (!base_val || !min_val || !max_val || !tu)
		return EINVAL;

	tu->base_counter = base_val;
	tu->min_counter = min_val;
	tu->max_counter = max_val;
	tu->cntr_ticks_per_msecs_int = base_val / 1000UL;
	tu->cntr_ticks_per_msecs_fract = base_val % 1000UL;
	tu->elapsed_counter = 0;

	return (EOK);
}

uint32_t kernel_time_get_msecs(uint32_t cntrticks, const struct time_util *tu)
{
	uint32_t retval = 0;
	uint32_t fract = 0;

	cntrticks = kernel_time_adjust_range(cntrticks, tu);
	retval = cntrticks / tu->cntr_ticks_per_msecs_int;
	fract = cntrticks % tu->cntr_ticks_per_msecs_int;
	/* rounding */
	if (fract >= tu->cntr_ticks_per_msecs_int / 2)
		retval++;
	return (retval);
}

uint64_t kernel_time_get_elapsed_msecs(const struct time_util *tu)
{
	uint64_t retval = 0ULL;

	if (tu) {
		/*
		 * Split computation: compute integer seconds
		 */
		retval = tu->elapsed_counter / (uint64_t) tu->base_counter;
		/*
		 * Transform into milliseconds
		 */
		retval *= 1000ULL;
		/*
		 * Add remainder in milliseconds
		 */
		retval +=
		    1000ULL * (tu->elapsed_counter % (uint64_t) tu->base_counter) /
		    (uint64_t) tu->base_counter;
	}

	return (retval);
}

uint32_t kernel_time_get_value(uint32_t msecs, const struct time_util *tu)
{
	if (!msecs || !tu)
		return 0;

	uint32_t retval = msecs * tu->cntr_ticks_per_msecs_int;
	uint32_t fract = msecs * tu->cntr_ticks_per_msecs_fract;
	/* rounding fract */
	fract += 500UL;
	fract /= 1000UL;
	retval += fract;

	return (kernel_time_adjust_range(retval, tu));
}

int kernel_time_get_minmax_msecs(uint32_t msecs[], const struct time_util *tu)
{
	if (!msecs || !tu)
		return EINVAL;

	msecs[0] = 1000UL * tu->min_counter / tu->base_counter;
	msecs[1] = 1000UL * tu->max_counter / tu->base_counter;

	return (EOK);
}

uint32_t kernel_time_adjust_msecs(uint32_t msecs, const struct time_util *tu)
{
	uint32_t range[2];

	if (EOK == kernel_time_get_minmax_msecs(range, tu)) {
		if (msecs < range[0])
			msecs = range[0];
		else if (msecs > range[1])
			msecs = range[1];
	}

	return msecs;
}
