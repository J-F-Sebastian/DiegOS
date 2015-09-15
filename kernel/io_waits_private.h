#ifndef IO_WAITS_PRIVATE_H_INCLUDED
#define IO_WAITS_PRIVATE_H_INCLUDED

#include <diegos/io_waits.h>

enum {
    IO_WAIT_DEFAULT,
    IO_WAIT_EXCLUSIVE,
    IO_WAIT_POLL
};

struct wait_queue_item {
    list_node header;
    unsigned char flags;
    union {
        uint8_t tid;
        void *pt;
    };
};

/*
 * Initialize the io_waits library.
 * Must be called internally by kernel init routine.
 *
 * RETURN VALUES
 * TRUE if initialization succeded.
 * FALSE in any other case
 *
 */
BOOL init_io_waits_lib (void);

struct wait_queue_item *io_wait_get_item (void);

void io_wait_put_item (struct wait_queue_item *item);

int io_wait_add (struct wait_queue_item *item, wait_queue_t *wq);

int io_wait_remove (struct wait_queue_item *item, wait_queue_t *wq);

#endif // IO_WAITS_PRIVATE_H_INCLUDED
