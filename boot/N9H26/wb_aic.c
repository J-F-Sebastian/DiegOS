/**************************************************************************//**
 * @file     wb_aic.c
 * @version  V3.00
 * @brief    N9H26 series SYS driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/****************************************************************************
* FILENAME
*   wb_aic.c
*
* VERSION
*   1.0
*
* DESCRIPTION
*   The AIC related functions of Nuvoton ARM9 MCU
*
* HISTORY
*   2008-06-25  Ver 1.0 draft by Min-Nan Cheng
* Modification
*   2011-06-01  Ver 1.1 draft by Shih-Wen Chou
*
* REMARK
*   None
 **************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <types_common.h>
#include "wblib.h"
#include "wberrcode.h"
#define WB_MIN_INT_SOURCE  1
//#define WB_MAX_INT_SOURCE 31
#define WB_MAX_INT_SOURCE  47
//#define WB_NUM_OF_AICREG   32
#define WB_NUM_OF_AICREG   48

/* Global variables */
BOOL volatile _sys_bIsAICInitial = FALSE;
BOOL volatile _sys_bIsHWMode = TRUE;

/* declaration the function prototype */
extern void WB_Interrupt_Shell(void);

/* SCR and SVR register access function */
#define AIC_REG_OFFSET                          0x4
#define AICRegRead(RegBase, RegNum)             inpw(RegBase+RegNum*AIC_REG_OFFSET)
#define AICRegWrite(RegBase, RegNum, Value)         outpw(RegBase+RegNum*AIC_REG_OFFSET, Value)

/* Interrupt Handler Table */
typedef void (*sys_pvFunPtr)();   /* function pointer */
sys_pvFunPtr sysIrqHandlerTable[] = { 0,                /* 0 */
                                      WB_Interrupt_Shell,   /* 1 */
                                      WB_Interrupt_Shell,   /* 2 */
                                      WB_Interrupt_Shell,   /* 3 */
                                      WB_Interrupt_Shell,   /* 4 */
                                      WB_Interrupt_Shell,   /* 5 */
                                      WB_Interrupt_Shell,   /* 6 */
                                      WB_Interrupt_Shell,   /* 7 */
                                      WB_Interrupt_Shell,   /* 8 */
                                      WB_Interrupt_Shell,   /* 9 */
                                      WB_Interrupt_Shell,   /* 10 */
                                      WB_Interrupt_Shell,   /* 11 */
                                      WB_Interrupt_Shell,   /* 12 */
                                      WB_Interrupt_Shell,   /* 13 */
                                      WB_Interrupt_Shell,   /* 14 */
                                      WB_Interrupt_Shell,   /* 15 */
                                      WB_Interrupt_Shell,   /* 16 */
                                      WB_Interrupt_Shell,   /* 17 */
                                      WB_Interrupt_Shell,   /* 18 */
                                      WB_Interrupt_Shell,   /* 19 */
                                      WB_Interrupt_Shell,   /* 20 */
                                      WB_Interrupt_Shell,   /* 21 */
                                      WB_Interrupt_Shell,   /* 22 */
                                      WB_Interrupt_Shell,   /* 23 */
                                      WB_Interrupt_Shell,   /* 24 */
                                      WB_Interrupt_Shell,   /* 25 */
                                      WB_Interrupt_Shell,   /* 26 */
                                      WB_Interrupt_Shell,   /* 27 */
                                      WB_Interrupt_Shell,   /* 28 */
                                      WB_Interrupt_Shell,   /* 29 */
                                      WB_Interrupt_Shell,   /* 30 */
                                      WB_Interrupt_Shell,   /* 31 */
                                      WB_Interrupt_Shell,   /* 32 */
                                      WB_Interrupt_Shell,   /* 33 */
                                      WB_Interrupt_Shell,   /* 34 */
                                      WB_Interrupt_Shell,   /* 35 */
                                      WB_Interrupt_Shell,   /* 36 */
                                      WB_Interrupt_Shell,   /* 37 */
                                      WB_Interrupt_Shell,   /* 38 */
                                      WB_Interrupt_Shell,   /* 39 */
                                      WB_Interrupt_Shell,   /* 40 */
                                      WB_Interrupt_Shell,   /* 41 */
                                      WB_Interrupt_Shell,   /* 42 */
                                      WB_Interrupt_Shell,   /* 43 */
                                      WB_Interrupt_Shell,   /* 44 */
                                      WB_Interrupt_Shell,   /* 45 */
                                      WB_Interrupt_Shell,   /* 46 */
                                      WB_Interrupt_Shell    /* 47 */
                                    };

