#include <unistd.h>

off_t lseek (int fd, off_t offset, int _whence)
{
    if ((STDIN_FILENO == fd) ||
            (STDOUT_FILENO == fd) ||
            (STDERR_FILENO == fd)) {
        return ((off_t) -1);
    }

    /*
     * No FS implementation right now!
     */
    return ((off_t) -1);
}
