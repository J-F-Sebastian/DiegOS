/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2015 Diego Gallizioli
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _N9H26_SIC_H_
#define _N9H26_SIC_H_

#include <types_common.h>

/*
 * All registers are 32 bit wide
 */

#define N9H26SICSB_BASE 0xB1006000UL

/*
 * This is the interface to SIC shared buffer
 */
enum N9H26SICSB {
	FMI_FB_0,
	FMI_FB_1,
	FMI_FB_2,
	FMI_FB_3,
	FMI_FB_4,
	FMI_FB_5,
	FMI_FB_6,
	FMI_FB_7,
	FMI_FB_8,
	FMI_FB_9,
	FMI_FB_10,
	FMI_FB_11,
	FMI_FB_12,
	FMI_FB_13,
	FMI_FB_14,
	FMI_FB_15,
	FMI_FB_16,
	FMI_FB_17,
	FMI_FB_18,
	FMI_FB_19,
	FMI_FB_20,
	FMI_FB_21,
	FMI_FB_22,
	FMI_FB_23,
	FMI_FB_24,
	FMI_FB_25,
	FMI_FB_26,
	FMI_FB_27,
	FMI_FB_28,
	FMI_FB_29,
	FMI_FB_30,
	FMI_FB_31
};

#define N9H26SICDMA_BASE 0xB1006400UL

/*
 * Interface to DMAC Registers
 */
enum N9H26SICDMA {
	DMACCSR,
	DMACSAR,
	DMACBCR,
	DMACIER,
	DMACISR
};

#define N9H26SICFMI_BASE 0xB1006800UL

/*
 * Interface to FMI Global Registers, Secure-Digital Registers, Smart-Media Registers
 */
enum N9H26SICFMI {
	FMICR,
	FMIIER,
	FMIISR,
	/*
	 * Interface to Secure-Digital Registers
	 */
	SDCR = 8,
	SDARG,
	SDIER,
	SDISR,
	SDRSP0,
	SDRSP1,
	SDBLEN,
	SDTMOUT,
	/*
	 * Smart-Media Registers
	 */
	SMCSR = 40,
	SMTCR,
	SMIER,
	SMISR,
	SMCMD,
	SMADDR,
	SMDATA,
	SMREAREA_CTL,
	SM_ECC_ST0,
	SM_ECC_ST1,
	SM_ECC_ST2,
	SM_ECC_ST3,
	SM_PROT_ADDR0,
	SM_PROT_ADDR1
};

#define N9H26SICECC_BASE 0xB1006900UL

enum N9H26SICECC {
	/*
	 * Smart-Media BCH Error Address Register
	 */
	BCH_ECC_ADDR0,
	BCH_ECC_ADDR1,
	BCH_ECC_ADDR2,
	BCH_ECC_ADDR3,
	BCH_ECC_ADDR4,
	BCH_ECC_ADDR5,
	BCH_ECC_ADDR6,
	BCH_ECC_ADDR7,
	BCH_ECC_ADDR8,
	BCH_ECC_ADDR9,
	BCH_ECC_ADDR10,
	BCH_ECC_ADDR11,
	/*
	 * Smart-Media BCH Error Data Register
	 */
	BCH_ECC_DATA0 = 40,
	BCH_ECC_DATA1,
	BCH_ECC_DATA2,
	BCH_ECC_DATA3,
	BCH_ECC_DATA4,
	BCH_ECC_DATA5,

};

#define N9H26SICSMRA_BASE 0xB1006A00UL

enum N9H26SICSMRA {
	SM_RA0,
	/* ... */
	SM_RA117 = 117
};

/*
 SIC Control Registers
*/

//#define     REG_FB_0          (DMAC_BA+0x000)  /* Shared Buffer (FIFO) */

//#define       REG_DMACCSR                     (DMAC_BA+0x400)  /* DMAC Control and Status Register */
#define FMI_BUSY	BIT32(9)	// FMI DMA transfer is in progress
#define SG_EN		BIT32(3)	// DMAC Scatter-gather function enable
#define DMAC_SWRST	BIT32(1)	// DMAC software reset enable
#define DMAC_EN		BIT32(0)	// DMAC enable

