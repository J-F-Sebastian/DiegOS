#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "fdescr_private.h"

#define DEV_PATH "/dev"

int open (const char *filename, int flags, int perms)
{
    unsigned fd, flg;

    /*
     * Only devices right now!
     */
    if (strncmp(filename, DEV_PATH, sizeof(DEV_PATH)-1)) {
        return (-1);
    }

    /*
     * To be added: exclusive access, multiple same-name opens
     */

    for (fd = 0;
            (fd < NELEMENTS(fdarray)) && (fdarray[fd].flags & FD_DATA_IS_INUSE);
            fd++) {};

    if (NELEMENTS(fdarray) == fd) {
        return (-1);
    }

    switch (flags & 0xFF) {
    case O_WRONLY:
        flg = FD_DATA_IS_W;
        break;
    case O_RDONLY:
        flg = FD_DATA_IS_R;
        break;
    case O_RDWR:
        flg = (FD_DATA_IS_W | FD_DATA_IS_R);
        break;
    default:
        return (-1);
    }

    fdarray[fd].absfname = malloc(strlen(filename));
    if (!fdarray[fd].absfname) {
        return (-1);
    }

    fdarray[fd].rawdev = device_lookup_name(filename+sizeof(DEV_PATH));

    if (!fdarray[fd].rawdev) {
        free(fdarray[fd].absfname);
        fdarray[fd].absfname = NULL;
        return (-1);
    }

    flg |= FD_DATA_IS_RAW;

    strcpy(fdarray[fd].absfname, filename);
    fdarray[fd].flags = flg;
    fdarray[fd].flags |= FD_DATA_IS_INUSE;

    return (fd);
}
