#include <unistd.h>
#include <diegos/kernel.h>

unsigned int sleep (unsigned int seconds)
{
    thread_delay(seconds*1000);
    return (0);
}

unsigned int msleep (unsigned int mseconds)
{
    thread_delay(mseconds);
    return (0);
}
