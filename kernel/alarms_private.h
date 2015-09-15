#ifndef _ALARM_PRIVATE_H_
#define _ALARM_PRIVATE_H_

/*
 * Initialize the alarms library.
 * Must be called internally by the kernel init
 * routine.
 *
 * RETURN VALUES
 *
 * TRUE if initialization succeded
 * FALSE in any other case
 */
BOOL init_alarms_lib(void);

#endif

