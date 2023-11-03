/****************************************************************************
 * @file     wb_config.c
 * @version  V3.00
 * @brief    N9H26 series SYS driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/****************************************************************************
 *
 * FILENAME : wb_config.c
 *
 * VERSION  : 1.1
 *
 * DESCRIPTION :
 *               PLL control functions of Nuvoton ARM9 MCU
 *
 * HISTORY
 *   2008-06-25  Ver 1.0 draft by Min-Nan Cheng
 * Modification
 *   2011-06-01  Ver 1.1 draft by Shih-Wen Chou
 *
 *      IBR set clocks default value
 *          UPLL= 240MHz
 *          SYS = 120MHz
 *          CPU = 60MHz
 *          HCLK = 60MHz
 *
 *
 *
 *
 ****************************************************************************/
#include <string.h>
#include <stdint.h>
#include <types_common.h>
#include "wblib.h"

//static uint32_t g_u32SysClkSrc;
static uint32_t g_u32UpllHz = 240000000UL;	// g_u32ApllHz=240000000; //g_u32SysHz = 120000000, g_u32CpuHz = 60000000, g_u32HclkHz = 60000000;
//static uint32_t g_u32MpllHz = 180000000UL;
static int32_t /*g_i32REG_APLL, */ g_i32REG_UPLL, g_i32REG_MPLL;
static uint32_t g_u32ExtClk = 12000000UL;

uint32_t sysGetHCLK234Clock(void);

//extern uint8_t _tmp_buf[];
static uint8_t _tmp_buf[PD_RAM_SIZE] __attribute__((aligned(32)));

//#define N9H26K_A_VERSION
#ifdef N9H26K_A_VERSION
#define REG_DLLMODE_R 0xb0003058
#else
#define REG_DLLMODE_R   0xb0003058	/* Not fix in B version. still read in 3058 */
#endif

extern uint32_t u32UartPort;

int bIsFirstReadClkSkew = 1;
uint32_t u32ClkSkewInit = 0;

//#define DBG_PRINTF        sysprintf
#define DBG_PRINTF(...)
/*-----------------------------------------------------------------------------------------------------------
 *
 * Function : sysGetPLLOutputHz
 *
 * DESCRIPTION :
 *               According to the external clock and expected PLL output clock to get the content of PLL controll register
 *
 * Parameters
 *              eSysPll : eSYS_APLL or eSYS_UPLL
 *          u32FinKHz: External clock. Unit: KHz
 * Return
 *          PLL clock.
 * HISTORY
 *               2010-07-15
 *
-----------------------------------------------------------------------------------------------------------*/
uint32_t sysGetPLLOutputHz(E_SYS_SRC_CLK eSysPll, uint32_t u32FinHz)
{
	uint32_t u32PllCntlReg = 0, u32Fout;
	uint32_t NF, NR, NO;
	uint32_t u32NOArray[] = { 1, 2, 4, 8 };

	// 2014/4/30: NandLoader don't need it
//  if(eSysPll==eSYS_APLL)
//      u32PllCntlReg = inp32(REG_APLLCON);
//  else
	if (eSysPll == eSYS_UPLL)
		u32PllCntlReg = inp32(REG_UPLLCON);
	else if (eSysPll == eSYS_MPLL)
		u32PllCntlReg = inp32(REG_MPLLCON);

	if (u32PllCntlReg & 0x10000)	//PLL power down.
		return 0;

	NF = (u32PllCntlReg & 0x7F) << 1;
	NR = (u32PllCntlReg & 0x780) >> 7;
	NO = u32NOArray[((u32PllCntlReg & 0x1800) >> 11)];

	u32Fout = u32FinHz / NO / NR * NF;
	return u32Fout;
}

/*-----------------------------------------------------------------------------------------------------------
 *
 * Function : sysGetPLLControlRegister
 *
 * DESCRIPTION :
 *               According to the external clock and expected PLL output clock to get the content of PLL controll register
 *
 * Parameters
 *              u32FinKHz : External clock.  Unit:KHz
 *          u32TargetKHz: PLL output clock. Unit:KHz
 * Return
 *                  0 : No any fit value for the specified PLL output clock
 *          PLL control register.
 * HISTORY
 *               2011-10-21
 *
-----------------------------------------------------------------------------------------------------------*/

BOOL bIsCheckConstraint = TRUE;
void sysCheckPllConstraint(BOOL bIsCheck)
{
	bIsCheckConstraint = bIsCheck;
}

#define MIN_FBDV_M      4
#define MAX_FBDV_M      256
#define MIN_INDV_N      2
#define MAX_INDV_N      16
int32_t _sysGetPLLControlRegister(uint32_t u32FinKHz, uint32_t u32TargetHz)
{
	uint32_t u32ClkOut;
	uint32_t u32NO;
	uint32_t u32IdxM, u32IdxN;
	int32_t i32IdxNO;
	uint32_t u32NOArray[] = { 1, 2, 4, 8 };
	/* To get little jiter on PLL output, i32IdxNO has better =0x03 or 0x02 */
	for (i32IdxNO = 3; i32IdxNO > 0; i32IdxNO = i32IdxNO - 1) {
		for (u32IdxM = MIN_FBDV_M; u32IdxM < MAX_FBDV_M; u32IdxM = u32IdxM + 1) {
			//u32IdxM=NR >=4. Fedback divider.
			for (u32IdxN = MIN_INDV_N; u32IdxN < MAX_INDV_N; u32IdxN = u32IdxN + 1) {
				//u32IdxN=N >=2. (NR = u32IdxN). Input divider
				if (bIsCheckConstraint == 1) {
					if ((u32FinKHz / u32IdxN) > 50000)	/* 1MHz < FIN/NR < 50MHz */
						continue;
					if ((u32FinKHz / u32IdxN) < 1000)
						continue;
				}
				u32NO = u32NOArray[i32IdxNO];	/* FOUT = (FIN * NF/NR)/NO */
				u32ClkOut = u32FinKHz * u32IdxM / u32IdxN / u32NO;	/* Where NF = u32IdxM,  NR = u32IdxN, NO=u32NOArray[i32IdxNO]. */
				if ((u32ClkOut * 1000) == u32TargetHz) {
					if (bIsCheckConstraint == 1) {
						if ((u32ClkOut * u32NO) < 500000)	/* 500MHz <= FIN/NO < 1500MHz */
							continue;
						if ((u32ClkOut * u32NO) > 1500000)
							continue;
					}
					DBG_PRINTF("\n****************\n");
					DBG_PRINTF("M = 0x%x\n", u32IdxM);
					DBG_PRINTF("N = 0x%x\n", u32IdxN);
					DBG_PRINTF("NO = 0x%x\n", i32IdxNO);
					return ((u32IdxM >> 1) | (u32IdxN << 7) | (i32IdxNO << 11));
				}
			}
		}
	}
	return -1;
}

