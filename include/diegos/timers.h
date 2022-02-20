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

#ifndef _TIMERS_H_
#define _TIMERS_H_

typedef struct timer timer_t;
typedef void (*tmr_cb) (void *);

/*
 * Timers API.
 * This API can be used in interrupt context.
 * Timers are time-triggered callbacks executed in the context of an internal
 * thread.
 * Callbacks are passed a specific context by parameter.
 * Caution is required when coding the callbacks - they run in another thread context,
 * they cannot suspend or resume, they MUST NOT perform intensive tasks.
 */

/*
 * Create a timer providing a name (optional), the expire period in
 * milliseconds, the recursiveness, the callback to invoke on expiration and the
 * context of the callack (parameter passed to the callback).
 * Once the timer is set, it will invoke the callback on expiration; if the
 * timer is recursive, it will automatically re-arm.
 *
 * PARAMETERS IN
 * const char *name   - timer name, can be NULL
 * unsigned millisecs - timer expiration
 * BOOL recursive     - if recursive is TRUE, the expired timer will re-arm automatically.
 *                      if recursive is FALSE, the expired timer will be inactive.
 * tmr_cb cb          - callback invoked on expiration
 * void *arg          - context to be passed to the callback
 *
 * RETURNS
 * A pointer to a timer handle or NULL in case of failure.
 */
timer_t *timer_create(const char *name, unsigned millisecs, BOOL recursive, tmr_cb cb, void *arg);

/*
 * Set a timer state.
 *
 * PARAMETERS IN
 * timer_t *tmr - the timer to be set
 * BOOL start   - if start is TRUE the timer will be set and counting; if start
 *                is false the timer will be stopped and set to inactive.
 */
void timer_set(timer_t * tmr, BOOL start);

/*
 * Update the period duration and recursiveness of a timer.
 *
 * PARAMETERS IN
 * timer_t *tmr       - The timer handle
 * unsigned millisecs - the new timer period
 * BOOL recursive     - the recursiveness
 *
 * RETURNS
 * EINVAL if tmr is not a valid pointer or millisecs is 0.
 * EOK in any other case.
 */
int alarm_update(timer_t * tmr, unsigned millisecs, BOOL recursive);

/*
 * Remove a timer.
 *
 * PARAMETERS IN
 * timer_t *tmr - The timer handle
 *
 * RETURNS
 * EINVAL if tmr is not a valid pointer
 * EGENERIC if removal fails
 * EOK in any other case.
 */
int timer_done(timer_t * tmr);

/*
 * Print to stderr the specified timer configuration and status.
 * If tmr is null, all timers are processed.
 *
 * PARAMETERS IN
 * timer_t *tmr - a specific timer to be dumped, or NULL to dump'em all
 */
void timers_dump(const timer_t * tmr);

#endif
