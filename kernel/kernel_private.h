#ifndef _KERNEL_PRIVATE_H_
#define _KERNEL_PRIVATE_H_

/*
 * kernel functions, if one of these fails,
 * you get a kernel panic.
 */
void kernel_init(void);

void kernel_libs_init(void);

void kernel_threads_init(void);

void kernel_panic(const char *msg);

void kernel_run(void);

void kernel_done(void);

#endif