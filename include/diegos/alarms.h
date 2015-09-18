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

#ifndef _ALARM_H_
#define _ALARM_H_

#include <diegos/events.h>
#include <diegos/kernel_events.h>

typedef struct alarm alarm_t;

/*
 * Alarms API.
 * This API can be used in interrupt context.
 * Alarms are time-triggered events sent to events' queues.
 * An alarm will send an event after *at least* the expiration time has elapsed;
 * the wake-up extra time is unbound.
 */

/*
 * Create an alarm providing a name (optional), an ID, the expire period in
 * milliseconds, the recursiveness and the events' queue to handle expiration.
 * Once the alarm is set, it will send an event to the specified queue; if the
 * alarm is recursive, it will automatically re-arm.
 *
 * PARAMETERS IN
 * const char *name - alarm name, can be NULL
 * uint16_t alarmid - the alarm ID reported in the event sent to evqueue
 * unsigned millisecs - once the alarm is set, it will arm and send an event
 *                      after millisecs are elapsed
 * BOOL recursive - if recursive is TRUE, the expired alarm will send an event
 *                  and re-arm automatically.
 *                  if recursive is FALSE, the expired alarm will be inactive.
 * ev_queue_t *evqueue - queue servicing alarms expiration events.
 *
 * RETURNS
 * A pointer to an alarm handle or NULL in case of failure.
 */
alarm_t *alarm_create (const char *name,
                       uint16_t alarmid,
                       unsigned millisecs,
                       BOOL recursive,
                       ev_queue_t *evqueue);

/*
 * Set an alarm, i.e. enable an alarm and start counting until expiration.
 *
 * PARAMETERS IN
 * alarm_t *alm - the alarm to be set
 * BOOL start - if start is TRUE the alarm will be set and counting; if start
 *               is false the alarm will be inactive.
 */
void alarm_set (alarm_t *alm, BOOL start);

/*
 * Acknowledge a recursive alarm. A recursive alarm won't send more events
 * unless this function is invoked. If an interval is crossed (i.e. the function
 * is invoked too late) the alarm won't generate events until the next interval
 * is reached.
 * Calling this function for a non_recursive alarm is a no-op.
 *
 * Time      ------------------------------------------------------------------>
 * Alarm
 * intervals +----------+----------+----------+----------+----------+---------->
 *
 * Function
 * calls     ACK        ACK             ACK   ACK                   ACK
 *           ^          ^          ^    ^     ^          ^          ^
 *           OK         OK         No event   OK         No event   OK
 *
 *
 * PARAMETERS IN
 * alarm_t *alm - The alarm handle
 *
 * RETURNS
 * EINVAL if alm is not a valid pointer
 * EOK in any other case.
 */
int alarm_acknowledge (alarm_t *alm);

/*
 * Update the period duration and recursiveness of an alarm.
 *
 * PARAMETERS IN
 * alarm_t *alm - The alarm handle
 * unsigned millisecs - the new timer period
 * BOOL recursive - the recursiveness
 *
 * RETURNS
 * EINVAL if alm is not a valid pointer or millisecs is 0.
 * EOK in any other case.
 */
int alarm_update (alarm_t *alm, unsigned millisecs, BOOL recursive);

/*
 * Remove an alarm.
 *
 * PARAMETERS IN
 * alarm_t *alm - The alarm handle
 *
 * RETURNS
 * EINVAL if alm is not a valid pointer
 * EGENERIC if removal fails
 * EOK in any other case.
 */
int alarm_done (alarm_t *alm);

/*
 * Print to stderr the specified alarms's configuration and status.
 * If alm is null, all alarms are processed.
 *
 * PARAMETERS IN
 * alarm_t *alm - a specific alarm to be dumped, or NULL to dump'em all
 */
void alarm_dump (const alarm_t *alm);

#endif

