/*******************************************
 *********** SOURCE FILE *******************
 *******************************************
 *
 * Name:
 * Author:
 * Date:
 *******************************************
 * Description:
 *
 *******************************************/

#include <diegos/kernel.h>

#include "platform_include.h"
#include "idle_thread.h"

void idle_thread_entry()
{
    while (TRUE) {
        power_save();
        thread_suspend();
    }
}