/*-----------------------------------------------------------------------------------------------------------
* Function: sysSetPLLControlRegister
*
* Parameters:
*              u32PllValue - [in], PLL setting value
*
* Returns:
*      None
*
* Description:
*              To set the PLL control register.
*
-----------------------------------------------------------------------------------------------------------*/
void sysSetPLLControlRegister(E_SYS_SRC_CLK eSysPll, uint32_t u32PllValue)
{
	if (eSysPll == eSYS_APLL)
		outp32(REG_APLLCON, u32PllValue);
	else if (eSysPll == eSYS_APLL)
		outp32(REG_UPLLCON, u32PllValue);
}

/*-----------------------------------------------------------------------------------------------------------
* Function: sysSetSystemClock
*
* Parameters:
*              u32PllValue - [in], PLL setting value
*
* Returns:
*      None
*
* Description:
*              To set the PLL control register.
*
*Note:
*       Switch systetm clock to external clock first.
*
*       refresh rate = REPEAT/Fmclk
*       1. Disable interrupt
*       2. Enter Self-refresh
*       3. Switch to external clock
*       4. Adjustment the sys divider.
*
*
*
*
*
*
-----------------------------------------------------------------------------------------------------------*/
void _sysClockSwitch(register E_SYS_SRC_CLK eSrcClk,
		     register uint32_t u32Hclk,
		     register uint32_t u32PllReg, register uint32_t u32SysDiv)
{
	register int reg2 = 100, reg1 = 0, reg0 = 1;
	uint32_t u32IntTmp;
	/* disable interrupt (I will recovery it after clock changed) */
	u32IntTmp = inp32(REG_AIC_IMR);
	outp32(REG_AIC_MDCR, 0xFFFFFFFE);

	/* DRAM enter self refresh mode */
	outp32(REG_SDCMD, inp32(REG_SDCMD) | (SELF_REF | REF_CMD));

	__asm volatile
	 ("  mov 	%0, #100       \n"
	  "  mov  %1, #0         \n"
	  "  mov  %2, #1         \n"
	  " loop1:	           \n"
	  "  add  %1, %1, %2     \n"
	  "  cmp 	%1, %0         \n"
	  "  bne  loop1          \n"::"r" (reg2), "r"(reg1), "r"(reg0):"memory");
	outp32(REG_UPLLCON, inp32(REG_UPLLCON) | BIT15);

	if (eSrcClk == eSYS_EXT) {
		/* Fill system clock divider */
		outp32(REG_CLKDIV0,
		       (inp32(REG_CLKDIV0) & ~(SYSTEM_N1 | SYSTEM_S | SYSTEM_N0)) | u32SysDiv);
	} else if ((eSrcClk == eSYS_APLL) || (eSrcClk == eSYS_UPLL)) {
		//outp32(REG_CLKDIV0,  (inp32(REG_CLKDIV0) | 0x01));    //PLL/3 Safe consider
		/* system clock always comes from UPLL */
		outp32(REG_UPLLCON, u32PllReg | BIT15);

		__asm volatile
		 ("  mov 	%0, #1000      \n"
		  "  mov  %1, #0         \n"
		  "  mov  %2, #1         \n"
		  " loop1a:	           \n"
		  "  add  %1, %1, %2     \n"
		  "  cmp 	%1, %0         \n"
		  "  bne  loop1a         \n"::"r" (reg2), "r"(reg1), "r"(reg0):"memory");
		/* Fill system clock divider */
		outp32(REG_CLKDIV0, (inp32(REG_CLKDIV0) & ~(SYSTEM_N1 | SYSTEM_S | SYSTEM_N0)) |
		       u32SysDiv);
	}
	__asm volatile
	 ("  mov 	%0, #200       \n"
	  "  mov  %1, #0         \n"
	  "  mov  %2, #1         \n"
	  " loop2:	           \n"
	  "  add  %1, %1, %2     \n"
	  "  cmp 	%1, %0         \n"
	  "  bne  loop2          \n"::"r" (reg2), "r"(reg1), "r"(reg0):"memory");
	outp32(REG_UPLLCON, inp32(REG_UPLLCON) & ~BIT15);

	/* DRAM escape self refresh mode */
	//outp32(REG_SDCMD, inp32(REG_SDCMD) & ~REF_CMD);
	outp32(0xB0003004, 0x20);

	__asm volatile
	 ("  mov 	%0, #600      \n"
	  "  mov  %1, #0         \n"
	  "  mov  %2, #1         \n"
	  " loop3a:	           \n"
	  "  add  %1, %1, %2     \n"
	  "  cmp 	%1, %0         \n"
	  "  bne  loop3a         \n"::"r" (reg2), "r"(reg1), "r"(reg0):"memory");
	outp32(REG_AIC_MECR, u32IntTmp);

}

void _sysClockSwitchStart(E_SYS_SRC_CLK eSrcClk,
			  uint32_t u32Hclk, uint32_t u32RegPll, uint32_t u32SysDiv)
{
	uint32_t vram_base, aic_status = 0, aic_statush = 0;
	int bIsCacheState = 0;
	uint32_t u32CacheMode = 0;
	void (*wb_func)(E_SYS_SRC_CLK, uint32_t, uint32_t, uint32_t);

	aic_status = inpw(REG_AIC_IMR);	//Disable interrupt
	aic_statush = inpw(REG_AIC_IMRH);	//Disable interrupt
	outpw(REG_AIC_MDCR, 0xFFFFFFFF);
	outpw(REG_AIC_MDCRH, 0xFFFFFFFF);

	vram_base = PD_RAM_BASE;
	memcpy((void *)((uint32_t) vram_base | 0x80000000), (void *)((uint32_t) _sysClockSwitch | 0x80000000), PD_RAM_SIZE);	//

	if (sysGetCacheState() == 1) {
		DBG_PRINTF("Cache enable\n");
		bIsCacheState = 1;
		u32CacheMode = sysGetCacheMode();
		sysDisableCache();
		sysFlushCache(I_D_CACHE);
	} else {
		DBG_PRINTF("Cache disable\n");
	}

	wb_func = (void (*)(E_SYS_SRC_CLK, uint32_t,	//u32Hclk
			    uint32_t, uint32_t))vram_base;

	DBG_PRINTF("SYS_DIV = %x\n", u32SysDiv);
//  DBG_PRINTF("CPU_DIV = %x\n", u32CpuDiv);
//  DBG_PRINTF("APB_DIV = %x\n", u32ApbDiv);
	DBG_PRINTF("Jump to SRAM\n");
	wb_func(eSrcClk, u32Hclk, u32RegPll, u32SysDiv);

	if (bIsCacheState == 1)
		sysEnableCache(u32CacheMode);

	DBG_PRINTF("Calibration Value = 0x%x\n", inp32(REG_SDOPM));
	//Restore VRAM
	outpw(REG_AIC_MECR, aic_status);	// Restore AIC setting
	outpw(REG_AIC_MECRH, aic_statush);	// Restore AIC setting
}

