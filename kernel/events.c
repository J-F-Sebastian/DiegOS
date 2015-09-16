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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <list.h>
#include <queue.h>
#include <diegos/events.h>
#include <diegos/interrupts.h>

#include "scheduler.h"
#include "events_private.h"
#include "platform_include.h"
#include "kprintf.h"

typedef struct ev_queue {
    list_node header;
    queue_inst msgqueue;
    char name[16];
    uint8_t  threadid;
} ev_queue_t;

static list_inst events_list;

ev_queue_t *event_init_queue(const char *name)
{
    ev_queue_t *ptr = malloc(sizeof(ev_queue_t));

    if (!ptr) {
        return (NULL);
    }

    if ((EOK != queue_init(&ptr->msgqueue)) ||
        (EOK != list_add(&events_list, NULL, &ptr->header))) {
        free(ptr);
        return (NULL);
    }

    if (name) {
        snprintf(ptr->name, sizeof(ptr->name),"%s",name);
    } else {
        snprintf(ptr->name, sizeof(ptr->name),"Evtqueue%x",(intptr_t)ptr);
    }

    ptr->threadid = THREAD_TID_INVALID;

    return (ptr);
}

int event_done_queue(ev_queue_t *evqueue)
{
    queue_node *ptr;

    if (!evqueue) {
        return (EINVAL);
    }

    if (queue_count(&evqueue->msgqueue)) {
        kerrprintf("there are %d events left in queue %s\n",
                   queue_count(&evqueue->msgqueue),
                   evqueue->name);
        while (EOK == queue_dequeue(&evqueue->msgqueue,&ptr)) {}
    }

    if (EOK != list_remove(&events_list, &evqueue->header)) {
        return EGENERIC;
    }

    free(evqueue);

    return EOK;
}

unsigned event_queue_size(ev_queue_t *evqueue)
{
    if (!evqueue) {
        return (EINVAL);
    }

    return (queue_count(&evqueue->msgqueue));
}

int event_watch_queue(ev_queue_t *evqueue)
{
    if (!evqueue) {
        return (EINVAL);
    }

    if (THREAD_TID_INVALID != evqueue->threadid) {
        kerrprintf("TID %d is already watching %s for events.\n",
                   evqueue->threadid, evqueue->name);
        return (EINVAL);
    }

    evqueue->threadid = scheduler_running_tid();

    return (EOK);
}

int event_put(ev_queue_t *evqueue, event_t *ev)
{
    STATUS retcode;

    if (!evqueue || !ev) {
        return (EINVAL);
    }

    retcode = queue_enqueue(&evqueue->msgqueue, &ev->header);

    if (EOK != retcode) {
        kerrprintf("failed queueing event to %s\n",evqueue->name);
        retcode = EPERM;
    }

    return (retcode);
}

event_t *event_get(ev_queue_t *evqueue)
{
    event_t *retval = NULL;
    STATUS retcode;

    if (!evqueue) {
        return (NULL);
    }

    retcode = queue_dequeue(&evqueue->msgqueue, (queue_node **)&retval);

    if (EOK != retcode) {
        kerrprintf("failed dequeueing event from %s\n",evqueue->name);
    }

    return (retval);
}

void wait_for_events(ev_queue_t *evqueue)
{
    thread_t *prev, *next;

    if (!evqueue) {
        errno = EINVAL;
        return;
    }

    prev = scheduler_running_thread();

    if (evqueue->threadid != prev->tid) {
        kerrprintf("TID %d is already watching %s for events.\n",
                   evqueue->threadid, evqueue->name);
        return;
    }

    if (queue_count(&evqueue->msgqueue)) {
        kerrprintf("events queue %s has events!\n",evqueue->name);
    }

    if (!scheduler_wait_thread(THREAD_FLAG_WAIT_EVENT)) {
        kerrprintf("Cannot wait for events TID %u\n", prev);
        return;
    }

    schedule_thread();

    next = scheduler_running_thread();

    switch_context(prev->context, next->context);
}

BOOL init_events_lib()
{
    if (EOK != list_init(&events_list)) {
        return (FALSE);
    }

    return (TRUE);
}

void resume_on_events()
{
    ev_queue_t *cur = (ev_queue_t *)list_head(&events_list);

    while (cur) {
        if (queue_count(&cur->msgqueue)) {
            scheduler_resume_thread(THREAD_FLAG_WAIT_EVENT, cur->threadid);
        }
        cur = (ev_queue_t *)cur->header.next;
    }
}

static void dump_internal (ev_queue_t *evqueue)
{
    fprintf(stderr,"%-15s   %12u   %u\n",
            evqueue->name,
            evqueue->threadid,
            queue_count(&evqueue->msgqueue));
}

void event_dump (ev_queue_t *evqueue)
{
    if (!evqueue) {
        fprintf(stderr,"\n--- EVENTS TABLE -----------------------\n\n");
    }
    fprintf(stderr,"%-15s   %12s   %s\n",
            "EVT QUEUE NAME",
            "LISTENER TID",
            "EVENTS IN QUEUE");
    fprintf(stderr,"________________________________________\n");
    if (evqueue) {
        dump_internal(evqueue);
    } else {
        evqueue = list_head(&events_list);
        while (evqueue) {
            dump_internal(evqueue);
            evqueue = (ev_queue_t *)evqueue->header.next;
        }
    }
    fprintf(stderr,"----------------------------------------\n\n");
}
