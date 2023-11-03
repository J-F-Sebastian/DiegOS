/****************************************************************************
 * @file     NandLoader.c
 * @brief    NandLoader source code.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdint.h>
#include <string.h>
#include <types_common.h>
#include "wblib.h"
#include "turbowriter.h"

/* global variable */
typedef struct nand_info {
	uint32_t startBlock;
	uint32_t endBlock;
	uint32_t fileLen;
	uintptr_t executeAddr;
} NVT_NAND_INFO_T;

/*
 *  2014/4/11, NVTLoader will write a 16 bytes signature pattern in image file offset
 *             between 0x40 to 0x9F. (0x55AA55AA, 0xAA55AA55, 0x4F56554E, 0x2E4E4F54)
 *             NandLoader believe the image is NVTLoader if it found one pattern within this range.
 */
int isNVTLoader(NVT_NAND_INFO_T * image)
{
	unsigned int i;
	uint32_t *check = (uint32_t *) (image->executeAddr + 64);

	for (i = 0; i < 6; i++) {
		//sysprintf("--> check NVTLoader signature = 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
		//    *(unsigned int *)(image->executeAddr+i),   *(unsigned int *)(image->executeAddr+i+4),
		//    *(unsigned int *)(image->executeAddr+i+8), *(unsigned int *)(image->executeAddr+i+12));
		if ((check[0] == 0x55AA55AA) &&
		    (check[1] == 0xAA55AA55) &&
		    (check[2] == 0x4F56554E) && (check[3] == 0x2E4E4F54)) {
			return 1;	// image is NVTLoader
		}

		check += 4;
	}
	return 0;		// image is not NVTLoader
}

int MoveData(NVT_NAND_INFO_T * image, BOOL IsExecute)
{
	unsigned int page_count, block_count, curBlock, addr;
	unsigned int i, j;
	void (*fw_func)(void);

	sysprintf("Load file length %d, execute address 0x%x\n", image->fileLen,
		  image->executeAddr);

	page_count = image->fileLen / pSM0->nPageSize;
	if ((image->fileLen % pSM0->nPageSize) != 0)
		page_count++;

	block_count = page_count / pSM0->uPagePerBlock;

	curBlock = image->startBlock;
	addr = image->executeAddr;
	j = 0;
	while (1) {
		if (j >= block_count)
			break;

		if (CheckBadBlockMark(curBlock) == Successful) {
			for (i = 0; i < pSM0->uPagePerBlock; i++) {
				sicSMpread(0, curBlock, i, (uint8_t *) addr);
				addr += pSM0->nPageSize;
			}
			j++;
		}
		curBlock++;
	}

	if ((page_count % pSM0->uPagePerBlock) != 0) {
		page_count = page_count - block_count * pSM0->uPagePerBlock;
 _read_:
		if (CheckBadBlockMark(curBlock) == Successful) {
			for (i = 0; i < page_count; i++) {
				sicSMpread(0, curBlock, i, (uint8_t *) addr);
				addr += pSM0->nPageSize;
			}
		} else {
			curBlock++;
			goto _read_;
		}
	}

	if (IsExecute) {
		// disable NAND control pin used
		outpw(REG_GPDFUN0, inpw(REG_GPDFUN0) & (~0xFFF00000));	// disable NRE/RB0/RB1 pins
		outpw(REG_GPDFUN1, inpw(REG_GPDFUN1) & (~0x0000000F));	// disable NWR pins
		outpw(REG_GPEFUN1, inpw(REG_GPEFUN1) & (~0x000FFFFF));	// disable CS0/ALE/CLE/ND3/CS1 pins

		sysprintf("Nand Boot Loader exit. Jump to execute address 0x%x ...\n",
			  image->executeAddr);

		// 2014/4/11, keep enable cache for NVTLoader only to speed up the system booting.
		if (!isNVTLoader(image)) {
			// Disable cache. Use assembly code to avoid be optimized by compile option -O2.
			extern void sys_flush_and_clean_dcache(void);
			volatile int temp;

			sys_flush_and_clean_dcache();
			__asm volatile
			 (
		/*----- flush I, D cache & write buffer -----*/
				 "MOV %0, #0x0				\n\t" "MCR p15, 0, %0, c7, c5, #0 	\n\t"	/* flush I cache */
				 "MCR p15, 0, %0, c7, c6, #0  \n\t"	/* flush D cache */
				 "MCR p15, 0, %0, c7, c10,#4  \n\t"	/* drain write buffer */
		/*----- disable Protection Unit -----*/
				 "MRC p15, 0, %0, c1, c0, #0   \n\t"	/* read Control register */
				 "BIC %0, %0, #0x01            \n\t" "MCR p15, 0, %0, c1, c0, #0   \n\t"	/* write Control register */
				 ::"r" (temp):"memory");
		}

		fw_func = (void (*)(void))(image->executeAddr);
		fw_func();
	}
	return 0;
}

static unsigned char image_buffer[8192] __attribute__((aligned(32)));