/*-----------------------------------------------------------------------------------------------------------
* Function: sysSetSystemClock
*
* Parameters:
*              u32PllHz         - [in], Specified PLL Clock
*              u32SysHz     - [in], Specified SYS Clock
* Returns:
*      Error Code
*
* Description:
*              To set the PLL clock and system clock
*
-----------------------------------------------------------------------------------------------------------*/
int32_t sysSetSystemClock(E_SYS_SRC_CLK eSrcClk,	// Specified the system clock come from external clock, APLL or UPLL
			  uint32_t u32PllHz,	// Specified the APLL/UPLL clock
			  uint32_t u32SysHz	// Specified the system clock
    )
{
	uint32_t u32RegPll;
	uint32_t u32RegSysDiv;
	//uint32_t u32CpuFreq, u32Hclk1Frq, u32MclkClock, u32Hclk234;
	uint32_t u32SysDiv, u32DivN0, u32DivN1;

	// 2014/4/30: NandLoader don't need it
	//uint32_t u32DramSrc = (inp32(REG_CLKDIV7)&DRAM_S)>>3;

	g_u32ExtClk = sysGetExternalClock();

	u32SysDiv = u32PllHz / u32SysHz;
	for (u32DivN1 = 1; u32DivN1 <= 16; u32DivN1 = u32DivN1 + 1) {
		for (u32DivN0 = 1; u32DivN0 <= 8; u32DivN0 = u32DivN0 + 1) {
			if (u32SysDiv == u32DivN1 * u32DivN0)
				break;
		}
		if (u32DivN0 >= 9)
			continue;
		if (u32SysDiv == u32DivN1 * u32DivN0)
			break;
	}
// 2019/9/19: NandLoader don't need it
//  if(u32DivN1>=17){
//      sysprintf("Can not set the clock due to divider is %d\n", u32SysDiv);
//      return (int32_t)WB_INVALID_CLOCK;
//  }
	if (u32DivN0 != 0)
		u32DivN0 = u32DivN0 - 1;
	if (u32DivN1 != 0)
		u32DivN1 = u32DivN1 - 1;

	switch (eSrcClk) {
	case eSYS_UPLL:
		//g_u32SysClkSrc = eSYS_UPLL;
		g_u32UpllHz = u32PllHz;
		g_i32REG_UPLL = _sysGetPLLControlRegister((g_u32ExtClk / 1000), g_u32UpllHz);
		//printf("UPLL register = %d\n", g_i32REG_UPLL);
		break;
	default:
		return (int32_t) WB_INVALID_CLOCK;
	}
	if (eSrcClk == eSYS_UPLL) {
		u32RegPll = g_i32REG_UPLL;
		DBG_PRINTF("UPLL  = %d\n", u32RegPll);
		outp32(REG_UPLLCON, u32RegPll);
	}

	u32RegSysDiv =
	    (inp32(REG_CLKDIV0) & ~(SYSTEM_N1 | SYSTEM_S | SYSTEM_N0)) | (((u32DivN1 & 0x0F) << 8) |
									  (eSrcClk << 3) |
									  (u32DivN0));
	DBG_PRINTF("REG_PLL = 0x%x\n", u32RegPll);
	DBG_PRINTF("REG_CLKDIV0 = 0x%x\n", u32RegSysDiv);
	outp32(REG_CLKDIV0,
	       (inp32(REG_CLKDIV0) & ~(SYSTEM_N1 | SYSTEM_S | SYSTEM_N0)) | u32RegSysDiv);
	/* Restore LVR */
//  outp32(REG_POR_LVRD, u32RegLVR);

	return Successful;
}

/*
    The function will set the CPU clock below the specified CPU clock
    And return the real clock of CPU.
    !!! Assume change CPU clock is workable in SDRAM!!!
*/
int32_t sysSetCPUClock(uint32_t u32CPUClock)
{
	//--- for Loader, the u32CPUClock should equal to u32SysClock. Simplify this API to shrink code size.
	uint32_t u32CPUDiv;
	uint32_t u32SysClock = sysGetSystemClock();
	u32CPUDiv = 0;
	outp32(REG_CLKDIV4, (inp32(REG_CLKDIV4) & ~CPU_N) | (u32CPUDiv | CHG_APB));
	return u32SysClock;
}

/*
     HCLK1 clcok is always equal to CPUCLK or CPUCLK/2 depends on CPU_N
     int32_t sysSetHCLK1Clock(uint32_t u32HCLK1Clock)
    {

    }
*/

/*
    Set APB clcok
*/
int32_t sysSetAPBClock(uint32_t u32APBClock)
{
	uint32_t u32APBDiv;
	uint32_t u32HCLK1Clock;
	u32HCLK1Clock = sysGetHCLK1Clock();
	u32APBDiv = (u32HCLK1Clock / u32APBClock) - 1;
	if ((u32HCLK1Clock % u32APBClock) != 0)
		u32APBDiv = u32APBDiv + 1;

	outp32(REG_CLKDIV4, (inp32(REG_CLKDIV4) & ~APB_N) |
	       //(u32APBDiv<<8)); /* N9H26K's program */
	       ((u32APBDiv << 8) | CHG_APB));	/* CHG_APB: Enable change APB clock */
	/* CHG_APB will auto clear */
	return (u32HCLK1Clock / (u32APBDiv + 1));
}

uint32_t sysGetExternalClock(void)
{
	g_u32ExtClk = EXTERNAL_CRYSTAL_CLOCK;
	return g_u32ExtClk;
}

/*
    Get system clcok
*/
uint32_t sysGetSystemClock(void)
{
	uint32_t u32Fin;
	uint32_t u32SysSrc, u32PllPreDiv = 1;
	uint32_t u32SysN1, u32SysN0;
	u32Fin = sysGetExternalClock();
	u32SysSrc = (inp32(REG_CLKDIV0) & SYSTEM_S) >> 3;
	//u32PllPreDiv = (inp32(REG_CLKDIV0) & SYSTEM_N0)+1;
	switch (u32SysSrc) {
	case 0:
		u32SysSrc = u32Fin;
		break;
	case 2:
		u32SysSrc = sysGetPLLOutputHz(eSYS_APLL, u32Fin) / u32PllPreDiv;
		break;
	case 3:
		u32SysSrc = sysGetPLLOutputHz(eSYS_UPLL, u32Fin) / u32PllPreDiv;
		break;
	}

	u32SysN1 = ((inp32(REG_CLKDIV0) & SYSTEM_N1) >> 8) + 1;
	u32SysN0 = (inp32(REG_CLKDIV0) & SYSTEM_N0) + 1;

	return (u32SysSrc / (u32SysN1 * u32SysN0));
}

