#include <unistd.h>
#include <errno.h>
#include "fdescr_private.h"

ssize_t write (int fd, const void *buf, size_t n)
{
    unsigned retries = 10;
    int retcode;

    if (!buf || (fd < 0) || (fd >= (int)NELEMENTS(fdarray)) ||
            !(fdarray[fd].flags & FD_DATA_IS_INUSE)) {
        return (-1);
    }

    if (fdarray[fd].flags & FD_DATA_IS_RAW) {
        do {
            retcode = device_io_tx(fdarray[fd].rawdev, (char *)buf, n);
        } while (retries-- && (retcode == EAGAIN));
        if (retcode < EOK) {
            return (-1);
        }
    } else if (fdarray[fd].flags & FD_DATA_IS_SOCK) {
    } else {
        return (-1);
    }

    return (n);
}