sys_pvFunPtr sysFiqHandlerTable[] = { 0,
                                      WB_Interrupt_Shell,   /* 1 */
                                      WB_Interrupt_Shell,   /* 2 */
                                      WB_Interrupt_Shell,   /* 3 */
                                      WB_Interrupt_Shell,   /* 4 */
                                      WB_Interrupt_Shell,   /* 5 */
                                      WB_Interrupt_Shell,   /* 6 */
                                      WB_Interrupt_Shell,   /* 7 */
                                      WB_Interrupt_Shell,   /* 8 */
                                      WB_Interrupt_Shell,   /* 9 */
                                      WB_Interrupt_Shell,   /* 10 */
                                      WB_Interrupt_Shell,   /* 11 */
                                      WB_Interrupt_Shell,   /* 12 */
                                      WB_Interrupt_Shell,   /* 13 */
                                      WB_Interrupt_Shell,   /* 14 */
                                      WB_Interrupt_Shell,   /* 15 */
                                      WB_Interrupt_Shell,   /* 16 */
                                      WB_Interrupt_Shell,   /* 17 */
                                      WB_Interrupt_Shell,   /* 18 */
                                      WB_Interrupt_Shell,   /* 19 */
                                      WB_Interrupt_Shell,   /* 20 */
                                      WB_Interrupt_Shell,   /* 21 */
                                      WB_Interrupt_Shell,   /* 22 */
                                      WB_Interrupt_Shell,   /* 23 */
                                      WB_Interrupt_Shell,   /* 24 */
                                      WB_Interrupt_Shell,   /* 25 */
                                      WB_Interrupt_Shell,   /* 26 */
                                      WB_Interrupt_Shell,   /* 27 */
                                      WB_Interrupt_Shell,   /* 28 */
                                      WB_Interrupt_Shell,   /* 29 */
                                      WB_Interrupt_Shell,   /* 30 */
                                      WB_Interrupt_Shell,   /* 31 */
                                      WB_Interrupt_Shell,   /* 32 */
                                      WB_Interrupt_Shell,   /* 33 */
                                      WB_Interrupt_Shell,   /* 34 */
                                      WB_Interrupt_Shell,   /* 35 */
                                      WB_Interrupt_Shell,   /* 36 */
                                      WB_Interrupt_Shell,   /* 37 */
                                      WB_Interrupt_Shell,   /* 38 */
                                      WB_Interrupt_Shell,   /* 39 */
                                      WB_Interrupt_Shell,   /* 40 */
                                      WB_Interrupt_Shell,   /* 41 */
                                      WB_Interrupt_Shell,   /* 42 */
                                      WB_Interrupt_Shell,   /* 43 */
                                      WB_Interrupt_Shell,   /* 44 */
                                      WB_Interrupt_Shell,   /* 45 */
                                      WB_Interrupt_Shell,   /* 46 */
                                      WB_Interrupt_Shell    /* 47 */
                                    };

/* Interrupt Handler */
static void __attribute__ ((interrupt("IRQ"))) sysIrqHandler(void)
{
    if (_sys_bIsHWMode)
    {
        uint32_t volatile _mIPER, _mISNR;

        _mIPER = inpw(REG_AIC_IPER) >> 2;
        //outpw(REG_AIC_IPER,0); //For test mode

        _mISNR = inpw(REG_AIC_ISNR);
        if (_mISNR != 0)
            if (_mIPER == _mISNR)
                (*sysIrqHandlerTable[_mIPER])();
        outpw(REG_AIC_EOSCR, 1);
    }
    else
    {
        uint32_t volatile _mISR, i;

        _mISR = inpw(REG_AIC_ISR);
        for (i=1; i<32; i++)
            if (_mISR & (1 << i))
                (*sysIrqHandlerTable[i])();

        _mISR = inpw(REG_AIC_ISRH);
        for (i=32; i<WB_NUM_OF_AICREG; i++)
            if (_mISR & (1 << (i-32)))
                (*sysIrqHandlerTable[i])();

    }
}
static void __attribute__ ((interrupt("FIQ"))) sysFiqHandler(void)
{
    if (_sys_bIsHWMode)
    {
        uint32_t volatile _mIPER, _mISNR;

        _mIPER = inpw(REG_AIC_IPER) >> 2;
        //outpw(REG_AIC_IPER,0); //For test mode
        _mISNR = inpw(REG_AIC_ISNR);
        if (_mISNR != 0)
            if (_mIPER == _mISNR)
                (*sysFiqHandlerTable[_mIPER])();
        outpw(REG_AIC_EOSCR, 1);

    }
    else
    {
        uint32_t volatile _mISR, i;

        _mISR = inpw(REG_AIC_ISR);
        for (i=1; i<32; i++)
            if (_mISR & (1 << i))
                (*sysFiqHandlerTable[i])();

        _mISR = inpw(REG_AIC_ISRH);
        for (i=32; i<WB_NUM_OF_AICREG; i++)
            if (_mISR & (1 << (i-32)))
                (*sysFiqHandlerTable[i])();
    }
}

