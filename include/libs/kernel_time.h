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

#ifndef _KERNEL_TIME_H_
#define _KERNEL_TIME_H_

#include <stdint.h>
#include <errno.h>

/*
 * In this module the counters are the values to be programmed in clock registers.
 * Clock devices usually count down (or up) at a certain speed
 * (the frequency of the clock) and trigger an interrupt as soon as the
 * count reaches 0 (or the maximum supported value).
 * The counted value is added to elapsed_ticks and keep track of
 * the time passing by.
 * The resolution of the clock (its base frequency) is open and does not need
 * to be configured in time_util.
 * Using 64 bit values as accumulator we do not care about the future: a clock measuring
 * nanosenconds (i.e. adding 1.000.000.000 to elapsed_counter) would overflow in 508 years...
 */
struct time_util {
	/*
	 * Value to count in a second,
	 * i.e. the frequency of the clock - or the counting number taken by the
	 * clock to measure 1 second.
	 */
	uint32_t base_counter;
	/* Minimum counter value supported */
	uint32_t min_counter;
	/* Maximum counter value supported */
	uint32_t max_counter;
	/* Counter value per milliseconds integer part */
	uint32_t cntr_ticks_per_msecs_int;
	/* Counter value per milliseconds fractional part */
	uint32_t cntr_ticks_per_msecs_fract;
	/* Accumulator for time base computing */
	uint64_t elapsed_counter;
};

/*
 * Compute values to be set in the passed-in struct time_util.
 *
 * PARAMETERS IN
 * uint32_t base_val    - the clock ticks (counter value) per second
 * uint32_t min_val     - the minimum programmable value for the clock counter
 * uint32_t max_val     - the maximum programmable value for the clock counter
 * struct time_util *tu - a pointer to a structure to be filled with values
 *
 * RETURNS
 * EOK in success
 * EINVAL if base_ticks is 0 or tu is 0
 */
int kernel_time_init(uint32_t base_val, uint32_t min_val, uint32_t max_val, struct time_util *tu);

/*
 * Converts the counter value to milliseconds.
 * cntrticks is adjusted to the range stored in tu.
 *
 * PARAMETERS IN
 * uint32_t   cntrticks   - the value to be converted, adjusted to range
 * struct time_util *tu   - a pointer to a struct time_util
 *
 * RETURNS
 * The convertion in milliseconds
 */
uint32_t kernel_time_get_msecs(uint32_t cntrticks, const struct time_util *tu);

/*
 * Convert the elapsed counter value to milliseconds.
 * This function performs the same operation as kernel_time_get_msecs()
 * but on 64 bit quantities ans using elapsed_counter as a counter value.
 *
 * PARAMETERS IN
 * struct time_util *tu	- a pointer to a struct time_util
 *
 * RETURNS
 * The value of elapsed_counter in milliseconds
 */
uint64_t kernel_time_get_elapsed_msecs(const struct time_util *tu);

/*
 * Converts milliseconds to counter value.
 * Return value is adjusted according to the range stored in tu.
 *
 * PARAMETERS IN
 * uint32_t msecs	- the milliseconds to be converted
 * struct time_util *tu - a pointer to a struct time_util
 *
 * RETURNS
 * The converted counter value, adjusted to range
 */
uint32_t kernel_time_get_value(uint32_t msecs, const struct time_util *tu);

/*
 * Converts the minimum and the maximum counter values to milliseconds and store
 * the result in msecs[].
 *
 * PARAMETERS IN
 * struct time_util *tu	- a pointer to a struct time_util
 *
 * PARAMETERS OUT
 * uint32_t msecs[]	- an array of size 2 storing the converted values
 *
 * RETURNS
 * EOK if the convertion is successful
 * EINVAL if the parameters are not valid
 */
int kernel_time_get_minmax_msecs(uint32_t msecs[], const struct time_util *tu);

inline int kernel_time_validate_range(uint32_t value, const struct time_util *tu)
{
	if (tu) {
		return ((value < tu->min_counter) || (value > tu->max_counter)) ? EINVAL : EOK;
	}

	return EINVAL;
}

inline uint32_t kernel_time_adjust_range(uint32_t value, const struct time_util * tu)
{
	if (tu) {
		if (value < tu->min_counter)
			return tu->min_counter;
		if (value > tu->max_counter)
			return tu->max_counter;
	}

	return value;
}

inline void kernel_time_update_elapsed_counter(uint32_t value, struct time_util *tu)
{
	if (tu) {
		tu->elapsed_counter += value;
	}
}

inline uint64_t kernel_time_get_elapsed_counter(const struct time_util *tu)
{
	return (tu) ? (tu->elapsed_counter) : (0ULL);
}

#endif