//#define     REG_DMACSAR           (DMAC_BA+0x408)  /* DMAC Transfer Starting Address Register */
#define PAD_ORDER       BIT32(0)	// Physical Address Descriptor table fetching in order or out of order
//#define     REG_DMACBCR               (DMAC_BA+0x40C)  /* DMAC Transfer Byte Count Register */
//#define     REG_DMACIER               (DMAC_BA+0x410)  /* DMAC Interrupt Enable Register */
#define WEOT_IE		BIT32(1)	// Wrong EOT encounterred interrupt enable
#define TABORT_IE	BIT32(0)	// DMA R/W target abort interrupt enable

//#define     REG_DMACISR               (DMAC_BA+0x414)  /* DMAC Interrupt Status Register */
#define WEOT_IF		BIT32(1)	// Wrong EOT encounterred interrupt flag
#define TABORT_IF	BIT32(0)	// DMA R/W target abort interrupt flag

/* Flash Memory Card Interface Registers */
//#define REG_FMICR                             (FMI_BA+0x000)          /* FMI Control Register */
#define FMI_SM_EN	BIT32(3)	// enable FMI SM function
#define FMI_SD_EN	BIT32(1)	// enable FMI SD function
#define FMI_SWRST	BIT32(0)	// enable FMI software reset

//#define REG_FMIIER                            (FMI_BA+0x004)          /* FMI DMA Transfer Starting Address Register */
#define FMI_DAT_IE	BIT32(0)	// enable DMAC READ/WRITE targe abort interrupt generation

//#define REG_FMIISR                            (FMI_BA+0x008)          /* FMI DMA Byte Count Register */
#define FMI_DAT_IF	BIT32(0)	// DMAC READ/WRITE targe abort interrupt flag register

/* Secure Digit Registers */
//#define REG_SDCR                              (FMI_BA+0x020)          /* SD Control Register */
#define SDCR_CLK_KEEP1	BIT32(31)	// SD-1 clock keep control
#define SDCR_SDPORT	BITMASK32(30,29)	// SD port select
#define SDCR_SDPORT_0	0	// SD-0 port selected
#define SDCR_SDPORT_1	BIT32(29)	// SD-1 port selected
#define SDCR_SDPORT_2	BIT32(30)	// SD-2 port selected
#define SDCR_CLK_KEEP2	BIT32(28)	// SD-1 clock keep control
#define SDCR_SDNWR	BITMASK32(27,24)	// Nwr paramter for Block Write operation
#define SDCR_BLKCNT	BITMASK32(23,16)	// Block conut to be transferred or received
#define SDCR_DBW	BIT32(15)	// SD data bus width selection
#define SDCR_SWRST	BIT32(14)	// enable SD software reset
#define SDCR_CMD_CODE	BITMASK32(13,8)	// SD Command Code
#define SDCR_CLK_KEEP	BIT32(7)	// SD Clock Enable
#define SDCR_8CLK_OE	BIT32(6)	// 8 Clock Cycles Output Enable
#define SDCR_74CLK_OE	BIT32(5)	// 74 Clock Cycle Output Enable
#define SDCR_R2_EN	BIT32(4)	// Response R2 Input Enable
#define SDCR_DO_EN	BIT32(3)	// Data Output Enable
#define SDCR_DI_EN	BIT32(2)	// Data Input Enable
#define SDCR_RI_EN	BIT32(1)	// Response Input Enable
#define SDCR_CO_EN	BIT32(0)	// Command Output Enable

//#define REG_SDARG                             (FMI_BA+0x024)          /* SD command argument register */

