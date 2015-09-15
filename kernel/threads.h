#ifndef THREADS_H_INCLUDED
#define THREADS_H_INCLUDED

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

BOOL init_thread_lib (void);

uint8_t init_thread (const char *name,
                     uint8_t prio,
                     void (*entry_ptr)(void),
                     void *stack,
                     uint32_t stack_size);

BOOL done_thread (uint8_t tid);

thread_t *get_thread (uint8_t tid);

const char *state2str (uint8_t state);

void check_thread_stack (void);

#endif // THREADS_H_INCLUDED
