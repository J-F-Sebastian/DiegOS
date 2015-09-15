#ifndef _KPRINTF_H_
#define _KPRINTF_H_

#include <stdarg.h>

int kprintf(const char *fmt, ...);

int kmsgprintf(const char *fmt, ...);

int kerrprintf(const char *fmt, ...);

int kvprintf(const char *fmt, va_list ap);

#endif

