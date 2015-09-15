#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <types_common.h>

/*
 * kernel clock callback definition.
 * This prototype has to be used with clock_add_cb
 * and clock_del_db.
 *
 * PARAMS
 * ticks - system counter of ticks elapsed since boot time.
 *
 * RETURN VALUES
 * none
 */
typedef void (*kernel_clock_cb)(uint64_t ticks);

/*
 * Init clock library, this will configure the system ticks
 * and the clock resolution.
 * Must be called internally by kernel init routine.
 *
 * RETURN VALUES
 * TRUE if initliazation succeded
 * FALSE in any other case
 */
BOOL clock_init (void);

/*
 * Add a callback to the clock library. Every system tick
 * the registered callbacks will be invoked, having as an
 * argument the actual system tick.
 */
BOOL clock_add_cb (kernel_clock_cb cb);

/*
 * Remove a callback from the clock library.
 */
BOOL clock_del_cb (kernel_clock_cb cb);

/*
 * Get system ticks.
 *
 * RETURN VALUES
 * 64 bit counter of ticks elapsed since boot time.
 */
uint64_t clock_get_ticks (void);

/*
 * Convert milliseconds into system ticks.
 *
 * RETURN VALUES
 * 64 bit counter of the ticks elapsed in msecs.
 */
uint64_t clock_convert_msecs_to_ticks (unsigned msecs);

#endif