/*
    Get CPU clcok
*/
uint32_t sysGetCPUClock()
{
	uint32_t u32SysClock = sysGetSystemClock();
	uint32_t CPUClock;
	CPUClock = u32SysClock / ((inp32(REG_CLKDIV4) & CPU_N) + 1);
	return (uint32_t) CPUClock;
}

/*
    Get HCLK1 clcok
*/
uint32_t sysGetHCLK1Clock()
{
	uint32_t u32CPUClock;
	uint32_t u32CPUDiv;
	u32CPUClock = sysGetCPUClock();
	u32CPUDiv = inp32(REG_CLKDIV4) & CPU_N;
	if (u32CPUDiv == 0)
		return u32CPUClock / 2;
	else
		return u32CPUClock;
}

/*
    Get HCLK234 clcok
*/
uint32_t sysGetHCLK234Clock(void)
{
	uint32_t u32HCLK1Clock;
	uint32_t u32HCLK234Div;

	u32HCLK1Clock = sysGetHCLK1Clock();
	u32HCLK234Div = (inp32(REG_CLKDIV4) & HCLK234_N) >> 4;
	return (u32HCLK1Clock / (u32HCLK234Div + 1));
}

/*
    Get APB clcok
*/
uint32_t sysGetAPBClock()
{
	uint32_t u32APBDiv;
	u32APBDiv = ((inp32(REG_CLKDIV4) & APB_N) >> 8) + 1;
	return (sysGetHCLK1Clock() / u32APBDiv);
}

/*-----------------------------------------------------------------------------------------------------------
*   The Function is used to set the other PLL which is not the system clock source.
*   If system clock source come from eSYS_UPLL. The eSrcClk only can be eSYS_APLL
*   And if specified PLL not meet some costraint, the funtion will search the near frequency and not over the specified frequency
*
*   Paramter:
*       eSrcClk: eSYS_UPLL or eSYS_APLL
*       u32TargetKHz: The specified frequency. Unit:Khz.
*
*   Return:
*       The specified PLL output frequency really.
-----------------------------------------------------------------------------------------------------------*/
uint32_t sysSetPllClock(E_SYS_SRC_CLK eSrcClk, uint32_t u32TargetHz)
{
	uint32_t u32PllReg, /*u32PllOutFreqHz, */ u32FinHz;

	u32FinHz = sysGetExternalClock();

	u32PllReg = _sysGetPLLControlRegister((u32FinHz / 1000), u32TargetHz);
	outp32(REG_APLLCON, u32PllReg);
	return 0;		// NandLoader don't use return value.
}

//==========================================================================================

/*-----------------------------------------------------------------------------------------------------------
* Function: sysSetMPLLClock
*
* Parameters:
*              u32PllValue - [in], PLL setting value
*              u32DramClock - [in], DRAM working frequency
* Returns:
*      Successful or Error Code
*
* Description:
*              To set the MPLL control register and DRAM working requency.
*
*Note:
*       Switch systetm clock to external clock first.
*
*       refresh rate = REPEAT/Fmclk
*       1. Disable interrupt
*       2. Enter Self-refresh
*       3. Switch to external clock
*       4. Adjustment the sys divider.
*
*
*
*
*
*
-----------------------------------------------------------------------------------------------------------*/
#if 0
#define dbg(u32LocalUartVar, x)  \
                                while (!(inpw(REG_UART_FSR+u32LocalUartVar) & 0x400000));\
                                outpb(REG_UART_THR+u32LocalUartVar, x);\
                                while (!(inpw(REG_UART_FSR+u32LocalUartVar) & 0x400000));\
                                outpb(REG_UART_THR+u32LocalUartVar, '\n');\
                                while (!(inpw(REG_UART_FSR+u32LocalUartVar) & 0x400000));
#define dbg_woc(u32LocalUartVar, x) \
                                while (!(inpw(REG_UART_FSR+u32LocalUartVar) & 0x400000));\
                                outpb(REG_UART_THR+u32LocalUartVar, x);\
                                while (!(inpw(REG_UART_FSR+u32LocalUartVar) & 0x400000));
#else
#define dbg(...)
#define dbg_woc(...)
#endif

#define SRAM_MEMCONTENT         0xFF001FD0
#define SRAM_MEMTYPE            0xFF001FD4
#define SRAM_SKEW               0xFF001FD8
#define SRAM_SRCClk             0xFF001FE0
#define SRAM_PLLREG             0xFF001FE4
#define SRAM_DRAMFREQ           0xFF001FE8
#define SRAM_DRAMDIVI           0xFF001FEC
#define SRAM_REG_CLKDIV0        0xFF001FF0
#define SRAM_HIGH_FREQ          0xFF001FF4
#define SRAM_UARTPORT           0xFF001FF8
#define SRAM_VAR                0xFF001FFC

