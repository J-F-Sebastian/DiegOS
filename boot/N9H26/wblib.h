/****************************************************************************
 * @file     wblib.h
 * @version  V3.00
 * @brief    N9H26 series SYS driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *     WBLIB.h
 *
 * VERSION
 *     1.1
 *
 * DESCRIPTION
 *     The header file of N9H26K system library.
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     2008-06-26  Ver 1.0 draft by Min-Nan Cheng
 *     2009-02-26  Ver 1.1 Changed for N9H26K MCU
 *
 * REMARK
 *     None
 **************************************************************************/
#ifndef _WBLIB_H
#define _WBLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "N9H26_reg.h"
#include "wberrcode.h"
#include "wbio.h"

//#define PD_RAM_BASE   0xFF000000
//#define PD_RAM_START  0xFF001000
//#define PD_RAM_SIZE   0x2000  /* 8KB Tmp Buffer */
#define PD_RAM_BASE     0xFF000000
#define PD_RAM_START    0xFF000800
#define PD_RAM_SIZE     0x2000	/* 8KB Tmp Buffer */

/* Error code return value */
#define WB_INVALID_PARITY           (SYSLIB_ERR_ID +1)
#define WB_INVALID_DATA_BITS        (SYSLIB_ERR_ID +2)
#define WB_INVALID_STOP_BITS        (SYSLIB_ERR_ID +3)
#define WB_INVALID_BAUD             (SYSLIB_ERR_ID +4)
#define WB_PM_PD_IRQ_Fail           (SYSLIB_ERR_ID +5)
#define WB_PM_Type_Fail             (SYSLIB_ERR_ID +6)
#define WB_PM_INVALID_IRQ_NUM       (SYSLIB_ERR_ID +7)
#define WB_MEM_INVALID_MEM_SIZE     (SYSLIB_ERR_ID +8)
#define WB_INVALID_TIME             (SYSLIB_ERR_ID +9)
#define WB_INVALID_CLOCK            (SYSLIB_ERR_ID +10)
#define E_ERR_CLK                    WB_INVALID_CLOCK
//-- function return value
#define    Successful  0
#define    Fail        1

#define EXTERNAL_CRYSTAL_CLOCK  12000000
//#define EXTERNAL_CRYSTAL_CLOCK  27000000

/* Define the vector numbers associated with each interrupt */
	typedef enum int_source_e {
		IRQ_WDT = 1,
		IRQ_EXTINT0 = 2,
		IRQ_EXTINT1 = 3,
		IRQ_EXTINT2 = 4,
		IRQ_EXTINT3 = 5,

		IRQ_IPSEC = 6,
		IRQ_SPU = 7,
		IRQ_I2S = 8,
		IRQ_VPOST = 9,
		IRQ_VIN = 10,

		IRQ_MDCT = 11,
		IRQ_BLT = 12,
		IRQ_VPE = 13,
		IRQ_HUART = 14,
		IRQ_TMR0 = 15,

		IRQ_TMR1 = 16,
		IRQ_UDC = 17,
		IRQ_SIC = 18,
		IRQ_SDIO = 19,
		IRQ_UHC = 20,

		IRQ_EHCI = 21,
		IRQ_OHCI = 22,
		IRQ_EDMA0 = 23,
		IRQ_EDMA1 = 24,
		IRQ_SPIMS0 = 25,

		IRQ_SPIMS1 = 26,
		IRQ_AUDIO = 27,
		IRQ_TOUCH = 28,
		IRQ_RTC = 29,
		IRQ_UART = 30,

		IRQ_PWM = 31,
		IRQ_JPG = 32,
		IRQ_VDE = 33,
		IRQ_VEN = 34,
		IRQ_SDIC = 35,

		IRQ_EMCTX = 36,
		IRQ_EMCRX = 37,
		IRQ_I2C = 38,
		IRQ_KPI = 39,
		IRQ_RSC = 40,

		IRQ_VTB = 41,
		IRQ_ROT = 42,
		IRQ_PWR = 43,
		IRQ_LVD = 44,
		IRQ_VIN1 = 45,

		IRQ_TMR2 = 46,
		IRQ_TMR3 = 47
	} INT_SOURCE_E;

	typedef enum {
		WE_EMAC = 0x1,
		WE_UHC20 = 02,
		WE_GPIO = 0x100,
		WE_RTC = 0x0200,
		//WE_SDH = 0x0400,
		WE_UART = 0x0800,
		WE_UDC = 0x1000,
		WE_UHC = 0x2000,
		WE_ADC = 0x4000,
		WE_KPI = 0x8000
	} WAKEUP_SOURCE_E;

	typedef struct datetime_t {
		uint32_t year;
		uint32_t mon;
		uint32_t day;
		uint32_t hour;
		uint32_t min;
		uint32_t sec;
	} DateTime_T;

/* Define constants for use timer in service parameters.  */
#define TIMER0            0
#define TIMER1            1
#define TIMER2            2
#define TIMER3            3
#define WDTIMER           4

