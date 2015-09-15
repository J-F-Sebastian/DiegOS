#include <stdio.h>
#include <diegos/kernel_dump.h>

#include "threads.h"
#include "mutex_private.h"

void threads_dump()
{
    uint32_t i;
    thread_t *ptr;

    fprintf(stderr,"\n--- THREADS TABLE ----------------------------\n\n");
    fprintf(stderr,"%-15s   %3s   %6s   %s\n","THREAD NAME","TID","STACK","STATE");
    fprintf(stderr,"______________________________________________\n");
    for (i = 0; i < DIEGOS_MAX_THREADS; i++) {
        ptr = get_thread(i);
        if (ptr) {
            fprintf(stderr,
                    "%-15s | %3d | %6d | %s\n",
                    ptr->name,
                    ptr->tid,
                    ptr->stack_size,
                    state2str(ptr->state));
        }
    }
    fprintf(stderr,"----------------------------------------------\n\n");
}

void mutexes_dump()
{
    dump_mutex(NULL);
}

void threads_check()
{
    check_thread_stack();
}
