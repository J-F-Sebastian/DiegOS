#include <diegos/kernel.h>

#include "fail_safe.h"
#include "scheduler.h"
#include "kprintf.h"

void scheduler_fail_safe(void)
{
    thread_t *me = scheduler_running_thread();

    kprintf("EXITING %s WITHOUT CALLING THREAD_TERMINATE !!!\n",me->name);
    thread_terminate();
}