//#define REG_SDIER                             (FMI_BA+0x028)          /* SD interrupt enable register */
#define SDIER_CD2SRC	BIT32(31)	// SD card detection source selection: SD-DAT3 or GPIO
#define SDIER_CDSRC	BIT32(30)	// SD card detection source selection: SD-DAT3 or GPIO
#define SDIER_R1B_IEN	BIT32(24)	// R1b interrupt enable
#define SDIER_WKUP_EN	BIT32(14)	// SDIO wake-up signal geenrating enable
#define SDIER_DITO_IEN	BIT32(13)	// SD data input timeout interrupt enable
#define SDIER_RITO_IEN	BIT32(12)	// SD response input timeout interrupt enable
#define SDIER_SDIO_IEN	BIT32(10)	// SDIO Interrupt Status Enable (SDIO issue interrupt via DAT[1]
#define SDIER_CD_IEN	BIT32(8)	// CD# Interrupt Status Enable
#define SDIER_CRC_IEN	BIT32(1)	// CRC-7, CRC-16 and CRC status error interrupt enable
#define SDIER_BLKD_IEN	BIT32(0)	// Block transfer done interrupt interrupt enable

//#define REG_SDISR                             (FMI_BA+0x02C)          /* SD interrupt status register */
#define SDISR_R1B_IF	BIT32(24)	// R1b interrupt flag
#define SDISR_SD_DATA1	BIT32(18)	// SD DAT1 pin status
#define SDISR_CD_Card	BIT32(16)	// CD detection pin status
#define SDISR_DITO_IF	BIT32(13)	// SD data input timeout interrupt flag
#define SDISR_RITO_IF	BIT32(12)	// SD response input timeout interrupt flag
#define SDISR_SDIO_IF	BIT32(10)	// SDIO interrupt flag (SDIO issue interrupt via DAT[1]
#define SDISR_CD_IF	BIT32(8)	// CD# interrupt flag
#define SDISR_SD_DATA0	BIT32(7)	// SD DATA0 pin status
#define SDISR_CRC	BITMASK32(6,4)	// CRC status
#define SDISR_CRC_16	BIT32(3)	// CRC-16 Check Result Status
#define SDISR_CRC_7	BIT32(2)	// CRC-7 Check Result Status
#define SDISR_CRC_IF	BIT32(1)	// CRC-7, CRC-16 and CRC status error interrupt status
#define SDISR_BLKD_IF	BIT32(0)	// Block transfer done interrupt interrupt status

//#define REG_SDRSP0                            (FMI_BA+0x030)          /* SD receive response token register 0 */
//#define REG_SDRSP1                            (FMI_BA+0x034)          /* SD receive response token register 1 */
//#define REG_SDBLEN                            (FMI_BA+0x038)          /* SD block length register */
//#define REG_SDTMOUT                   (FMI_BA+0x03C)          /* SD block length register */

/* NAND-type Flash Registers */

//#define     REG_SM_ECC48_ST0          (FMI_BA+0x0D4)   /* ECC Register */
//#define     REG_SM_ECC48_ST1          (FMI_BA+0x0D8)   /* ECC Register */
//#define     REG_BCH_ECC_BIT_ADDR0     (FMI_BA+0x220)   /* NAND Flash BCH error bit address for error bit 0-7 Register */
//#define     REG_BCH_ECC_BIT_ADDR1     (FMI_BA+0x224)   /* NAND Flash BCH error bit address for error bit 8-14 Register */

//#define REG_SMCSR                             (FMI_BA+0x0A0)          /* NAND Flash Control and Status Register */
#define SMCR_CS1	BIT32(26)	// SM chip select
#define SMCR_CS0	BIT32(25)	// SM chip select
#define SMCR_CS		BIT32(25)	// SM chip select
#define SMCR_ECC_EN	BIT32(23)	// SM chip select
#define SMCR_BCH_TSEL 	BITMASK32(22,18)	// BCH T4/8/12/15/24 selection
#define BCH_T15 	BIT32(22)	// BCH T15 selected
#define BCH_T12 	BIT32(21)	// BCH T12 selected
#define BCH_T8	 	BIT32(20)	// BCH T8 selected
#define BCH_T4	 	BIT32(19)	// BCH T4 selected
#define BCH_T24	 	BIT32(18)	// BCH T24 selected

