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
int drivers_list_init(const void *drvlist[], unsigned drvlistsize)
{
    unsigned d, u;
    int retcode;
    int retval = EOK;
    device_t *retptr;
	driver_header_t *cmn; 

    for (d = 0; d < drvlistsize; d++) {
        cmn = (driver_header_t *)drvlist[d];
        for (u = 0; u < DRV_UNIT_MAX; u++) {			
            retcode = cmn->init_fn(u);
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
            retcode = drvlist[d]->cmn.init_fn(u);
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