#define ONE_SHOT_MODE     0
#define PERIODIC_MODE     1
#define TOGGLE_MODE       2
#define UNINTERRUPT_MODE  3

#define WDT_INTERVAL_0     0
#define WDT_INTERVAL_1     1
#define WDT_INTERVAL_2     2
#define WDT_INTERVAL_3     3

/* Define constants for use UART in service parameters.  */
#define WB_UART_0       0
#define WB_UART_1       1

#define WB_DATA_BITS_5    0x00
#define WB_DATA_BITS_6    0x01
#define WB_DATA_BITS_7    0x02
#define WB_DATA_BITS_8    0x03

#define WB_STOP_BITS_1    0x00
#define WB_STOP_BITS_2    0x04

#define WB_PARITY_NONE   0x00
#define WB_PARITY_ODD     0x08
#define WB_PARITY_EVEN    0x18

#define LEVEL_1_BYTE      0x0
#define LEVEL_4_BYTES     0x1
#define LEVEL_8_BYTES     0x2
#define LEVEL_14_BYTES    0x3
#define LEVEL_30_BYTES    0x4	/* High speed UART only */
#define LEVEL_46_BYTES    0x5
#define LEVEL_62_BYTES    0x6

/* Define constants for use AIC in service parameters.  */
#define WB_SWI            0
#define WB_D_ABORT        1
#define WB_I_ABORT        2
#define WB_UNDEFINE       3

/* The parameters for sysSetInterruptPriorityLevel() and
   sysInstallISR() use */
#define FIQ_LEVEL_0        0
#define IRQ_LEVEL_1        1
#define IRQ_LEVEL_2        2
#define IRQ_LEVEL_3        3
#define IRQ_LEVEL_4        4
#define IRQ_LEVEL_5        5
#define IRQ_LEVEL_6        6
#define IRQ_LEVEL_7        7

/* The parameters for sysSetGlobalInterrupt() use */
#define ENABLE_ALL_INTERRUPTS      0
#define DISABLE_ALL_INTERRUPTS     1

/* The parameters for sysSetInterruptType() use */
#define LOW_LEVEL_SENSITIVE        0x00
#define HIGH_LEVEL_SENSITIVE       0x01
#define NEGATIVE_EDGE_TRIGGER      0x02
#define POSITIVE_EDGE_TRIGGER      0x03

/* The parameters for sysSetLocalInterrupt() use */
#define ENABLE_IRQ                 0x7F
#define ENABLE_FIQ                 0xBF
#define ENABLE_FIQ_IRQ             0x3F
#define DISABLE_IRQ                0x80
#define DISABLE_FIQ                0x40
#define DISABLE_FIQ_IRQ            0xC0

/* Define Cache type  */
#define CACHE_WRITE_BACK        0
#define CACHE_WRITE_THROUGH     1
#define CACHE_DISABLE           -1

#define MMU_DIRECT_MAPPING  0
#define MMU_INVERSE_MAPPING 1

/* Define system clock come from */
	typedef enum {
		eSYS_EXT = 0,
		eSYS_MPLL = 1,	/* Generally, for memory clock */
		eSYS_APLL = 2,	/* Generally, for audio clock */
		eSYS_UPLL = 3	/* Generally, for system/engine clock */
	} E_SYS_SRC_CLK;

/* Define constants for use Cache in service parameters.  */
#define CACHE_4M        2
#define CACHE_8M        3
#define CACHE_16M       4
#define CACHE_32M       5
#define I_CACHE         6
#define D_CACHE         7
#define I_D_CACHE       8

/* Define UART initialization data structure */
	typedef struct UART_INIT_STRUCT {
		uint32_t uart_no;
		uint32_t uiFreq;
		uint32_t uiBaudrate;
		uint8_t uiDataBits;
		uint8_t uiStopBits;
		uint8_t uiParity;
		uint8_t uiRxTriggerLevel;
	} WB_UART_T;

/* Define the constant values of PM */
#define WB_PM_IDLE          1
#define WB_PM_PD            2
#define WB_PM_MIDLE         5

/* Define system library Timer functions */
	uint32_t sysGetTicks(int32_t nTimeNo);
	int32_t sysResetTicks(int32_t nTimeNo);
	int32_t sysUpdateTickCount(int32_t nTimeNo, uint32_t uCount);
	int32_t sysSetTimerReferenceClock(int32_t nTimeNo, uint32_t uClockRate);
	int32_t sysStartTimer(int32_t nTimeNo, uint32_t uTicksPerSecond, int32_t nOpMode);
	int32_t sysStopTimer(int32_t nTimeNo);
	int32_t sysSetTimerEvent(int32_t nTimeNo, uint32_t uTimeTick, void *pvFun);
	void sysClearTimerEvent(int32_t nTimeNo, uint32_t uTimeEventNo);
	void sysSetLocalTime(DateTime_T ltime);
	void sysGetCurrentTime(DateTime_T * curTime);
	void sysDelay(uint32_t uTicks);
	void sysClearWatchDogTimerCount(void);
	void sysClearWatchDogTimerInterruptStatus(void);
	void sysDisableWatchDogTimer(void);
	void sysDisableWatchDogTimerReset(void);
	void sysEnableWatchDogTimer(void);
	void sysEnableWatchDogTimerReset(void);
	void *sysInstallWatchDogTimerISR(int32_t nIntTypeLevel, void *pvNewISR);
	int32_t sysSetWatchDogTimerInterval(int32_t nWdtInterval);