#define SMCR_PSIZE 	BITMASK32(17,16)	// SM page size selection
#define PSIZE_8K 	BIT32(17)+BIT32(16)	// page size 8K selected
#define PSIZE_4K 	BIT32(17)	// page size 4K selected
#define PSIZE_2K 	BIT32(16)	// page size 2K selected
#define PSIZE_512	0	// page size 512 selected

#define SMCR_SRAM_INIT		BIT32(9)	// SM RA0_RA1 initial bit (to 0xFFFF_FFFF)
#define SMCR_ECC_3B_PROTECT	BIT32(8)	// ECC protect redundant 3 bytes
#define SMCR_ECC_CHK		BIT32(7)	// ECC parity check enable bit during read page
#define SMCR_PROT_REGION_EN 	BIT32(5)	// Protect Region enable
#define SMCR_REDUN_AUTO_WEN 	BIT32(4)	// Redundant auto write enable
#define SMCR_REDUN_REN 		BIT32(3)	// Redundant read enable
#define SMCR_DWR_EN 		BIT32(2)	// DMA write data enable
#define SMCR_DRD_EN 		BIT32(1)	// DMA read data enable
#define SMCR_SM_SWRST 		BIT32(0)	// SM software reset

//#define REG_SMTCR                             (FMI_BA+0x0A4)          /* NAND Flash Timing Control Register */

//#define REG_SMIER                             (FMI_BA+0x0A8)          /* NAND Flash Interrupt Control Register */
#define SMIER_RB1_IE			BIT32(11)	// RB1 pin rising-edge detection interrupt enable
#define SMIER_RB0_IE			BIT32(10)	// RB0 pin rising-edge detection interrupt enable
#define SMIER_RB_IE			BIT32(10)	// RB0 pin rising-edge detection interrupt enable
#define SMIER_PROT_REGION_WR_IE 	BIT32(3)	// Protect Region write detect interrupt enable
#define SMIER_ECC_FIELD_IE 		BIT32(2)	// ECC field error check interrupt enable
#define SMIER_DMA_IE			BIT32(0)	// DMA RW data complete interrupr enable

//#define REG_SMISR                             (FMI_BA+0x0AC)          /* NAND Flash Interrupt Status Register */
#define SMISR_RB1		 	BIT32(19)	// RB1 pin status
#define SMISR_RB0 			BIT32(18)	// RB0 pin status
#define SMISR_RB 			BIT32(18)	// RB pin status
#define SMISR_RB1_IF		 	BIT32(11)	// RB pin rising-edge detection interrupt flag
#define SMISR_RB0_IF	 		BIT32(10)	// RB pin rising-edge detection interrupt flag
#define SMISR_PROT_REGION_WR_IF 	BIT32(3)	// Protect Region write detect interrupt flag
#define SMISR_ECC_FIELD_IF 		BIT32(2)	// ECC field error check interrupt flag
#define SMISR_DMA_IF			BIT32(0)	// DMA RW data complete interrupr flag

//#define REG_SMCMD                             (FMI_BA+0x0B0)          /* NAND Flash Command Port Register */

//#define REG_SMADDR                            (FMI_BA+0x0B4)          /* NAND Flash Address Port Register */
#define EOA_SM 	BIT32(31)	// end of SM address for last SM address

//#define REG_SMDATA                            (FMI_BA+0x0B8)          /* NAND Flash Data Port Register */

//#define REG_SMREAREA_CTL              (FMI_BA+0x0BC)          /* NAND Flash redundnat area control register */
#define SMRE_MECC 	BITMASK32(31,16)	// Mask ECC parity code to NAND during Write Page Data to NAND by DMAC
#define SMRE_REA128_EXT	BITMASK32(8,0)	// Redundant area enabled byte number

