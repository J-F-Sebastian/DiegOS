#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <list.h>
#include <diegos/barriers.h>
#include <diegos/interrupts.h>
#include <bitmaps.h>
#include <string.h>

#include "barriers_private.h"
#include "threads_data.h"
#include "scheduler.h"
#include "platform_include.h"
#include "kprintf.h"

enum {
    /* Barrier state, open or closed */
    BARRIER_OPEN        = (1 << 0),
    /* Open barriers will be automatially closed in resume_on_barriers */
    BARRIER_AUTOCLOSE   = (1 << 1)
};

struct barrier {
    list_node header;
    long thread_ids[BITMAPLEN(DIEGOS_MAX_THREADS)];
    char name[16];
    unsigned flags;
};

static list_inst barriers_list;

barrier_t *barrier_create(const char *name, BOOL autoclose)
{
    struct barrier *ptr = malloc(sizeof(struct barrier));

    if (!ptr) {
        return (NULL);
    }

    lock();

    if (EOK != list_add(&barriers_list, NULL, &ptr->header)) {
        unlock();
        free(ptr);
        return (NULL);
    }

    memset(ptr->thread_ids, 0, sizeof(ptr->thread_ids));

    if (name) {
        snprintf(ptr->name, sizeof(ptr->name),"%s",name);
    } else {
        snprintf(ptr->name, sizeof(ptr->name),"Barrier%x",(intptr_t)ptr);
    }

    if (autoclose) {
        ptr->flags = BARRIER_AUTOCLOSE;
    } else {
        ptr->flags = 0;
    }

    unlock();

    return (ptr);
}

int barrier_open (barrier_t *barrier)
{
    lock();

    if (barrier) {
        barrier->flags |= BARRIER_OPEN;
    }

    unlock();

    return ((barrier) ? EOK : EINVAL);
}

int barrier_close (barrier_t *barrier)
{
    lock();

    if (barrier) {
        barrier->flags &= ~BARRIER_OPEN;
    }

    unlock();

    return ((barrier) ? EOK : EINVAL);
}

int wait_for_barrier (barrier_t *barrier)
{
    thread_t *prev, *next;

    if (!barrier) {
        kerrprintf("Invalid barrier");
        return EINVAL;
    }

    lock();

    if (barrier->flags & BARRIER_OPEN) {
        unlock();
        return EOK;
    }

    bitmap_set(barrier->thread_ids, scheduler_running_tid());

    unlock();

    prev = scheduler_running_thread();
    if (!scheduler_wait_thread(THREAD_FLAG_WAIT_BARRIER)) {
        return EPERM;
    }
    schedule_thread();

    next = scheduler_running_thread();
    switch_context(prev->context, next->context);

    return EOK;
}

BOOL init_barriers_lib ()
{
    if (EOK != list_init(&barriers_list)) {
        return (FALSE);
    }

    return (TRUE);
}

static void resumecb (long *bitmap, unsigned pos, void *param)
{
    scheduler_resume_thread(THREAD_FLAG_WAIT_BARRIER, (uint8_t)pos);
    bitmap_clear(bitmap, pos);
}

void resume_on_barriers ()
{
    struct barrier *cur = (struct barrier *)list_head(&barriers_list);

    while (cur) {
        if (cur->flags & BARRIER_OPEN) {
            bitmap_for_each_set(cur->thread_ids,
                                BITMAPLEN(DIEGOS_MAX_THREADS),
                                resumecb,
                                NULL);
            if (cur->flags & BARRIER_AUTOCLOSE) {
                cur->flags &= ~BARRIER_OPEN;
            }
        }
        cur = (barrier_t *)cur->header.next;
    }
}

static void dump_internal (const barrier_t *barrier)
{
    fprintf(stderr,
            "%-15s | %5s | %4s | %s\n",
            barrier->name,
            (barrier->flags & BARRIER_OPEN) ? "OPEN" : "CLOSED",
            (barrier->flags & BARRIER_AUTOCLOSE) ? "YES" : "NO",
            "N/A");
}

void barrier_dump (const barrier_t *barrier)
{
    if (!barrier) {
        fprintf(stderr,"\n--- BARRIERS TABLE ----------------------\n\n");
    }
    fprintf(stderr,"%-15s   %5s   %4s   %s\n",
                       "BARRIER NAME",
                       "STATE",
                       "AUTO",
                       "WAITING THREADS");
        fprintf(stderr,"________________________________________\n");
    if (barrier) {
        dump_internal(barrier);
    } else {
        barrier = list_head(&barriers_list);
        while (barrier) {
            dump_internal(barrier);
            barrier = (barrier_t *)barrier->header.next;
        }
    }
    fprintf(stderr,"----------------------------------------\n\n");
}

