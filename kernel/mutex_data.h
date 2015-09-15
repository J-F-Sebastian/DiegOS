#ifndef MUTEX_DATA_H_INCLUDED
#define MUTEX_DATA_H_INCLUDED

/*******************************************
 *********** HEADER FILE *******************
 *******************************************
 *
 * Name:
 * Author:
 * Date:
 *******************************************
 * Description:
 *
 *******************************************/

struct mutex {
    list_node header;
    queue_inst wait_queue;
    uint8_t locker_tid;
    char name[16];
};

#endif // MUTEX_DATA_H_INCLUDED
