#ifndef _KPUTB_H_
#define _KPUTB_H_

/*
 * This function is platform dependent, it must be
 * defined in platform sources and must be linked to
 * the kernel.
 * PARAMETERS
 * char *buffer - character buffer to be sent to the console
 *                TTY output.
 * unsigned bytes - size fo the buffer.
 */
extern void kputb(char *buffer, unsigned bytes);

#endif
