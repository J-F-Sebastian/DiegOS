#include <limits.h>
#include <stdint.h>
#include <platform_config.h>
#include <diegos/delays.h>

#include "kprintf.h"
#include "platform_include.h"

unsigned long loops_mdelay;

void calibrate_delay (volatile unsigned long *ticks)
{
    unsigned long oldticks;
    unsigned long resolution, finder;

    finder = 10000;
    resolution = UINT_MAX/1024;

    while (1) {
        oldticks = *ticks;

        /*
         * wait for the tick to be updated
         */
        while (oldticks == *ticks) {};
        oldticks = *ticks;
        delay_loop(finder);
        if (oldticks < *ticks) {
            break;
        } else {
            finder += resolution;
        }
    }

    /*
     * We know finder's value is overestimating the correct value
     */
    resolution /= 2;

    while (1) {
        oldticks = *ticks;

        /*
         * wait for the tick to be updated
         */
        while (oldticks == *ticks) {};
        oldticks = *ticks;
        delay_loop(finder);
        if (oldticks < *ticks) {
            if (finder < resolution) {
                resolution = finder/2 + 1;
            }
            finder -= resolution;
        } else {
            finder += resolution;
            resolution /= 2;
            if (!resolution) {
                finder--;
                break;
            }
        }
    }

    resolution = 8;
    while (resolution--) {
        oldticks = *ticks;

        /*
         * wait for the tick to be updated
         */
        while (oldticks == *ticks) {};
        oldticks = *ticks;
        delay_loop(finder);
        if (oldticks < *ticks) {
            finder++;
        } else {
            finder--;
        }
    }

    loops_mdelay = finder;
}

void mdelay(unsigned long msecs)
{
    delay_loop(loops_mdelay*msecs);
}

void udelay(unsigned long usecs)
{
    unsigned long temp = loops_mdelay*usecs/1000;
    if (((loops_mdelay*usecs) % 1000) > 499) temp++;

    delay_loop(temp);
}

void ndelay(unsigned long nsecs)
{
    unsigned long temp = loops_mdelay*nsecs/1000000;
    if (((loops_mdelay*nsecs) % 1000000) > 499999) temp++;

    delay_loop(temp);
}