void _dramClockSwitch(register E_SYS_SRC_CLK eSrcClk,
		      register uint32_t u32PllReg,
		      register uint32_t u32DramFreq, register uint32_t u32DramClkDiv)
{

	register int reg2 = 0x200, reg1 = 0, reg0 = 1;
	uint32_t u32mem_1aaaa8;
	uint32_t u32mem_1fffff0;
	//uint32_t u32REG_CLKDIV0, High_Freq;

	//uint32_t dly;
	//register uint32_t u32LocalUartVar = u32UartPort;

	//outp32(SRAM_CHIP_TYPE, (inp32(REG_CHIPCFG)&SDRAMSEL)>>4);
	outp32(SRAM_SRCClk, eSrcClk);
	outp32(SRAM_PLLREG, u32PllReg);
	outp32(SRAM_DRAMFREQ, u32DramFreq);
	outp32(SRAM_DRAMDIVI, u32DramClkDiv);
	outp32(SRAM_UARTPORT, u32UartPort);
	u32mem_1aaaa8 = inp32(0x1aaaa8);	/* Back up content in address 0x1aaaa8 */
	outp32(0x1aaaa8, 0x5555aaaa);
	u32mem_1fffff0 = inp32(0x1FFFFF0);

	dbg(inp32(SRAM_UARTPORT), '0' + ((inp32(REG_CHIPCFG) & SDRAMSEL) >> 4));
	outp32(SRAM_MEMTYPE, ((inp32(REG_CHIPCFG) & SDRAMSEL) >> 4));
	// Very important: Disable chip DLL first with 100us delay for calibration stable **********
	outp32(REG_DLLMODE, (inp32(REG_DLLMODE_R) & ~0x8) | 0x10);	// Disable DLL of chip

	//for(dly=0; dly<0x2800;dly++);
	__asm volatile
	 ("  mov 	%0, #0x200     \n"
	  "  mov  %1, #0         \n"
	  "  mov  %2, #1         \n"
	  " DRAM_A:	           \n"
	  "  add  %1, %1, %2     \n"
	  "  cmp 	%1, %0         \n"
	  "  bne  DRAM_A         \n"::"r" (reg2), "r"(reg1), "r"(reg0):"memory");

	dbg(inp32(SRAM_UARTPORT), '0' + (inp32(SRAM_DRAMDIVI) & 0x7));
	outp32(SRAM_REG_CLKDIV0, inp32(REG_CLKDIV0));

    /*********** DRAM enter self refresh mode ************/
	outp32(REG_SDCMD, (inp32(REG_SDCMD) & ~0x20) | 0x10);
    /*********** DRAM enter self refresh mode ************/

	__asm volatile
	 ("  mov 	%0, #0x100    \n"
	  "  mov  %1, #0         \n"
	  "  mov  %2, #1         \n"
	  " DRAM_S0:	           \n"
	  "  add  %1, %1, %2     \n"
	  "  cmp 	%1, %0         \n"
	  "  bne  DRAM_S0        \n"::"r" (reg2), "r"(reg1), "r"(reg0):"memory");

	if (inp32(SRAM_DRAMFREQ) >= 96000000) {
		dbg(inp32(SRAM_UARTPORT), 'H');
		//while(1)
		//dbg(inp32(SRAM_UARTPORT), 'H');
	} else {
		dbg(inp32(SRAM_UARTPORT), 'L');
	}

	outp32(REG_DLLMODE, (inp32(REG_DLLMODE_R) & ~0x8) | 0x10);	// Disable chip's DLL

	/* Switch system clock to external clock and divider to 0 *//* Due to delay time. Switch to external clock for delay loop */
	outp32(REG_CLKDIV0, (inp32(REG_CLKDIV0) & ~(SYSTEM_N1 | SYSTEM_S | SYSTEM_N0)));	//HCLK = 6MHz.

	/* Switch DRAM clock to external clock and divider to 0 */
	outp32(REG_CLKDIV7, (inp32(REG_CLKDIV7) & ~(DRAM_N1 | DRAM_S | DRAM_N0)));	//DRAM = 6MHz.

	//Set Pll clock
	if (eSrcClk == eSYS_MPLL) {
		outp32(REG_MPLLCON, u32PllReg);
		while ((inp32(REG_POR_LVRD) & MPLL_LKDT) == 0) ;
	} else if (eSrcClk == eSYS_UPLL) {
		outp32(REG_UPLLCON, u32PllReg);
		while ((inp32(REG_POR_LVRD) & UPLL_LKDT) == 0) ;
	}

	//Set DRAM clock divider and source
	outp32(REG_CLKDIV7, u32DramClkDiv);
	__asm volatile
	 ("  mov 	%0, #1000      \n"
	  "  mov  %1, #0         \n"
	  "  mov  %2, #1         \n"
	  " DRAM_S1:	           \n"
	  "  add  %1, %1, %2     \n"
	  "  cmp 	%1, %0         \n"
	  "  bne  DRAM_S1        \n"::"r" (reg2), "r"(reg1), "r"(reg0):"memory");
	outp32(REG_CLKDIV0, inp32(SRAM_REG_CLKDIV0));

    /*********** DRAM escape self refresh mode ************/
	outp32(REG_SDCMD, 0x20);
    /*********** DRAM escape self refresh mode ************/

	__asm volatile
	 ("  mov 	%0, #2000     \n"
	  "  mov  %1, #0         \n"
	  "  mov  %2, #1         \n"
	  " DRAM1a:	           \n"
	  "  add  %1, %1, %2     \n"
	  "  cmp 	%1, %0         \n"
	  "  bne  DRAM1a         \n"::"r" (reg2), "r"(reg1), "r"(reg0):"memory");

	dbg(inp32(SRAM_UARTPORT), '0' + ((inp32(REG_CHIPCFG) & SDRAMSEL) >> 4));

	//dly = (inp32(REG_CHIPCFG)&SDRAMSEL)>>4;    /* SDRAM has escaped self-refresh mode */
	//dbg(inp32(SRAM_UARTPORT), '0'+dly);

	if (((inp32(REG_CHIPCFG) & SDRAMSEL) >> 4) == 2)
		//High_Freq = 64000000;
		outp32(SRAM_HIGH_FREQ, 64000000);
	else
		//High_Freq = 96000000;
		outp32(SRAM_HIGH_FREQ, 96000000);

	if (inp32(SRAM_DRAMFREQ) >= 96000000) {
		dbg_woc(inp32(SRAM_UARTPORT), '+');
	} else {
		dbg_woc(inp32(SRAM_UARTPORT), '-');
	}
	//DRAM auto-calibration for optimal DRAM Phase
	if (inp32(SRAM_DRAMFREQ) >= inp32(SRAM_HIGH_FREQ)) {
		//High Frequency
		outp32(REG_SDEMR, inp32(REG_SDEMR) & ~DLLEN);	// Enable  DLL of DDR2
		outp32(REG_SDMR, 0x532);	// RESET DLL(bit[8]) of DDR2

		//for(dly=0; dly<50;dly++);

		__asm volatile
		 ("  mov 	%0, #50    \n"
		  "  mov  %1, #0         \n"
		  "  mov  %2, #1         \n"
		  " DRAMHA:	           \n"
		  "  add  %1, %1, %2     \n"
		  "  cmp 	%1, %0         \n"
		  "  bne  DRAMHA         \n"::"r" (reg2), "r"(reg1), "r"(reg0):"memory");

		outp32(REG_SDMR, 0x432);	// RESET DLL(bit[8]) of DDR2
 again:
		outp32(REG_DLLMODE, (inp32(REG_DLLMODE_R) & ~0x8) | 0x10);	// Disable chip's DLL
		outp32(REG_DLLMODE, inp32(REG_DLLMODE_R) | 0x18);	// Enable  chip's DLL
		outp32(REG_CKDQSDS, 0x00888800);	// Skew for high freq

		//change = 0;
		//for(i=0; i<7; i=i+1)
		outp32(SRAM_VAR, 0);
		while (inp32(SRAM_VAR) < 7) {
			//outp32(REG_DLLMODE, (0x19+i) );              //DLLMODE phase search
			outp32(REG_DLLMODE, (0x19 + inp32(SRAM_VAR)));	//DLLMODE phase search
			__asm volatile
			 ("  mov 	%0, #0x2000    \n"
			  "  mov  %1, #0         \n"
			  "  mov  %2, #1         \n"
			  " DRAMHB:	           \n"
			  "  add  %1, %1, %2     \n"
			  "  cmp 	%1, %0         \n"
			  "  bne  DRAMHB         \n"::"r" (reg2), "r"(reg1), "r"(reg0):"memory");
			outp32(0x1FFFFF0, 0);
			if (inp32(SRAM_MEMTYPE) == 3) {
				//DDR2 type
				//dbg_woc(inp32(SRAM_UARTPORT), 'A');
				outp32(REG_SDOPM, 0x01130476);	//Set DQS_PHASE_RST
				outp32(REG_SDOPM, 0x01030476);	//Clr DQS_PHASE_RST
			} else if (inp32(SRAM_MEMTYPE) == 2) {
				//DDR type
				//dbg_woc(inp32(SRAM_UARTPORT), 'B');
				outp32(REG_SDOPM, 0x01130456);	//Set DQS_PHASE_RST
				outp32(REG_SDOPM, 0x01030456);	//Clr DQS_PHASE_RST
			} else if (inp32(SRAM_MEMTYPE) == 0) {
				//DDR type
				//dbg_woc(inp32(SRAM_UARTPORT), 'C');
				outp32(REG_SDOPM, 0x01130416);	//Set DQS_PHASE_RST
				outp32(REG_SDOPM, 0x01030416);	//Clr DQS_PHASE_RST
			} else {
				dbg(inp32(SRAM_UARTPORT), 'D');
			}
			//tmp = inp32(0x1aaaa8);                   //Dummy Read DRAM
			outp32(SRAM_MEMCONTENT, inp32(0x1aaaa8));

			//skew = skew <<1;
			outp32(SRAM_SKEW, inp32(SRAM_SKEW) << 1);
			switch (inp32(SRAM_VAR)) {
				//case 0:       skew = (inp32(REG_SDOPM) & 0x10000000)!=0 ;    break;
				//case 1:     skew = skew | ((inp32(REG_SDOPM) & 0x10000000)!=0);    break;
				//case 2:     skew = skew | ((inp32(REG_SDOPM) & 0x10000000)!=0);    break;
				//case 3:     skew = skew | ((inp32(REG_SDOPM) & 0x10000000)!=0);    break;
				//case 4:     skew = skew | ((inp32(REG_SDOPM) & 0x10000000)!=0);    break;
				//case 5:     skew = skew | ((inp32(REG_SDOPM) & 0x10000000)!=0);    break;
				//case 6:     skew = skew | ((inp32(REG_SDOPM) & 0x10000000)!=0);    break;
			case 0:
				outp32(SRAM_SKEW, ((inp32(REG_SDOPM) & 0x10000000) != 0));
				break;
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
				outp32(SRAM_SKEW,
				       inp32(SRAM_SKEW) | ((inp32(REG_SDOPM) & 0x10000000) != 0));
				break;
				//skew = skew | ((inp32(REG_SDOPM) & 0x10000000)!=0);    break;
			}
			outp32(SRAM_VAR, inp32(SRAM_VAR) + 1);
		}		//for(i=0; i<7; i=i+1)//

		/*
		   //Choose final DLLMODE value and re-calibration
		   if( (skew_19==skew_1a) &&  (skew_1a==skew_1b)  && (skew_1b==skew_1c) && (skew_1c==skew_1d) ) {
		   //////////// New modification ////////////////////////////
		   if( ((skew_1d!=skew_1e)  || (skew_1e!=skew_1f) )  && ((inp32(REG_SDOPM)>>28)>=3) ){ //>= 360M & SS case
		   //                      outp32(REG_DLLMODE, 0x1f);
		   change = 5;
		   }
		   else
		   outp32(REG_DLLMODE, 0x1a);
		   }
		   else if(                   (skew_1a==skew_1b)  && (skew_1b==skew_1c) && (skew_1c==skew_1d) )
		   {
		   outp32(REG_DLLMODE, 0x1b);
		   change = 1;
		   }
		   else if(                                          (skew_1b==skew_1c) && (skew_1c==skew_1d) )
		   {   //00111 or 11000
		   outp32(REG_DLLMODE, 0x1c);
		   change = 2;
		   }
		   else if(                                                                 skew_1c==skew_1d )
		   {
		   outp32(REG_DLLMODE, 0x1d);
		   change = 3;
		   }
		   else if(                                                                skew_1c!=skew_1d )
		   {
		   outp32(REG_DLLMODE, 0x1e);
		   change = 4;
		   }
		   else
		   outp32(REG_DLLMODE, 0x1a);
		 */
		//for(i=6; i>=0; i=i-1)
		outp32(SRAM_VAR, 6);
		while ((int32_t) (inp32(SRAM_VAR)) >= 0) {
			//dly = (skew >>i)&0x01;
			//dbg_woc(inp32(SRAM_UARTPORT), '0'+dly);
			dbg_woc(inp32(SRAM_UARTPORT),
				'0' + ((inp32(SRAM_SKEW) >> inp32(SRAM_VAR)) & 0x01));
			outp32(SRAM_VAR, inp32(SRAM_VAR) - 1);
		}
		__asm volatile
		 ("  mov 	%0, #0x2800    \n"
		  "  mov  %1, #0         \n"
		  "  mov  %2, #1         \n"
		  " DRAMHC:	           \n"
		  "  add  %1, %1, %2     \n"
		  "  cmp 	%1, %0         \n"
		  "  bne  DRAMHC         \n"::"r" (reg2), "r"(reg1), "r"(reg0):"memory");

		//dly = ((inp32(REG_CHIPCFG)&SDRAMSEL)>>4);
		if (inp32(SRAM_MEMTYPE) == 3) {
			//DDR2 type
			outp32(REG_SDOPM, 0x01130476);	//DQS_PHASE_RST
			outp32(REG_SDOPM, 0x01030476);
		} else if (inp32(SRAM_MEMTYPE) == 2) {
			//DDR type
			outp32(REG_SDOPM, 0x01130456);	//DQS_PHASE_RST
			outp32(REG_SDOPM, 0x01030456);
		} else if (inp32(SRAM_MEMTYPE) == 0) {
			//SDRAM type
			outp32(REG_SDOPM, 0x01130416);	//DQS_PHASE_RST
			outp32(REG_SDOPM, 0x01030416);
		} else {

		}

		//tmp = inp32(0x1aaaa8);                        //Dummy Read DRAM
		outp32(SRAM_MEMCONTENT, inp32(0x1aaaa8));

		dbg_woc(inp32(SRAM_UARTPORT), '\n');

		//while (!(inpw(REG_UART_FSR+u32LocalUartVar) & 0x400000));
		//outpb(REG_UART_THR+u32LocalUartVar, (inp32(REG_SDOPM)>>28)+48);   //Phase
		dbg_woc(inp32(SRAM_UARTPORT), '0' + (inp32(REG_SDOPM) >> 28));	//Phase

//Bon           outp32(REG_DLLMODE, (inp32(REG_DLLMODE_R) - change) | 0x10 );  //Real DLL phase
		outp32(REG_DLLMODE, 0x1a);	//Real DLL phase
		__asm volatile
		 ("  mov 	%0, #0x4000    \n"
		  "  mov  %1, #0         \n"
		  "  mov  %2, #1         \n"
		  " DRAMHD:	           \n"
		  "  add  %1, %1, %2     \n"
		  "  cmp 	%1, %0         \n"
		  "  bne  DRAMHD         \n"::"r" (reg2), "r"(reg1), "r"(reg0):"memory");
		//tmp = inp32(0x1aaaa8);                        //Read DRAM
		outp32(SRAM_MEMCONTENT, inp32(0x1aaaa8));

		//while (!(inpw(REG_UART_FSR+u32LocalUartVar) & 0x400000));
		//outpb(REG_UART_THR+u32LocalUartVar, '.');
		dbg_woc(inp32(SRAM_UARTPORT), '.');

		//while (!(inpw(REG_UART_FSR+u32LocalUartVar) & 0x400000));
		//outpb(REG_UART_THR+u32LocalUartVar, inp32(REG_DLLMODE_R)+39);
		dbg_woc(inp32(SRAM_UARTPORT), inp32(REG_DLLMODE_R) + 39);

		//while (!(inpw(REG_UART_FSR+u32LocalUartVar) & 0x400000));
		if (inp32(SRAM_MEMCONTENT) == 0x5555aaaa) {
			//outpb(REG_UART_THR+u32LocalUartVar, 'p');
			dbg_woc(inp32(SRAM_UARTPORT), 'p');
		} else {
			//outpb(REG_UART_THR+u32LocalUartVar, 'f');
			dbg_woc(inp32(SRAM_UARTPORT), 'f');
		}
		//while (!(inpw(REG_UART_FSR+u32LocalUartVar) & 0x400000));
		//outpb(REG_UART_THR+u32LocalUartVar,(inp32(REG_SDOPM)>>28)+48);    //Final Phase
		dbg_woc(inp32(SRAM_UARTPORT), (inp32(REG_SDOPM) >> 28) + '0');

		//while (!(inpw(REG_UART_FSR+u32LocalUartVar) & 0x400000));
		//outpb(REG_UART_THR+u32LocalUartVar, ':');
		dbg_woc(inp32(SRAM_UARTPORT), ':');

		//if(tmp != 0x5555aaaa)
		if (inp32(SRAM_MEMCONTENT) != 0x5555aaaa)
			goto again;

		//dly = (inp32(REG_CHIPCFG)&SDRAMSEL)>>4;
		//if(u32DramFreq>=180000000)
		if (inp32(SRAM_DRAMFREQ) >= 180000000) {
			if (inp32(SRAM_MEMTYPE) == 3) {
				//DDR2 type
				outp32(REG_SDOPM, 0x0103046E);	// Enable open page
			} else if (inp32(SRAM_MEMTYPE) == 2) {
				//DDR type
				outp32(REG_SDOPM, 0x0103044E);	// Enable open page
			} else if (inp32(SRAM_MEMTYPE) == 0) {
				//SDRAM type
				outp32(REG_SDOPM, 0x0103040E);	// Enable open page
			}
		} else {
			if (inp32(SRAM_MEMTYPE) == 3) {
				//DDR2 type
				outp32(REG_SDOPM, 0x01030476);	// Disable open page
			} else if (inp32(SRAM_MEMTYPE) == 2) {
				//DDR type
				outp32(REG_SDOPM, 0x01030456);	// Disable open page
			} else if (inp32(SRAM_MEMTYPE) == 0) {
				//SDRAM type
				outp32(REG_SDOPM, 0x01030416);	// Disable open page
			}
		}
		if (inp32(SRAM_MEMTYPE) == 2)	//DDR
			outp32(REG_SDMR, (inp32(REG_SDMR) & (~0xF0)) | 0x30);	//CL = 3;

	} else			//Low freq. mode setting
	{
		dbg_woc(inp32(SRAM_UARTPORT), 'V');
		outp32(REG_SDEMR, inp32(REG_SDEMR) | DLLEN);	//Disable DLL of DRAM device
		__asm volatile
		 ("  mov 	%0, #0x2800    \n"
		  "  mov  %1, #0         \n"
		  "  mov  %2, #1         \n"
		  " DRAM_L:	           \n"
		  "  add  %1, %1, %2     \n"
		  "  cmp 	%1, %0         \n"
		  "  bne  DRAM_L         \n"::"r" (reg2), "r"(reg1), "r"(reg0):"memory");

		outp32(REG_DLLMODE, ((inp32(REG_DLLMODE_R) & ~0x18)));	//Disable chip's DLL
		if (inp32(SRAM_MEMTYPE) == 3) {
			//DDR2 type
			outp32(REG_SDOPM, 0x00078476);	//bit[24]=0, bit[18]&[15]=1  and bit[4]=0 (SEL_USE_DLL)
		} else if (inp32(SRAM_MEMTYPE) == 2) {
			//DDR type
			outp32(REG_SDOPM, 0x00078456);	//bit[24]=0, bit[18]&[15]=1  and bit[4]=0 (SEL_USE_DLL)
			outp32(REG_SDMR, (inp32(REG_SDMR) & (~0xF0)) | 0x20);	//CL=2
		} else if (inp32(SRAM_MEMTYPE) == 0) {
			//SDRAM type
			outp32(REG_SDOPM, 0x00078416);	//bit[24]=0, bit[18]&[15]=1  and bit[4]=0 (SEL_USE_DLL)
		}
		outp32(REG_CKDQSDS, 0x0000ff00);	// Skew for low freq
		__asm volatile
		 ("  mov 	%0, #2000    \n"
		  "  mov  %1, #0         \n"
		  "  mov  %2, #1         \n"
		  " DRAM1B:	           \n"
		  "  add  %1, %1, %2     \n"
		  "  cmp 	%1, %0         \n"
		  "  bne  DRAM1B         \n"::"r" (reg2), "r"(reg1), "r"(reg0):"memory");
//Zentel_mode:
		//tmp = inp32(0x1aaaa8);
		outp32(SRAM_MEMCONTENT, inp32(0x1aaaa8));
		//if( tmp == 0x5555aaaa )
		if (inp32(SRAM_MEMCONTENT) == 0x5555aaaa) {
			//outpb(REG_UART_THR+u32LocalUartVar, 'W');
			//while (!(inpw(REG_UART_FSR+u32LocalUartVar) & 0x400000));
			//outpb(REG_UART_THR+u32LocalUartVar, '\n');
			dbg(inp32(SRAM_UARTPORT), 'W');

		} else {
			//outpb(REG_UART_THR+u32LocalUartVar, 'Z');
			//while (!(inpw(REG_UART_FSR+u32LocalUartVar) & 0x400000));
			//outpb(REG_UART_THR+u32LocalUartVar, '\n');
			dbg(inp32(SRAM_UARTPORT), 'Z');
			outp32(REG_CKDQSDS, inp32(REG_CKDQSDS) | 0x01000000);	// bit[25:24] = 1 for Zentel 3T+4ns.
		}
	}
	outp32(0x1aaaa8, u32mem_1aaaa8);	/* Restore content in address 0x1aaaa8 */
	outp32(0x1FFFFF0, u32mem_1fffff0);	/* Restore content in address 0x1fffff0 */
}

