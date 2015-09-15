#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

/*******************************************
 *********** HEADER FILE *******************
 *******************************************
 *
 * Name:
 * Author:
 * Date:
 *******************************************
 * Description:
 *
 *******************************************/

#include "threads_data.h"

BOOL scheduler_init(void);

void schedule_thread(void);

void update_schedule(void);

BOOL scheduler_suspend_thread(void);

BOOL scheduler_resume_thread(uint8_t flags, uint8_t tid);

BOOL scheduler_delay_thread(uint64_t ticks);

BOOL scheduler_wait_thread(uint8_t flags);

BOOL scheduler_add_thread(uint8_t tid);

BOOL scheduler_remove_thread(uint8_t tid);

uint8_t scheduler_running_tid(void);

thread_t *scheduler_running_thread(void);

uint8_t scheduler_ready_threads(uint8_t prio);

void scheduler_fail_safe(void);

#endif // SCHEDULER_H_INCLUDED