void WB_Interrupt_Shell()
{

}
void sysInitializeAIC()
{
    //void * _mOldIrqVect, _mOldFiqVect;

    *((unsigned volatile *)0x18) = 0xe59ff018;
    *((unsigned volatile *)0x1C) = 0xe59ff018;

    //_mOldIrqVect = *(void volatile *)0x38;
    *(uintptr_t *)0x38 = (uintptr_t)sysIrqHandler;

    //_mOldFiqVect = *(void volatile *)0x3C;
    *(uintptr_t *)0x3C = (uintptr_t)sysFiqHandler;

    if (sysGetCacheState() == TRUE)
        sysFlushCache(I_CACHE);
}


/* Interrupt library functions */
int32_t sysDisableInterrupt(INT_SOURCE_E eIntNo)
{
    if ((eIntNo > WB_MAX_INT_SOURCE) || (eIntNo < WB_MIN_INT_SOURCE))
        return (int32_t)WB_PM_INVALID_IRQ_NUM;
    if (eIntNo > 31)
        outpw(REG_AIC_MDCRH, (1 << (eIntNo-32)));
    else
        outpw(REG_AIC_MDCR, (1 << eIntNo));
    return Successful;
}

#if 0
int32_t sysDisableGroupInterrupt(INT_SOURCE_GROUP_E eIntNo)
{
    outpw(REG_AIC_GEN, inpw(REG_AIC_GEN) & ~eIntNo);
    return Successful;
}
#endif

int32_t sysEnableInterrupt(INT_SOURCE_E eIntNo)
{
    if ((eIntNo > WB_MAX_INT_SOURCE) || (eIntNo < WB_MIN_INT_SOURCE))
        return (int32_t)WB_PM_INVALID_IRQ_NUM;

    if (eIntNo > 31)
        outpw(REG_AIC_MECRH, (1 << (eIntNo-32)));
    else
        outpw(REG_AIC_MECR, (1 << eIntNo));
    return Successful;
}


 void *sysInstallExceptionHandler(int32_t nExceptType, void * pvNewHandler)
{
    uintptr_t _mOldVect;

    switch (nExceptType)
    {
    case WB_SWI:
        _mOldVect = *(uintptr_t *)0x28;
        *(uintptr_t *)0x28 = (uintptr_t)pvNewHandler;
        break;

    case WB_D_ABORT:
        _mOldVect = *(uintptr_t *)0x30;
        *(uintptr_t *)0x30 = (uintptr_t)pvNewHandler;
        break;

    case WB_I_ABORT:
        _mOldVect = *(uintptr_t *)0x2C;
        *(uintptr_t *)0x2C = (uintptr_t)pvNewHandler;
        break;

    case WB_UNDEFINE:
        _mOldVect = *(uintptr_t *)0x24;
        *(uintptr_t *)0x24 = (uintptr_t)pvNewHandler;
        break;

    default:
	_mOldVect = 0;
        break;
    }
    return (void *)_mOldVect;
}

void * sysInstallFiqHandler(void * pvNewISR)
{
    uintptr_t _mOldVect;

    _mOldVect = *(uintptr_t *)0x3C;
    *(uintptr_t *)0x3C = (uintptr_t)pvNewISR;
    return (void *)_mOldVect;
}

void * sysInstallIrqHandler(void * pvNewISR)
{
    uintptr_t _mOldVect;

    _mOldVect = *(uintptr_t *)0x38;
    *(uintptr_t *)0x38 = (uintptr_t)pvNewISR;
    return (void *)_mOldVect;
}

