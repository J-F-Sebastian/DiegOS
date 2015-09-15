#ifndef _POLL_PRIVATE_H_
#define _POLL_PRIVATE_H_

#include "io_waits_private.h"

BOOL init_poll_lib (void);

int poll_wakeup (struct wait_queue_item *wqi);

#endif