/* Define system library UART functions */
#define UART_INT_RDA        0
#define UART_INT_RDTO       1
#define UART_INT_NONE       255

	typedef void (*PFN_SYS_UART_CALLBACK)(uint8_t * u8Buf, uint32_t u32Len);
	void sysUartPort(uint32_t u32Port);
	char sysGetChar(void);
	int32_t sysInitializeUART(WB_UART_T * uart);
	void sysPrintf(char * pcStr, ...);
	void sysprintf(char * pcStr, ...);
	void sysPutChar(char ucCh);
	void sysUartInstallcallback(uint32_t u32IntType, PFN_SYS_UART_CALLBACK pfnCallback);
	void sysUartEnableInt(int32_t eIntType);
	void sysUartTransfer(int8_t * pu8buf, uint32_t u32Len);

/*--------------------------------------------------------------------------*/
/* Define Function Prototypes for HUART                                     */
/*--------------------------------------------------------------------------*/
	int32_t sysInitializeHUART(WB_UART_T * uart);
	void sysHuartInstallcallback(uint32_t u32IntType, PFN_SYS_UART_CALLBACK pfnCallback);
	void sysHuartEnableInt(int32_t eIntType);
	int8_t sysHuartReceive(void);
	void sysHuartTransfer(int8_t * pu8buf, uint32_t u32Len);

/* Define system library AIC functions */
	int32_t sysDisableInterrupt(INT_SOURCE_E eIntNo);
	int32_t sysEnableInterrupt(INT_SOURCE_E eIntNo);
	BOOL sysGetIBitState(void);
	uint32_t sysGetInterruptEnableStatus(void);
	uint32_t sysGetInterruptHighEnableStatus(void);
	void *sysInstallExceptionHandler(int32_t nExceptType, void *pvNewHandler);
	void *sysInstallFiqHandler(void *pvNewISR);	/* Almost is not used, sysInstallISR() can cover the job */
	void *sysInstallIrqHandler(void *pvNewISR);	/* Almost is not used, sysInstallISR() can cover the job */
	void *sysInstallISR(int32_t nIntTypeLevel, INT_SOURCE_E eIntNo, void *pvNewISR);
	int32_t sysSetGlobalInterrupt(int32_t nIntState);
	int32_t sysSetInterruptPriorityLevel(INT_SOURCE_E eIntNo, uint32_t uIntLevel);
	int32_t sysSetInterruptType(INT_SOURCE_E eIntNo, uint32_t uIntSourceType);
	int32_t sysSetLocalInterrupt(int32_t nIntState);
	int32_t sysSetAIC2SWMode(void);

/* Define system library Cache functions */
	void sysDisableCache(void);
	int32_t sysEnableCache(uint32_t uCacheOpMode);
	void sysFlushCache(int32_t nCacheType);
	BOOL sysGetCacheState(void);
	int32_t sysGetCacheMode(void);
	int32_t sysGetSdramSizebyMB(void);
	void sysInvalidCache(void);
	int32_t sysSetCachePages(uint32_t addr, int32_t size, int32_t cache_mode);

/* Define system power management functions */
	int32_t sysPowerDown(uint32_t u32WakeUpSrc);

/* Define system clock functions */
	uint32_t sysGetExternalClock(void);
	uint32_t sysSetPllClock(E_SYS_SRC_CLK eSrcClk, uint32_t u32TargetHz);
	void sysCheckPllConstraint(BOOL bIsCheck);
	int32_t sysSetSystemClock(E_SYS_SRC_CLK eSrcClk, uint32_t u32PllHz, uint32_t u32SysHz);	/* Unit HZ */
	int32_t sysSetCPUClock(uint32_t u32CPUClock);	/* Unit HZ */
	int32_t sysSetAPBClock(uint32_t u32APBClock);	/* Unit HZ */
	uint32_t sysGetPLLOutputHz(E_SYS_SRC_CLK eSysPll, uint32_t u32FinHz);
	uint32_t sysGetSystemClock(void);	/* Unit HZ */
	uint32_t sysGetCPUClock(void);	/* Unit HZ */
	uint32_t sysGetHCLK1Clock(void);	/* Unit HZ */
	uint32_t sysGetAPBClock(void);	/* Unit HZ */

	uint32_t sysSetDramClock(E_SYS_SRC_CLK eSrcClk, uint32_t u32PLLClockHz, uint32_t u32DramClock);	/* Unit HZ */
	uint32_t sysGetDramClock(void);

#ifdef __cplusplus
}
#endif
#endif				/* _WBLIB_H */