//OK
void * sysInstallISR(int32_t nIntTypeLevel, INT_SOURCE_E eIntNo, void * pvNewISR)
{
    void *  _mOldVect;
    uint32_t _mRegValue;

    if (!_sys_bIsAICInitial)
    {
        sysInitializeAIC();
        _sys_bIsAICInitial = TRUE;
    }

    _mRegValue = AICRegRead(REG_AIC_SCR1, (eIntNo/4));
    _mRegValue = _mRegValue & ( 0xFFFFFFF8<<((eIntNo%4)*8) | ( (1<<((eIntNo%4)*8))-1 ) );
    _mRegValue = _mRegValue | nIntTypeLevel<<((eIntNo%4)*8);
    AICRegWrite(REG_AIC_SCR1, (eIntNo/4), _mRegValue );

    switch (nIntTypeLevel)
    {
    case FIQ_LEVEL_0:
        _mOldVect = (void *) sysFiqHandlerTable[eIntNo];
        sysFiqHandlerTable[eIntNo] = (sys_pvFunPtr)pvNewISR;
        break;

    case IRQ_LEVEL_1:
    case IRQ_LEVEL_2:
    case IRQ_LEVEL_3:
    case IRQ_LEVEL_4:
    case IRQ_LEVEL_5:
    case IRQ_LEVEL_6:
    case IRQ_LEVEL_7:
        _mOldVect = (void *) sysIrqHandlerTable[eIntNo];
        sysIrqHandlerTable[eIntNo] = (sys_pvFunPtr)pvNewISR;
        break;

    default:
	_mOldVect = NULL;
        break;
    }
    return _mOldVect;
}
//OK
int32_t sysSetGlobalInterrupt(int32_t nIntState)
{
    switch (nIntState)
    {
    case ENABLE_ALL_INTERRUPTS:
        outpw(REG_AIC_MECR, 0xFFFFFFFF);
        outpw(REG_AIC_MECRH, 0xFFFF);
        break;

    case DISABLE_ALL_INTERRUPTS:
        outpw(REG_AIC_MDCR, 0xFFFFFFFF);
        outpw(REG_AIC_MDCRH, 0xFFFF);
        break;

    default:
        ;
    }
    return Successful;
}
//OK
int32_t sysSetInterruptPriorityLevel(INT_SOURCE_E eIntNo, uint32_t uIntLevel)
{
    uint32_t _mRegValue;

    if ((eIntNo > WB_MAX_INT_SOURCE) || (eIntNo < WB_MIN_INT_SOURCE))
        return (int32_t)WB_PM_INVALID_IRQ_NUM;

    _mRegValue = AICRegRead(REG_AIC_SCR1, (eIntNo/4));
    _mRegValue = _mRegValue & ( 0xFFFFFFF8<<((eIntNo%4)*8) | ( (1<<((eIntNo%4)*8))-1 ) );
    _mRegValue = _mRegValue | uIntLevel<<((eIntNo%4)*8);
    AICRegWrite(REG_AIC_SCR1, (eIntNo/4), _mRegValue);

    return Successful;
}
//OK
/*
typedef enum
{
    eDRVAIC_LOW_LEVEL,
    eDRVAIC_HIGH_LEVEL,
    eDRVAIC_FALLING,
    eDRVAIC_RISING
}E_DRVAIC_INT_TYPE;
*/
int32_t sysSetInterruptType(INT_SOURCE_E eIntNo, uint32_t uIntSourceType)
{
    uint32_t _mRegValue;

    if ((eIntNo > WB_MAX_INT_SOURCE) || (eIntNo < WB_MIN_INT_SOURCE))
        return (int32_t)WB_PM_INVALID_IRQ_NUM;

    _mRegValue = AICRegRead(REG_AIC_SCR1, (eIntNo/4));
    _mRegValue = _mRegValue & ( 0xFFFFFF3F<<((eIntNo & 0x3)*8) | ( (1<<((eIntNo%4)*8))-1 ) );
    _mRegValue = _mRegValue | ( uIntSourceType<<((eIntNo & 0x3)*8 + 6) );
    AICRegWrite(REG_AIC_SCR1, (eIntNo >> 2), _mRegValue );

    return Successful;
}
//OK
int32_t sysSetLocalInterrupt(int32_t nIntState)
{
    switch (nIntState)
    {
    case ENABLE_IRQ:
    case ENABLE_FIQ:
    case ENABLE_FIQ_IRQ:
        __asm
        (
            "mrs    r0, CPSR  \n"
            "bic    r0, r0, #0x80  \n"
            "msr    CPSR_c, r0  \n"
        );
        break;

    case DISABLE_IRQ:
        case DISABLE_FIQ:
            case DISABLE_FIQ_IRQ:
                    __asm
                    (
                        "MRS    r0, CPSR  \n"
                        "ORR    r0, r0, #0x80  \n"
                        "MSR    CPSR_c, r0  \n"
                    );
        break;

    default:
            ;
    }
    return Successful;
}
//OK
int32_t sysSetAIC2SWMode()
{
    _sys_bIsHWMode = FALSE;
    return Successful;
}

//OK
uint32_t  sysGetInterruptEnableStatus(void)
{
    return (inpw(REG_AIC_IMR));
}
uint32_t  sysGetInterruptHighEnableStatus(void)
{
    return (inpw(REG_AIC_IMRH));
}
//OK
BOOL sysGetIBitState()
{
    int32_t temp;
    __asm
    (
        "MRS %0, CPSR   \n"
        :"=r" (temp) : :
    );
    if (temp & 0x80)
        return FALSE;
    else
        return TRUE;
}


