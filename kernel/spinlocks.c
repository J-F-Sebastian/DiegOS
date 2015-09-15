#include <types_common.h>
#include <diegos/spinlocks.h>
#include <diegos/kernel.h>

void spinlock_init(spinlock_t *sl)
{
    *sl = FALSE;
}

void spinlock_lock(spinlock_t *sl)
{
    while (TRUE == *sl) {
        thread_suspend();
    }

    *sl = TRUE;
}

void spinlock_unlock(spinlock_t *sl)
{
    *sl = FALSE;
}

