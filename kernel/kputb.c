#include <diegos/devices.h>
#include <diegos/interrupts.h>

static device_t *ttydev = NULL;

/*
 * kputb won't take semaphores, mutexes, or any other system call.
 * output will go through anyway for debugging purposes.
 */
void kputb(char *buffer, unsigned bytes)
{
    if (!ttydev) {
        ttydev = device_lookup_name(DEFAULT_DBG_TTY);
        if (!ttydev) {
            return;
        }
    }

    ttydev->drv->write_fn(buffer, bytes, 0);
}