//#define REG_SM_ECC_ST0                        (FMI_BA+0x0D0)          /* ECC Register */
#define ECCST_F4_ECNT 	BITMASK32(30,26)	// error count of ECC for field 4
#define ECCST_F4_STAT 	BITMASK32(25,24)	// error status of ECC for field 4
#define ECCST_F3_ECNT 	BITMASK32(22,18)	// error count of ECC for field 3
#define ECCST_F3_STAT 	BITMASK32(17,16)	// error status of ECC for field 3
#define ECCST_F2_ECNT 	BITMASK32(14,10)	// error count of ECC for field 2
#define ECCST_F2_STAT 	BITMASK32(9,8)	// error status of ECC for field 2
#define ECCST_F1_ECNT 	BITMASK32(6,2)	// error count of ECC for field 1
#define ECCST_F1_STAT 	BITMASK32(1,0)	// error status of ECC for field 1

//#define REG_SM_ECC_ST1                    (FMI_BA+0x0D4)      /* ECC Register */
#define ECCST_F8_ECNT 	BITMASK32(30,26)	// error count of ECC for field 8
#define ECCST_F8_STAT 	BITMASK32(25,24)	// error status of ECC for field 8
#define ECCST_F7_ECNT 	BITMASK32(22,18)	// error count of ECC for field 7
#define ECCST_F7_STAT 	BITMASK32(17,16)	// error status of ECC for field 7
#define ECCST_F6_ECNT 	BITMASK32(14,10)	// error count of ECC for field 6
#define ECCST_F6_STAT 	BITMASK32(9,8)	// error status of ECC for field 6
#define ECCST_F5_ECNT 	BITMASK32(6,2)	// error count of ECC for field 5
#define ECCST_F5_STAT 	BITMASK32(1,0)	// error status of ECC for field 5

//#define REG_SM_ECC_ST2                    (FMI_BA+0x0D8)      /* ECC Register */
#define ECCST_F12_ECNT		BITMASK32(30,26)	// error count of ECC for field 12
#define ECCST_F12_STAT 		BITMASK32(25,24)	// error status of ECC for field 12
#define ECCST_F11_ECNT 	        BITMASK32(22,18)	// error count of ECC for field 11
#define ECCST_F11_STAT 	        BITMASK32(17,16)	// error status of ECC for field 11
#define ECCST_F10_ECNT 	        BITMASK32(14,10)	// error count of ECC for field 10
#define ECCST_F10_STAT 	        BITMASK32(9,8)	// error status of ECC for field 10
#define ECCST_F9_ECNT 	        BITMASK32(6,2)	// error count of ECC for field 9
#define ECCST_F9_STAT 	        BITMASK32(1,0)	// error status of ECC for field 9

//#define REG_SM_ECC_ST3                    (FMI_BA+0x0DC)      /* ECC Register */
#define ECCST_F16_ECNT		BITMASK32(30,26)	// error count of ECC for field 16
#define ECCST_F16_STAT 		BITMASK32(25,24)	// error status of ECC for field 16
#define ECCST_F15_ECNT 	        BITMASK32(22,18)	// error count of ECC for field 15
#define ECCST_F15_STAT 	        BITMASK32(17,16)	// error status of ECC for field 15
#define ECCST_F14_ECNT 	        BITMASK32(14,10)	// error count of ECC for field 14
#define ECCST_F14_STAT 	        BITMASK32(9,8)	// error status of ECC for field 14
#define ECCST_F13_ECNT 	        BITMASK32(6,2)	// error count of ECC for field 13
#define ECCST_F13_STAT 	        BITMASK32(1,0)	// error status of ECC for field 13

//#define REG_SM_PROT_ADDR0         (FMI_BA+0x0E0)      /* Smart-Media Protect Region End Address 0 Register */
//#define REG_SM_PROT_ADDR1         (FMI_BA+0x0E4)      /* Smart-Media Protect Region End Address 1 Register */
#define SM_PROT_ADDR1 	        BITMASK32(7,0)	// High address for Protect Region End Address

#endif