void _dramClockSwitchStart(E_SYS_SRC_CLK eSrcClk,
			   uint32_t u32RegPll, uint32_t u32DramClock, uint32_t u32DramClockReg)
{
	int bIsCacheState = 0;
	int32_t u32CacheMode = 0;
	uint32_t vram_base, aic_status = 0, aic_statush = 0;

	//void    (*wb_func)(uint32_t,uint32_t);
	void (*wb_func)(uint32_t, uint32_t, uint32_t, uint32_t);

//  DBG_PRINTF("_dramClockSwitchStart\n");

	aic_status = inpw(REG_AIC_IMR);
	aic_statush = inpw(REG_AIC_IMRH);
	outpw(REG_AIC_MDCR, 0xFFFFFFFF);	//Disable interrupt
	outpw(REG_AIC_MDCRH, 0xFFFFFFFF);	//Disable interrupt

	vram_base = PD_RAM_BASE;

//  outp32(0xff000f80,8);
	memcpy((char *)((uint32_t) _tmp_buf | 0x80000000),
	       (char *)((uint32_t) vram_base | 0x80000000), PD_RAM_SIZE);
	memcpy((void *)((uint32_t) vram_base | 0x80000000),
	       (void *)(((uint32_t) _dramClockSwitch - (PD_RAM_START - PD_RAM_BASE)) | 0x80000000),
	       PD_RAM_SIZE);
//  outp32(0xff000f80,9);
	DBG_PRINTF("memcpy ok\n");

//  outp32(0xff000f80,10);
	if (sysGetCacheState() == 1) {
		//DBG_PRINTF("Cache enable\n");
		bIsCacheState = 1;
		u32CacheMode = sysGetCacheMode();
		sysDisableCache();
		sysFlushCache(I_D_CACHE);
	}
//    else
//    {
//        DBG_PRINTF("Cache disable\n");
//    }

//  outp32(0xff000f80,11);
	//sysFlushCache(I_D_CACHE);
	vram_base = PD_RAM_START;
	//wb_func = (void(*)(uint32_t, uint32_t)) vram_base;
	wb_func = (void (*)(uint32_t, uint32_t, uint32_t, uint32_t))vram_base;

	//--------------------------------------
	DBG_PRINTF("Jump to SRAM\n");
	//wb_func(eSrcClk, u32RegPll);
	wb_func(eSrcClk, u32RegPll, u32DramClock, u32DramClockReg);
	vram_base = PD_RAM_BASE;
	memcpy((void *)((uint32_t) vram_base | 0x80000000),
	       (void *)((uint32_t) _tmp_buf | 0x80000000), PD_RAM_SIZE);
	if (bIsCacheState == 1)
		sysEnableCache(u32CacheMode);

	outpw(REG_AIC_MECR, aic_status);	// Restore AIC setting
	outpw(REG_AIC_MECRH, aic_statush);
}

