/****************************************************************************
 * @file     wbio.h
 * @version  V3.00
 * @brief    N9H26 series SYS driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *     WBIO.h
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     This file contains I/O macros, and basic macros and inline functions.
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     03/28/02      Ver 1.0 Created by PC30 YCHuang
 *
 * REMARK
 *     None
 **************************************************************************/

#ifndef _WBIO_H
#define _WBIO_H

#ifndef _WBTYPES_H_
#include "wbtypes.h"
#endif

#define outpb(port,value)     (*((uint8_t volatile *) (port))=value)
#define inpb(port)            (*((uint8_t volatile *) (port)))
#define outphw(port,value)    (*((uint16_t volatile *) (port))=value)
#define inphw(port)           (*((uint16_t volatile *) (port)))
#define outpw(port,value)     (*((uint32_t volatile *) (port))=value)
#define inpw(port)            (*((uint32_t volatile *) (port)))

#define readb(addr)           (*(uint8_t volatile *)(addr))
#define writeb(addr,x)        ((*(uint8_t volatile *)(addr)) = (uint8_t volatile)x)
#define readhw(addr)          (*(uint16_t volatile *)(addr))
#define writehw(addr,x)       ((*(uint16_t volatile *)(addr)) = (uint16_t volatile)x)
#define readw(addr)           (*(uint32_t volatile *)(addr))
#define writew(addr,x)        ((*(uint32_t volatile *)(addr)) = (uint32_t volatile)x)

#define outp8(port,value)     (*((uint8_t volatile *) (port))=value)
#define inp8(port)            (*((uint8_t volatile *) (port)))
#define outp16(port,value)    (*((uint16_t volatile *) (port))=value)
#define inp16(port)           (*((uint16_t volatile *) (port)))
#define outp32(port,value)     (*((uint32_t volatile *) (port))=value)
#define inp32(port)            (*((uint32_t volatile *) (port)))

#define Maximum(a,b)          (a>b ? a : b)
#define Minimum(a,b)          (a<b ? a : b)
#define Middle(a,b)           ((a+b)/2)
#define Equal(a,b)            (a==b ? 1 : 0)
#define NotEqual(a,b)         (a==b ? 0 : 1)
#define GreaterEqual(a,b)     (a>=b ? 1 : 0)
#define LittleEqual(a,b)      (a<=b ? 1 : 0)

inline uint16_t Swap16(uint16_t val)
{
	return (val << 8) | (val >> 8);
}

inline uint32_t Swap32(uint32_t val)
{
	return (val << 24) | ((val << 8) & 0xff0000) | ((val >> 8) & 0xff00) | (val >> 24);
}

inline uint16_t Get16(uint8_t * addr)
{
	return ((addr[1] << 8) | addr[0]);
}

inline uint32_t Get32(uint8_t * addr)
{
	return (addr[3] << 24) | (addr[2] << 16) | (addr[1] << 8) | addr[0];
}

inline void Set16(uint16_t val, uint8_t * addr)
{
	addr[0] = (uint8_t) val;
	addr[1] = (uint8_t) (val >> 8);
}

inline void Set32(uint32_t val, uint8_t * addr)
{
	addr[0] = (uint8_t) val;
	addr[1] = (uint8_t) (val >> 8);
	addr[2] = (uint8_t) (val >> 16);
	addr[3] = (uint8_t) (val >> 24);
}

#endif				/* _WBIO_H */
