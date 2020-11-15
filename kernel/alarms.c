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
#include <diegos/alarms.h>
#include <diegos/events.h>
#include <diegos/kernel_events.h>
#include <diegos/kernel_ticks.h>
#include <diegos/interrupts.h>
#include <list.h>

#include "scheduler.h"
#include "alarms_private.h"
#include "clock.h"
#include "kprintf.h"

enum {
    /* Alarm repeat continuosly */
    ALM_RECURSIVE = (1 << 0),
    /* Alarm is counting... */
    ALM_TRIGGERED = (1 << 1),
    /* Alarm has expired */
    ALM_EXPIRED   = (1 << 2)
};

struct alarm {
    list_node header;
    uint64_t expiration;
    uint32_t flags;
    ev_queue_t *notify;
    event_t event;
    char name[16];
    unsigned msecs;
};

static list_inst alarms_list;

static void alarm_cb(uint64_t msecs)
{
    struct alarm *cursor;

    if (!list_count(&alarms_list)) {
        return;
    }

    cursor = list_head(&alarms_list);

    while (cursor) {
        if (ALM_TRIGGERED == (cursor->flags & (ALM_TRIGGERED | ALM_EXPIRED))) {
            if (msecs >= cursor->expiration) {
                if (EOK != event_put(cursor->notify, &cursor->event, NULL)) {
                    kerrprintf("unable to queue alarm %s\n", cursor->name);
                } else {
                    cursor->flags |= ALM_EXPIRED;
                    /* One shot */
                    if (!(cursor->flags & ALM_RECURSIVE)) {
                        cursor->flags &= ~ALM_TRIGGERED;
                    }
                }
            }
        }

        cursor = (alarm_t *)cursor->header.next;
    }
}

BOOL init_alarms_lib()
{
    if (EOK != list_init(&alarms_list) ||
            !clock_add_cb(alarm_cb)) {
        kerrprintf("failed initing alarm lib.\n");
        return (FALSE);
    }

    return (TRUE);
}

alarm_t *alarm_create (const char *name,
                       uint16_t alarmid,
                       unsigned millisecs,
                       BOOL recursive,
                       ev_queue_t *evqueue)
{
    struct alarm *ptr;

    if (!millisecs || !evqueue) {
        return (NULL);
    }

    ptr = malloc(sizeof(struct alarm));

    if (!ptr) {
        return (NULL);
    }

    lock();

    if (EOK != list_add(&alarms_list, NULL, &ptr->header)) {
        unlock();
        free(ptr);
        return (NULL);
    }

    if (name) {
        snprintf(ptr->name, sizeof(ptr->name),"%s",name);
    } else {
        snprintf(ptr->name, sizeof(ptr->name),"Alarm%x",(intptr_t)ptr);
    }

    ptr->flags = 0;

    if (recursive) {
        ptr->flags |= ALM_RECURSIVE;
    }

    ptr->expiration = 0;
    ptr->msecs = millisecs;
    ptr->notify = evqueue;
    ptr->event.classid = CLASS_ALARM;
    ptr->event.eventid = alarmid;

    unlock();

    return (ptr);
}

void alarm_set (alarm_t *alm, BOOL start)
{
    if (!alm) {
        errno = EINVAL;
        return;
    }

    lock();

    if (start) {
        alm->flags |= ALM_TRIGGERED;
        alm->flags &= ~ALM_EXPIRED;
        alm->expiration = alm->msecs + clock_get_milliseconds();
    } else {
        alm->flags &= ~ALM_TRIGGERED;
        alm->flags |= ALM_EXPIRED;
        alm->expiration = 0;
    }

    unlock();
}

STATUS alarm_acknowledge (alarm_t *alm)
{
    const uint32_t MATCH = (ALM_RECURSIVE | ALM_TRIGGERED | ALM_EXPIRED);

    if (!alm) {
        return (EINVAL);
    }

    lock();

    if (MATCH == (alm->flags & MATCH)) {
        alm->expiration += alm->msecs;
        alm->flags &= ~ALM_EXPIRED;
    }

    unlock();

    return (EOK);
}

STATUS alarm_update (alarm_t *alm, unsigned millisecs, BOOL recursive)
{
    if (!alm || !millisecs) {
        return (EINVAL);
    }

    lock();

    if (millisecs != alm->msecs) {
        alm->msecs = millisecs;
    }

    if (recursive && !(alm->flags & ALM_RECURSIVE)) {
        alm->flags |= ALM_RECURSIVE;
    } else if (!recursive && (alm->flags & ALM_RECURSIVE)) {
        alm->flags &= ~ALM_RECURSIVE;
    }

    unlock();

    return (EOK);
}

STATUS alarm_done (alarm_t *alm)
{
    if (!alm) {
        return (EINVAL);
    }

    lock();

    if (EOK != list_remove(&alarms_list, &alm->header)) {
        unlock();
        return (EGENERIC);
    }

    unlock();

    free(alm);

    return (EOK);
}

static void dump_internal (const alarm_t *alm)
{
    char tempbuf[60];

    sprintf(tempbuf, "(%#x) ", alm->flags);
    if (alm->flags & ALM_EXPIRED) {
        strcat(tempbuf, "EXPIRED ");
    }
    if (alm->flags & ALM_RECURSIVE) {
        strcat(tempbuf, "RECURSIVE ");
    }
    if (alm->flags & ALM_TRIGGERED) {
        strcat(tempbuf, "TRIGGERED ");
    }
    if (!alm->flags) {
        strcat(tempbuf, "DISABLED");
    }
    fprintf(stderr,
            "%-15s | %6u | %s\n",
            alm->name,
            alm->msecs,
            tempbuf);
}

void alarm_dump (const alarm_t *alm)
{
    if (!alm) {
        fprintf(stderr,"\n--- ALARMS TABLE -----------------------\n\n");
    }
    fprintf(stderr,"%-15s   %6s   %s\n",
            "ALARM NAME",
            "PERIOD",
            "FLAGS");
    fprintf(stderr,"________________________________________\n");
    if (alm) {
        dump_internal(alm);
    } else {
        alm = list_head(&alarms_list);
        while (alm) {
            dump_internal(alm);
            alm = (alarm_t *)alm->header.next;
        }
    }
    fprintf(stderr,"----------------------------------------\n\n");
}
