#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <chunks.h>
#include <queue.h>
#include <diegos/interrupts.h>
#include <diegos/drivers.h>
#include <diegos/devices.h>
#include <diegos/net_interfaces.h>
#include <diegos/io_waits.h>

#include "kprintf.h"
#include "threads.h"
#include "drivers_private.h"
#include "devices_private.h"

int driver_def_ok(unsigned unitno)
{
    return (EOK);
}

int driver_def_error(unsigned unitno)
{
    return (EGENERIC);
}

int kdrvprintf(const char *fmt, ...)
{
    int n;
    va_list ap;

    va_start(ap, fmt);
    kvprintf("[DRV]: ", ap);
    n = kvprintf(fmt, ap);
    va_end(ap);

    return (n);
}

/* Go down all the list even on error, at least some driver will be ok */
int char_drivers_list_init(char_driver_t *drvlist[], unsigned drvlistsize)
{
    unsigned d, u;
    int retcode;
    int retval = EOK;
    device_t *retptr;

    for (d = 0; d < drvlistsize; d++) {
        for (u = 0; u < DRV_UNIT_MAX; u++) {
            retcode = drvlist[d]->init_fn(u);
            if (EOK != retcode) {
                //kerrprintf("Driver %s[%u] failed to init.\n",
                //          drvlist[d]->name,
                //        u);
            } else {
                retptr = device_create(u, drvlist[d]);
                if (!retptr) {
                    retval = EGENERIC;
                }
            }
        }
    }

    return (retval);
}

int net_drivers_list_init(net_driver_t *drvlist[], unsigned drvlistsize)
{
    /* Ethernet only !!! */
    unsigned d, u;
    int retcode;
    int retval = EOK;
    net_interface_t *retptr;

    for (d = 0; d < drvlistsize; d++) {
        for (u = 0; u < DRV_UNIT_MAX; u++) {
            retcode = drvlist[d]->init_fn(u);
            if (EOK != retcode) {
                //kerrprintf("Driver %s[%u] failed to init.\n",
                //          drvlist[d]->name,
                //        u);
            } else {
                retptr = net_interface_create(drvlist[d]);
                if (!retptr) {
                    retval = EGENERIC;
                }
            }
        }
    }

    return (retval);
}

/*
 * Private section
 */

BOOL init_drivers_lib()
{
    return (TRUE);
}
