/*
 * fcntl.c
 *
 *  Created on: Dec 15, 2014
 *      Author: diego
 */

#include <fcntl.h>
#include <errno.h>

#include "fdescr_private.h"

int fcntl(int fd, int cmd, ...)
{
    fd_data_t *fdata = fdget(fd);
    int retcode;

    if (!fdata) {
        errno = EINVAL;
        return (-1);
    }

    switch (cmd) {
    case F_DUPFD: /* duplicate file descriptor */
        break;
    case F_GETFD: /* get file descriptor flags */
        break;
    case F_SETFD: /* set file descriptor flags */
        break;
    case F_GETFL: /* get file status flags */
        retcode = 0;
        if ((fdata->flags
            & (FD_DATA_IS_R | FD_DATA_IS_W)) == (FD_DATA_IS_R | FD_DATA_IS_W)) {
            retcode = O_RDWR;
        } else if (fdata->flags & FD_DATA_IS_R) {
            retcode = O_RDONLY;
        } else if (fdata->flags & FD_DATA_IS_W) {
            retcode = O_WRONLY;
        }
        return (retcode);
    case F_SETFL: /* set file status flags */
        break;
    case F_GETOWN:
        /* get the process or process group ID specified to
         * receive SIGURG signals when out-of-band data is available.
         * SOCKETS ONLY
         */
        break;
    case F_SETOWN:
        /* set the process or process group ID specified to
         * receive SIGURG signals when out-of-band data is available.
         * SOCKETS ONLY
         */
        break;
    case F_GETLK: /* get record locking information */
        break;
    case F_SETLK: /* set record locking information */
        break;
    case F_SETLKW: /* set record locking info; wait if blocked */
        break;
    }

    return (-1);
}