/*
    Memory clock constraint
    1. SDIC' clock > AHB1/AHB3/AHB4

    E_SYS_SRC_CLK eSrcClk           Memory clock source. It will always = MPLL
    uint32_t u32PLLClockHz,           MPLL frequency
    uint32_t u32DramClock         DDR clock (2* SDIC)

*/
extern int _sys_bIsUARTInitial;
uint32_t sysSetDramClock(E_SYS_SRC_CLK eSrcClk, uint32_t u32PLLClockHz, uint32_t u32DdrClock)
{
	uint32_t u32FinHz, u32DramDiv;
	uint32_t u32DivN1, u32DivN0, u32DramClockReg;
//  uint32_t u32CpuFreq, u32Hclk1Frq, u32Hclk234;
//  uint32_t u32DramClock = u32DdrClock/2;
	//uint32_t u32sysSrc = (inp32(REG_CLKDIV0)&SYSTEM_S)>>3;
	/* Judge MCLK > HCLK1 */
	u32DramDiv = u32PLLClockHz / u32DdrClock;
	for (u32DivN1 = 1; u32DivN1 <= 8; u32DivN1 = u32DivN1 + 1) {
		for (u32DivN0 = 1; u32DivN0 <= 8; u32DivN0 = u32DivN0 + 1) {
			if (u32DramDiv == (u32DivN1 * u32DivN0))
				break;
		}
		if (u32DivN0 >= 9)
			continue;
		if (u32DramDiv == (u32DivN1 * u32DivN0))
			break;
	}
//    if(u32DivN0>=9) //Over 8*8 range or prime number (19, 17, 13, 15..)
//    {
//        sysprintf("Can not set the clock due to divider is %d\n", u32DramDiv);
//        return WB_INVALID_CLOCK;
//    }
	if (u32DivN0 != 0)
		u32DivN0 = u32DivN0 - 1;
	if (u32DivN1 != 0)
		u32DivN1 = u32DivN1 - 1;
	u32DramClockReg = ((u32DivN1 << 5) | (eSrcClk << 3) | u32DivN0);
	u32DramClockReg = (inp32(REG_CLKDIV7) & ~(DRAM_N1 | DRAM_S | DRAM_N0)) | u32DramClockReg;

	u32FinHz = sysGetExternalClock();
	g_i32REG_MPLL = _sysGetPLLControlRegister((u32FinHz / 1000), u32PLLClockHz);

	//sysprintf("Want to set DRAM DIV REG = 0x%x\n",u32DramClockReg);
	_dramClockSwitchStart(eSrcClk,	/* PLL clock source */
			      g_i32REG_MPLL,	/* From MLL */
			      u32DdrClock / 2,	/* SDIC clock to judge high or low frequency */
			      u32DramClockReg);	/* DRAM clock divider */
	//sysprintf("MPLL REG = 0x%x\n", inp32(REG_MPLLCON));
	//sysprintf("DRAM DIV REG = 0x%x\n", inp32(REG_CLKDIV7));
	return Successful;
}

/*
    Here return is DDR clock.
    MCLK should be DDR/2
*/
uint32_t sysGetDramClock(void)
{
	uint32_t u32DramClock, u32RegData;
	uint32_t u32SrcClock, u32Div, u32ExtFreq;

	u32RegData = inp32(REG_CLKDIV7);
	u32SrcClock = (u32RegData & DRAM_S) >> 3;
	u32ExtFreq = sysGetExternalClock();

	u32Div = ((u32RegData & DRAM_N0) + 1) * (((u32RegData & DRAM_N1) >> 5) + 1);
	if (u32SrcClock == 0)
		u32SrcClock = sysGetExternalClock();
	else
		u32SrcClock = sysGetPLLOutputHz((E_SYS_SRC_CLK) u32SrcClock, u32ExtFreq);
	u32DramClock = (u32SrcClock / u32Div);
	return u32DramClock;

}
