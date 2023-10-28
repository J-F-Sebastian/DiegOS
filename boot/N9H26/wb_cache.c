/**************************************************************************//**
 * @file     wb_cache.c
 * @version  V3.00
 * @brief    N9H26 series SYS driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/****************************************************************************
* FILENAME
*   wb_cache.c
*
* VERSION
*   1.0
*
* DESCRIPTION
*   The cache related functions of Nuvoton ARM9 MCU
*
* HISTORY
*   2008-06-25  Ver 1.0 draft by Min-Nan Cheng
* Modification
*   2011-06-01  Ver 1.1 draft by Shih-Wen Chou
*
* REMARK
*   None
 **************************************************************************/
#include <stdint.h>
#include <types_common.h>
#include "wblib.h"

BOOL volatile _sys_IsCacheOn = FALSE;
int32_t volatile _sys_CacheMode;
extern void sys_flush_and_clean_dcache(void);
extern void sysDisableDCache(void);
extern void sysDisableICache(void);
extern int sysInitMMUTable(int);


int32_t sysGetSdramSizebyMB()
{
    unsigned int volatile reg, totalsize=0;

    reg = inpw(REG_SDSIZE0) & 0x07;
    switch(reg)
    {
    case 1:
        totalsize += 2;
        break;

    case 2:
        totalsize += 4;
        break;

    case 3:
        totalsize += 8;
        break;

    case 4:
        totalsize += 16;
        break;

    case 5:
        totalsize += 32;
        break;

    case 6:
        totalsize += 64;
        break;
    }

    reg = inpw(REG_SDSIZE1) & 0x07;
    switch(reg)
    {
    case 1:
        totalsize += 2;
        break;

    case 2:
        totalsize += 4;
        break;

    case 3:
        totalsize += 8;
        break;

    case 4:
        totalsize += 16;
        break;

    case 5:
        totalsize += 32;
        break;

    case 6:
        totalsize += 64;
        break;
    }

    if (totalsize != 0)
        return totalsize;
    else
        return (int32_t)WB_MEM_INVALID_MEM_SIZE;
}


int32_t sysEnableCache(uint32_t uCacheOpMode)
{
    sysInitMMUTable(uCacheOpMode);
    _sys_IsCacheOn = TRUE;
    _sys_CacheMode = uCacheOpMode;

    return Successful;
}
void sys_flush_and_clean_dcache(void)
{
    __asm volatile(
        " tci_loop100%=:                              \n"
        "     MRC p15, #0, r15, c7, c14, #3         \n"     /* test clean and invalidate */
        "     BNE tci_loop100%=                       \n"
        : : : "memory");
}

void sysDisableCache()
{
    int temp = 0;

    sys_flush_and_clean_dcache();
    __asm volatile
    (
        /*----- flush I, D cache & write buffer -----*/
        "MOV %0, #0x0				\n\t"
        "MCR p15, 0, %0, c7, c5, #0 	\n\t" /* flush I cache */
        "MCR p15, 0, %0, c7, c6, #0  \n\t" /* flush D cache */
        "MCR p15, 0, %0, c7, c10,#4  \n\t" /* drain write buffer */

        /*----- disable Protection Unit -----*/
        "MRC p15, 0, %0, c1, c0, #0   \n\t" /* read Control register */
        "BIC %0, %0, #0x01            \n\t"
        "MCR p15, 0, %0, c1, c0, #0   \n\t" /* write Control register */
        : :"r"(temp) : "memory"
    );

    _sys_IsCacheOn = FALSE;
    _sys_CacheMode = CACHE_DISABLE;
}

void sysFlushCache(int32_t nCacheType)
{
    volatile int temp = 0;

    switch (nCacheType)
    {
    case I_CACHE:
        __asm volatile
        (
            /*----- flush I-cache -----*/
            "MOV %0, #0x0  \n\t"
            "MCR p15, #0, %0, c7, c5, 0  \n\t" /* invalidate I cache */
            : "=r"(temp)
            : "0" (temp)
            : "memory"
        );
        break;

    case D_CACHE:
        sys_flush_and_clean_dcache();
        __asm volatile
        (
            /*----- flush D-cache & write buffer -----*/
            "MOV %0, #0x0 \n\t"
            "MCR p15, #0, %0, c7, c10, #4 \n\t" /* drain write buffer */
            :"=r" (temp)
            :"0"  (temp)
            :"memory"
        );
        break;

    case I_D_CACHE:
        sys_flush_and_clean_dcache();
        __asm volatile
        (
            /*----- flush I, D cache & write buffer -----*/
            "MOV %0, #0x0  \n\t"
            "MCR p15, #0, %0, c7, c5, #0  \n\t" /* invalidate I cache */
            "MCR p15, #0, %0, c7, c10, #4 \n\t" /* drain write buffer */
            :"=r" (temp)
            :"0"  (temp)
            :"memory"
        );
        break;

    default:
        ;
    }
}

void sysInvalidCache()
{
    volatile int temp = 0;

    __asm volatile
    (
        "MOV %0, #0x0 \n\t"
        "MCR p15, #0, %0, c7, c7, #0 \n\t" /* invalidate I and D cache */
        :"=r" (temp)
        :"0" (temp)
        :"memory"
    );
}

BOOL sysGetCacheState()
{
    return _sys_IsCacheOn;
}


int32_t sysGetCacheMode()
{
    return _sys_CacheMode;
}


int32_t _sysLockCode(uint32_t addr, int32_t size)
{
    volatile int i, cnt, temp = 0;

    __asm volatile
    (
        /* use way3 to lock instructions */
        "MRC p15, #0, %0, c9, c0, #1 \n\t"
        "ORR %0, %0, #0x07 \n\t"
        "MCR p15, #0, %0, c9, c0, #1 \n\t"
        :"=r" (temp)
        :"0" (temp)
        :"memory"
    );

    if (size % 16)  cnt = (size/16) + 1;
    else            cnt = size / 16;

    for (i=0; i<cnt; i++)
    {
        __asm volatile
        (
            "MCR p15, #0, r0, c7, c13, #1 \n\t"
        );

        addr += 16;
    }

    __asm volatile
    (
        /* use way3 to lock instructions */
        "MRC p15, #0, %0, c9, c0, #1 \n\t"
        "BIC %0, %0, #0x07  \n\t"
        "ORR %0, %0, #0x08 \n\t"
        "MCR p15, #0, %0, c9, c0, #1 \n\t"
        :"=r" (temp)
        :"0"  (temp)
        :"memory"
    );

    return Successful;

}


int32_t _sysUnLockCode()
{
    volatile int temp = 0;

    /* unlock I-cache way 3 */
    __asm volatile
    (
        "MRC p15, #0, %0, c9, c0, #1  \n"
        "BIC %0, %0, #0x08 \n"
        "MCR p15, #0, %0, c9, c0, #1  \n"
        :"=r" (temp)
        :"0"  (temp)
        :"memory"
    );

    return Successful;
}