unsigned int *imagebuf;
unsigned int *pImageList;

#define __DDR2__
#define E_CLKSKEW   0x00888800

void initClock(void)
{
	uint32_t u32ExtFreq;
	uint32_t reg_tmp;

	u32ExtFreq = sysGetExternalClock();	// Hz unit
	if (u32ExtFreq == 12000000UL) {
		outp32(REG_SDREF, 0x802D);
	} else {
		outp32(REG_SDREF, 0x80C0);
	}

#ifdef __UPLL_240__
	//--- support UPLL 240MHz, MPLL 360MHz, APLL 432MHz
	outp32(REG_CKDQSDS, E_CLKSKEW);
#ifdef __DDR2__
	outp32(REG_SDTIME, 0x2ABF394A);	// REG_SDTIME for N9H26 360MHz SDRAM clock
	outp32(REG_SDMR, 0x00000432);
	outp32(REG_MISC_SSEL, 0x00000155);	// set MISC_SSEL to Reduced Strength to improve EMI
#endif

	// initial DRAM clock BEFORE initial system clock since we change it from low (216MHz by IBR) to high (360MHz)
	sysSetDramClock(eSYS_MPLL, 360000000, 360000000);	// change from 216MHz (IBR) to 360MHz,

	// initial system clock
	sysSetSystemClock(eSYS_UPLL, 240000000,	// Specified the APLL/UPLL clock, unit Hz
			  240000000);	// Specified the system clock, unit Hz
	sysSetCPUClock(240000000);	// Unit Hz
	sysSetAPBClock(60000000);	// Unit Hz

	// set APLL to 432MHz to support TVout (need 27MHz)
	sysSetPllClock(eSYS_APLL, 432000000);
#endif				// __UPLL_240__

	// always set HCLK234 to 0
	reg_tmp = inp32(REG_CLKDIV4) | CHG_APB;	// MUST set CHG_APB to HIGH when configure CLKDIV4
	outp32(REG_CLKDIV4, reg_tmp & (~HCLK234_N));
}

int main()
{
	NVT_NAND_INFO_T image;
	int count, i;

	/* Clear Boot Code Header in SRAM to avoid booting fail issue */
	outp32(0xFF000000, 0);

	//--- enable cache to speed up booting
	sysDisableCache();
	sysFlushCache(I_D_CACHE);
	sysEnableCache(CACHE_WRITE_BACK);
	
	sysprintf("DiegOS N9H26 Nand Boot Loader\n");

	/* PLL clock setting */
	initClock();

	sysprintf("System clock = %dHz\nDRAM clock = %dHz\nREG_SDTIME = 0x%08X\n",
		  sysGetSystemClock(), sysGetDramClock(), inp32(REG_SDTIME));

	imagebuf = (unsigned int *)((uintptr_t) image_buffer | 0x80000000UL);
	pImageList = (unsigned int *)(imagebuf);

	/* Initial DMAC and NAND interface */
	fmiInitDevice();
	sicSMInit();
	memset((char *)&image, 0, sizeof(NVT_NAND_INFO_T));

	/* read physical block 0~3 - image information */
	for (i = 0; i < 4; i++) {
		if (!sicSMpread(0, i, pSM0->uPagePerBlock - 2, (uint8_t *) imagebuf)) {
			if ((pImageList[0] == 0x574255aa) && (pImageList[3] == 0x57425963)) {
				sysprintf("Get image information from block 0x%x ..\n", i);
				break;
			}
		}
	}

	if (i < 4) {
		count = pImageList[1];

		/* load logo first */
		pImageList = pImageList + 4;
		for (i = 0; i < count; i++) {
			if (((*(pImageList) >> 16) & 0xffff) == 4)	// logo
			{
				image.startBlock = *(pImageList + 1) & 0xffff;
				image.endBlock = (*(pImageList + 1) & 0xffff0000) >> 16;
				image.executeAddr = *(pImageList + 2);
				image.fileLen = *(pImageList + 3);
				MoveData(&image, FALSE);
				break;
			}
			/* pointer to next image */
			pImageList = pImageList + 12;
		}

		pImageList = ((unsigned int *)(((unsigned int)image_buffer) | 0x80000000));
		memset((char *)&image, 0, sizeof(NVT_NAND_INFO_T));

		/* load execution file */
		pImageList = pImageList + 4;
		for (i = 0; i < count; i++) {
			if (((*(pImageList) >> 16) & 0xffff) == 1)	// execute
			{
				image.startBlock = *(pImageList + 1) & 0xffff;
				image.endBlock = (*(pImageList + 1) & 0xffff0000) >> 16;
				image.executeAddr = *(pImageList + 2);
				// sysprintf("executing address = 0x%x\n", image.executeAddr);
				image.fileLen = *(pImageList + 3);
				MoveData(&image, TRUE);
				break;
			}
			/* pointer to next image */
			pImageList = pImageList + 12;
		}
		while (1) ;
	}
	return 0;
}
