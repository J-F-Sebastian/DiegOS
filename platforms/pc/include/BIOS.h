#ifndef _BIOS_H_
#define _BIOS_H_

#include <stdint.h>

/*
Values for diskette drive type:
 01h	360K
 02h	1.2M
 03h	720K
 04h	1.44M
 05h	??? (reportedly an obscure drive type shipped on some IBM machines)
	2.88M on some machines (at least AMI 486 BIOS)
 06h	2.88M
 10h	ATAPI Removable Media Device
*/

/*
 * drv 0 1 2 3 ....
 *     0x80 hard disk
 */
int INT13_read_drive_param(const uint8_t drv,
			   uint8_t * drv_type, uint16_t * C, uint8_t * H, uint8_t * S);

#endif
