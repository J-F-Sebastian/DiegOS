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

#ifndef _N9H26_GCR_H_
#define _N9H26_GCR_H_

#include <types_common.h>

/*
 * All registers are 32 bit wide
 */

#define N9H26GCR_BASE 0xB0000000UL

enum N9H26GCR {
	CHIPID,
	CHIPCFG,
	OVCKCFG,
	AHBCTL = 4,
	AHBIPRST,
	APBIPRST,
	MISCR = 8,
	SDRBIST,
	CRBIST,
	ED0SSR,
	ED1SSR,
	MISSR,
	AHB4_TOC0 = 16,
	AHB4_TOC1,
	CDCVC,
	POR_LVRD = 29,
	GPAFUN0 = 128,
	GPAFUN1,
	GPBFUN0,
	GPBFUN1,
	GPCFUN0,
	GPCFUN1,
	GPDFUN0,
	GPDFUN1,
	GPEFUN0,
	GPEFUN1,
	MISFUN,
	SPI_LVD_GPH,
	MISCPCR,
	MISC_SL_GPA,
	MISC_SL_GPB,
	MISC_SL_GPC,
	MISC_SL_GPD,
	MISC_SL_GPE,
	MISC_SL_ND,
	MISC_DS_GPA,
	MISC_DS_GPB,
	MISC_DS_GPC,
	MISC_DS_GPD,
	MISC_DS_GPE,
	MISC_DS_ND,
	MISC_SSEL,
	GPGFUN0,
	GPGFUN1,
	GPHFUN,
	ShrPin_TVDAC,
	ShrPin_AUDIO,
	ShrPin_TOUCH,
	ExtRST_DEBOUNCE,
	ExtRST_REBOUNCE_CNTR
};

// REG_CHIPID   (GCR_BA+0x00)   // R    Chip Identification Register
#define CHIP_ID			BITMASK32(23, 0)	// Chip Identification

// REG_CHIPCFG  (GCR_BA+0x04)   // R/W  Chip Power-On Configuration Register
#define HVMODE		BIT32(17)	// ARM vector mode
#define CAP_SRC		BIT32(14)	// Common mode. Capture0 and Capture1 share the common sensor.
#define ROT_IRM		BIT32(13)	// IR mode. Rotate Engine's 32KB SRAM is for internal RAM or working memory
#define DLL_TEST	BIT32(12)	// Enter DLL test mode
#define DQSWAP		BIT32(11)	// Swap DDRII DQ8/DQ15 and DQ9/DQ14
#define NTYPE		BIT32(10)	// NAND Type
#define NPAGE		BITMASK32(9, 8)	// NAND Page Size
#define NADDR		BIT32(7)	// NAND Address Cycle
#define CLK_SRC		BIT32(6)	// System Clock Source Selection
#define SDRAMSEL	BITMASK32(5, 4)	// SDRAM Type Selection
#define COPMODE		BITMASK32(3, 0)	// Chip Operation Mode Selection

// REG_OVCKCFG  (GCR_BA+0x08)   // R/W  Overclk Configuration Register
#define OVCK_SEL	BITMASK32(18, 17)	// Overclk counter output select
#define OVCK_SET	BIT32(16)	// Overclk_circuit enable
#define OVCK_CNT	BITMASK32(15, 0)	// Overclk circuit counting result (Read Only)

// REG_AHBCTL   (GCR_BA+0x10)   // R/W  AHB Bus Arbitration Control Register
#define IPACT		BIT32(5)	// Interrupt Active Status
#define IPEN		BIT32(4)	// CPU Priority Raising Enable during Interrupt Period
#define PRTMOD1		BIT32(1)	// Priority Mode Control 1
#define PRTMOD0		BIT32(0)	// Priority Mode Control 0

// REG_AHBIPRST (GCR_BA+0x14)   // R/W  AHB IP Reset Control Resister
#define CRC1_RST	BIT32(24)	// CRC2 Controller Reset
#define SDIO_RST	BIT32(23)	// SDIO Controller Reset
#define CRC_RST		BIT32(22)	// CRC Controller Reset
#define UHC20_RST	BIT32(21)	// USB Host 2.0 Controller Reset
#define ROT_RST		BIT32(20)	// ROT Reset
#define VPE_RST		BIT32(19)	// VPE Reset
#define VIN1_RST	BIT32(18)	// VIN1 Reset
#define JPG_RST		BIT32(17)	// JPG Reset
#define BLT_RST		BIT32(16)	// BLT Reset
#define VDE_RST		BIT32(15)	// H.264 Video Decoder Reset
#define IPSEC_RST	BIT32(14)	// AES Reset
#define VEN_RST		BIT32(13)	// H.264 Video Encoder Reset
#define AAC_RST		BIT32(12)	// OVG Reset
#define VIN0_RST	BIT32(11)	// VIN 0 Reset
#define VPOST_RST	BIT32(10)	// VPOST Reset
#define I2S_RST		BIT32(9)	// I2S Reset
#define SPU_RST		BIT32(8)	// SPU Reset
#define UHC_RST		BIT32(7)	// UHC Reset
#define UDC_RST		BIT32(6)	// UDC Reset
#define SIC_RST		BIT32(5)	// SIC Reset
#define TIC_RST		BIT32(4)	// TIC Reset
#define EDMA_RST	BIT32(3)	// EDMA Reset
#define SRAM_RST	BIT32(2)	// SRAM Reset
#define EMAC_RST	BIT32(1)	// EDMA Reset
#define SDIC_RST	BIT32(0)	// SDIC Reset

// REG_APBIPRST (GCR_BA+0x18)   // R/W  APB IP Reset Control Resister
#define TMR3RST		BIT32(17)	// Timer 3 Reset
#define TMR2RST		BIT32(16)	// Timer 2 Reset
#define KPIRST		BIT32(15)	// KPI Reset
#define ADCRST		BIT32(14)	// ADC Reset
#define SPI1RST		BIT32(13)	// SPI 1 Reset
#define SPI0RST		BIT32(12)	// SPI 0 Reset
#define RSCRST		BIT32(11)	// RSC Reset
#define PWMRST		BIT32(10)	// PWM Reset
#define RFCRST		BIT32(9)	// RFC Reset
#define I2CRST		BIT32(8)	// I2C Reset
#define UART1RST	BIT32(7)	// UART 1 Reset
#define UART0RST	BIT32(6)	// UART 0 Reset
#define TMR1RST		BIT32(5)	// TMR1 Reset
#define TMR0RST		BIT32(4)	// TMR0 Reset
#define WDTRST		BIT32(3)	// WDT Reset
#define GPIORST		BIT32(1)	// RTC Reset
#define TOUCHRST	BIT32(0)	// TOUCH Reset

// REG_MISCR    (GCR_BA+0x20)   // R/W  Miscellaneous Control Register
#define WDTRSTEN	BIT32(24)	// WatchDog Timer Reset Connection Enable
#define UTMISnoop	BIT32(23)	// UTMI Monitor Mode Enable
#define SEL_HSCUR	BITMASK32(19, 18)	// USB 2.0 PHY Control SEL_HSCUR
#define SEL_PHASE	BITMASK32(17, 16)	// USB 2.0 PHY Control SEL_PHASE
#define UTMISnoop_HOST	BIT32(15)	// UTMI Monitor HOST Mode Enable
#define SEL_HSCUR_HOST	BITMASK32(11, 10)	// USB 2.0 HOST PHY Control SEL_HSCUR
#define SEL_PHASE_HOST	BITMASK32(9, 8)	// USB 2.0 HOST PHY Control SEL_PHASE
#define CPURSTON	BIT32(1)	// CPU always keep in reset state for TIC
#define CPURST		BIT32(0)	// CPU one shutte reset.

// REG_SDRBIST  (GCR_BA+0x24)   // R/W  Power Management Control Register
#define TEST_BUSY	BIT32(31)	// Test BUSY
#define CON_BUSY	BIT32(30)	// Connection Test Busy
#define BIST_BUSY	BIT32(29)	// BIST Test Busy
#define TEST_FAIL	BIT32(28)	// Test Failed
#define CON_FAIL	BIT32(27)	// Connection Test Failed
#define BIST_FAIL	BIT32(26)	// BIST Test Failed

// REG_ED0SSR   (GCR_BA+0x2C)   // R/W  EDMA Service Selection Control Register
#define CH4_TXSEL	BITMASK32(30, 28)	// EDMA Channel 4 Tx Selection
#define CH3_TXSEL	BITMASK32(26, 24)	// EDMA Channel 3 Tx Selection
#define CH2_TXSEL	BITMASK32(22, 20)	// EDMA Channel 2 Tx Selection
#define CH1_TXSEL	BITMASK32(18, 16)	// EDMA Channel 1 Tx Selection
#define CH4_RXSEL	BITMASK32(14, 12)	// EDMA Channel 4 Rx Selection
#define CH3_RXSEL	BITMASK32(10, 8)	// EDMA Channel 3 Rx Selection
#define CH2_RXSEL	BITMASK32(6, 4)	// EDMA Channel 2 Rx Selection
#define CH1_RXSEL	BITMASK32(2, 0)	// EDMA Channel 1 Rx Selection

// REG_ED1SSR   (GCR_BA+0x30)   // R/W  EDMA Service Selection Control Register
#define CH12_TXSEL	BITMASK32(30, 28)	// EDMA Channel 12 Tx Selection
#define CH11_TXSEL	BITMASK32(26, 24)	// EDMA Channel 11 Tx Selection
#define CH10_TXSEL	BITMASK32(22, 20)	// EDMA Channel 10 Tx Selection
#define CH9_TXSEL	BITMASK32(18, 16)	// EDMA Channel 9 Tx Selection
#define CH12_RXSEL	BITMASK32(14, 12)	// EDMA Channel 12 Rx Selection
#define CH11_RXSEL	BITMASK32(10, 8)	// EDMA Channel 11 Rx Selection
#define CH10_RXSEL	BITMASK32(6, 4)	// EDMA Channel 10 Rx Selection
#define CH9_RXSEL	BITMASK32(2, 0)	// EDMA Channel 9 Rx Selection

// REG_MISSR    (GCR_BA+0x34)   // R/W  Miscellaneous Status Register
#define KPI_WS		BIT32(31)	//KPI Wake-Up Status
#define ADC_WS		BIT32(30)	//ADC Wake-Up Status
#define UHC_WS		BIT32(29)	//UHC Wake-Up Status
#define UDC_WS		BIT32(28)	//UDC Wake-Up Status
#define UART_WS		BIT32(27)	//UART Wake-Up Status
#define SDH_WS		BIT32(26)	//SDH Wake-Up Status
#define RTC_WS		BIT32(25)	//RTC Wake-Up Status
#define GPIO_WS		BIT32(24)	//GPIO Wake-Up Status
#define KPI_WE		BIT32(23)	//KPI Wake-Up Enable
#define ADC_WE		BIT32(22)	//ADC Wake-Up Enable
#define UHC_WE		BIT32(21)	//UHC Wake-Up Enable
#define UDC_WE		BIT32(20)	//UDC Wake-Up Enable
#define UART_WE		BIT32(19)	//UART Wake-Up Enable
#define SDH_WE		BIT32(18)	//SDH Wake-Up Enable
#define RTC_WE		BIT32(17)	//RTC Wake-Up Enable
#define GPIO_WE		BIT32(16)	//GPIO Wake-Up Enable
#define UHC20_WS	BIT32(15)	//UHC20 Wake-Up Status
#define EMAC_WS		BIT32(14)	//EMAC Wake-Up Status
#define UHC20_WE	BIT32(7)	//UHC20 Wake-Up Enable
#define EMAC_WE		BIT32(6)	//EMAC Wake-Up Enable
#define POR12_RST	BIT32(5)	//POR12 Reset Active Status
#define CPU_RST		BIT32(4)	//CPU RST
#define WDT_RST		BIT32(3)	//WDT RST
#define KPI_RST		BIT32(2)	//KPI RST
#define LVR_RST		BIT32(1)	//LVR RST
#define EXT_RST		BIT32(0)	//EXT RST

// REG_AHB4_TOC0        (GCR_BA+0x40)   // R/W  AHB4 Master's 1st Time-Out Counter Control Register
#define Master3_TOC		BITMASK32(31, 24)	// AHB4 Master3 Time-Out Counter
#define Master2_TOC		BITMASK32(23, 16)	// AHB4 Master2 Time-Out Counter
#define Master1_TOC		BITMASK32(15, 8)	// AHB4 Master1 Time-Out Counter
#define Master0_TOC		BITMASK32(7, 0)	// AHB4 Master0 Time-Out Counter

// REG_AHB4_TOC1        (GCR_BA+0x44)   // R/W  AHB4 Master's 1st Time-Out Counter Control Register
#define Master7_TOC		BITMASK32(31, 24)	// AHB4 Master7 Time-Out Counter
#define Master6_TOC		BITMASK32(23, 16)	// AHB4 Master6 Time-Out Counter
#define Master5_TOC		BITMASK32(15, 8)	// AHB4 Master5 Time-Out Counter
#define Master4_TOC		BITMASK32(7, 0)	// AHB4 Master4 Time-Out Counter

// REG_CDCVC            (GCR_BA+0x48)   // R/W  AHB4 Master's 1st Time-Out Counter Control Register
#define YEAR		BITMASK32(31, 24)	// Year Code (8 bits)
#define MONTH		BITMASK32(23, 16)	// Month Code (8 bits)
#define DAY		BITMASK32(15, 8)	// Day Code (8 bits)
#define VERSION		BITMASK32(7, 0)	// Version Code (8 bits)

// REG_POR_LVRD (GCR_BA+0x74)   // R/W  POR and LVRD Control Register
#define MPLL_LKDT	BIT32(7)	// MPLL lock status
#define UPLL_LKDT	BIT32(6)	// UPLL lock status
#define APLL_LKDT	BIT32(5)	// APLL lock status
#define POR_ENB		BIT32(4)	// Power on circuit enable bar: active low (default low)
#define EN_LVR		BIT32(3)	// LVR enable: active high (default high)
#define EN_LVD		BIT32(2)	// LVR enable: active high (default high)
#define LVD_SEL		BIT32(1)	// LVD enable: active high (default low)
#define LVD_OUTB	BIT32(0)	// LVD flag. read only

// REG_GPAFUN0  (GCR_BA+0x80)   // R/W  Multi Function Pin Control Register 0
#define MF_GPA7		BITMASK32(31, 28)	// GPA[7] Multi Function
#define MF_GPA6		BITMASK32(27, 24)	// GPA[6] Multi Function
#define MF_GPA5		BITMASK32(23, 20)	// GPA[5] Multi Function
#define MF_GPA4		BITMASK32(19, 16)	// GPA[4] Multi Function
#define MF_GPA3		BITMASK32(15, 12)	// GPA[3] Multi Function
#define MF_GPA2		BITMASK32(11, 8)	// GPA[2] Multi Function
#define MF_GPA1		BITMASK32(7, 4)	// GPA[1] Multi Function
#define MF_GPA0		BITMASK32(3, 0)	// GPA[0] Multi Function

// REG_GPAFUN1  (GCR_BA+0x84)   // R/W  Multi Function Pin Control Register 0
#define MF_GPA15	BITMASK32(31, 28)	// GPA[15] Multi Function
#define MF_GPA14	BITMASK32(27, 24)	// GPA[14] Multi Function
#define MF_GPA13	BITMASK32(23, 20)	// GPA[13] Multi Function
#define MF_GPA12	BITMASK32(19, 16)	// GPA[12] Multi Function
#define MF_GPA11	BITMASK32(15, 12)	// GPA[11] Multi Function
#define MF_GPA10	BITMASK32(11, 8)	// GPA[10] Multi Function
#define MF_GPA9		BITMASK32(7, 4)	// GPA[9] Multi Function
#define MF_GPA8		BITMASK32(3, 0)	// GPA[8] Multi Function

// REG_GPBFUN0  (GCR_BA+0x88)   // R/W  Multi Function Pin Control Register 0

#define MF_GPB7		BITMASK32(31, 28)	// GPB[7] Multi Function
#define MF_GPB6		BITMASK32(27, 24)	// GPB[6] Multi Function
#define MF_GPB5		BITMASK32(23, 20)	// GPB[5] Multi Function
#define MF_GPB4		BITMASK32(19, 16)	// GPB[4] Multi Function
#define MF_GPB3		BITMASK32(15, 12)	// GPB[3] Multi Function
#define MF_GPB2		BITMASK32(11, 8)	// GPB[2] Multi Function
#define MF_GPB1		BITMASK32(7, 4)	// GPB[1] Multi Function
#define MF_GPB0		BITMASK32(3, 0)	// GPB[0] Multi Function

// REG_GPBFUN1  (GCR_BA+0x8C)   // R/W  Multi Function Pin Control Register 0
#define MF_GPB15	BITMASK32(31, 28)	// GPB[15] Multi Function
#define MF_GPB14	BITMASK32(27, 24)	// GPB[14] Multi Function
#define MF_GPB13	BITMASK32(23, 20)	// GPB[13] Multi Function
#define MF_GPB12	BITMASK32(19, 16)	// GPB[12] Multi Function
#define MF_GPB11	BITMASK32(15, 12)	// GPB[11] Multi Function
#define MF_GPB10	BITMASK32(11, 8)	// GPB[10] Multi Function
#define MF_GPB9		BITMASK32(7, 4)	// GPB[9] Multi Function
#define MF_GPB8		BITMASK32(3, 0)	// GPB[8] Multi Function

// REG_GPCFUN0  (GCR_BA+0x90)   // R/W  Multi Function Pin Control Register 0
#define MF_GPC7		BITMASK32(31, 28)	// GPC[7] Multi Function
#define MF_GPC6		BITMASK32(27, 24)	// GPC[6] Multi Function
#define MF_GPC5		BITMASK32(23, 20)	// GPC[5] Multi Function
#define MF_GPC4		BITMASK32(19, 16)	// GPC[4] Multi Function
#define MF_GPC3		BITMASK32(15, 12)	// GPC[3] Multi Function
#define MF_GPC2		BITMASK32(11, 8)	// GPC[2] Multi Function
#define MF_GPC1		BITMASK32(7, 4)	// GPC[1] Multi Function
#define MF_GPC0		BITMASK32(3, 0)	// GPC[0] Multi Function

// REG_GPCFUN1  (GCR_BA+0x94)   // R/W  Multi Function Pin Control Register 0
#define MF_GPC15	BITMASK32(31, 28)	// GPC[15] Multi Function
#define MF_GPC14	BITMASK32(27, 24)	// GPC[14] Multi Function
#define MF_GPC13	BITMASK32(23, 20)	// GPC[13] Multi Function
#define MF_GPC12	BITMASK32(19, 16)	// GPC[12] Multi Function
#define MF_GPC11	BITMASK32(15, 12)	// GPC[11] Multi Function
#define MF_GPC10	BITMASK32(11, 8)	// GPC[10] Multi Function
#define MF_GPC9		BITMASK32(7, 4)	// GPC[9] Multi Function
#define MF_GPC8		BITMASK32(3, 0)	// GPC[8] Multi Function

// REG_GPDFUN0  (GCR_BA+0x98)   // R/W  Multi Function Pin Control Register 0
#define MF_GPD7		BITMASK32(31, 28)	// GPD[7] Multi Function
#define MF_GPD6		BITMASK32(27, 24)	// GPD[6] Multi Function
#define MF_GPD5		BITMASK32(23, 20)	// GPD[5] Multi Function
#define MF_GPD4		BITMASK32(19, 16)	// GPD[4] Multi Function
#define MF_GPD3		BITMASK32(15, 12)	// GPD[3] Multi Function
#define MF_GPD2		BITMASK32(11, 8)	// GPD[2] Multi Function
#define MF_GPD1		BITMASK32(7, 4)	// GPD[1] Multi Function
#define MF_GPD0		BITMASK32(3, 0)	// GPD[0] Multi Function

// REG_GPDFUN1  (GCR_BA+0x9C)   // R/W  Multi Function Pin Control Register 0
#define MF_GPD15	BITMASK32(31, 28)	// GPD[15] Multi Function
#define MF_GPD14	BITMASK32(27, 24)	// GPD[14] Multi Function
#define MF_GPD13	BITMASK32(23, 20)	// GPD[13] Multi Function
#define MF_GPD12	BITMASK32(19, 16)	// GPD[12] Multi Function
#define MF_GPD11	BITMASK32(15, 12)	// GPD[11] Multi Function
#define MF_GPD10	BITMASK32(11, 8)	// GPD[10] Multi Function
#define MF_GPD9		BITMASK32(7, 4)	// GPD[9] Multi Function
#define MF_GPD8		BITMASK32(3, 0)	// GPD[8] Multi Function

// REG_GPEFUN0  (GCR_BA+0xA0)   // R/W  Multi Function Pin Control Register 0
#define MF_GPE7		BITMASK32(31, 28)	// GPE[7] Multi Function
#define MF_GPE6		BITMASK32(27, 24)	// GPE[6] Multi Function
#define MF_GPE5		BITMASK32(23, 20)	// GPE[5] Multi Function
#define MF_GPE4		BITMASK32(19, 16)	// GPE[4] Multi Function
#define MF_GPE3		BITMASK32(15, 12)	// GPE[3] Multi Function
#define MF_GPE2		BITMASK32(11, 8)	// GPE[2] Multi Function
#define MF_GPE1		BITMASK32(7, 4)	// GPE[1] Multi Function
#define MF_GPE0		BITMASK32(3, 0)	// GPE[0] Multi Function

// REG_GPEFUN1  (GCR_BA+0xA4)   // R/W  Multi Function Pin Control Register 0
#define MF_GPE15	BITMASK32(31, 28)	// GPE[15] Multi Function
#define MF_GPE14	BITMASK32(27, 24)	// GPE[14] Multi Function
#define MF_GPE13	BITMASK32(23, 20)	// GPE[13] Multi Function
#define MF_GPE12	BITMASK32(19, 16)	// GPE[12] Multi Function
#define MF_GPE11	BITMASK32(15, 12)	// GPE[11] Multi Function
#define MF_GPE10	BITMASK32(11, 8)	// GPE[10] Multi Function
#define MF_GPE9		BITMASK32(7, 4)	// GPE[9] Multi Function
#define MF_GPE8		BITMASK32(3, 0)	// GPE[8] Multi Function

// REG_MISFUN   (GCR_BA+0xA8)   // R/W  Miscellaneous Multi Function Control Register
#define MF_I2S		BIT32(0)	// I2S I/F Functional Selection

// REG_SPI_LVD_GPH      (GCR_BA+0xAC)           // R/W  PI[3:2] And LVDATA[23:18] (GPIO H) Control Register
#define DS_SPI_GPH	BITMASK32(23, 22)	// SPI_DATA[3:2] (GPIO_H[7:6]) Pin Driver Strength Control
#define DS_LVD_GPH	BITMASK32(21, 16)	// LVDATAT[23:18] (GPIO_H[5:0]) Pin Driver Strength Control
#define SL_SPI_GPH	BITMASK32(7, 6)	// SPI_DATA[3:2] (GPIO_H[7:6]) Pin Slew Rate Control
#define SL_LVD_GPH	BITMASK32(5, 0)	// LVDATA[23:18] (GPIO_H[5:0]) Pin Slew Rate Control

// REG_MISCPCR  (GCR_BA+0xB0)   // R/W  Miscellaneous Pin Control Register
#define SL_MD		BIT32(7)	// MD Pin Slew Rate Control
#define SL_MA		BIT32(6)	// MA Pin Slew Rate Control
#define SL_MCTL		BIT32(5)	// Memory I/F Control Pin Slew Rate Control
#define SL_MCLK		BIT32(4)	// MCLK Pin Rate Control
#define DS_MD		BIT32(3)	// MD Pins Driving Strength Control
#define DS_MA		BIT32(2)	// MA Pins Driving Strength Control
#define DS_MCTL		BIT32(1)	// MCTL Pins Driving Strength Control
#define DS_MCLK		BIT32(0)	// MCLK Pins Driving Strength Control

// REG_MISC_SL_GPA      (GCR_BA+0xB4)   // R/W  GPIO A Slew Rate Control Register
#define SL_GPA		BITMASK32(11, 0)	// GPA Pins Slew Rate Control
// REG_MISC_SL_GPB      (GCR_BA+0xB8)   // R/W  GPIO B Slew Rate Control Register
#define SL_GPB		BITMASK32(15, 0)	// GPB Pins Slew Rate Control
// REG_MISC_SL_GPC      (GCR_BA+0xBC)   // R/W  GPIO C Slew Rate Control Register
#define SL_GPC		BITMASK32(15, 0)	// GPC Pins Slew Rate Control
// REG_MISC_SL_GPD      (GCR_BA+0xC0)   // R/W  GPIO D Slew Rate Control Register
#define SL_GPD		BITMASK32(15, 0)	// GPD Pins Slew Rate Control
// REG_MISC_SL_GPE      (GCR_BA+0xC4)   // R/W  GPIO E Slew Rate Control Register
#define SL_GPE		BITMASK32(11, 0)	// GPE Pins Slew Rate Control
// REG_MISC_SL_ND       (GCR_BA+0xC8)   // R/W  NAND Data PAD Slew Rate Control Register
#define SL_ND		BITMASK32(7, 0)	// NAND Data Pin Slew Rate Control

// REG_MISC_DS_GPA      (GCR_BA+0xCC)   // R/W  GPIO A Driver Strength Control Register
#define DS_GPA		BITMASK32(11, 0)	// Driver Strength Control
// REG_MISC_DS_GPB      (GCR_BA+0xD0)   // R/W  GPIO B Driver Strength Control Register
#define DS_GPB		BITMASK32(15, 0)	// Driver Strength Control
// REG_MISC_DS_GPC      (GCR_BA+0xD4)   // R/W  GPIO C Driver Strength Control Register
#define DS_GPC		BITMASK32(15, 0)	// Driver Strength Control
// REG_MISC_DS_GPD      (GCR_BA+0xD8)   // R/W  GPIO D Driver Strength Control Register
#define DS_GPD		BITMASK32(15, 0)	// Driver Strength Control
// REG_MISC_DS_GPE      (GCR_BA+0xDC)   // R/W  GPIO E Driver Strength Control Register
#define DS_GPE		BITMASK32(11, 0)	// Driver Strength Control
// REG_MISC_DS_ND       (GCR_BA+0xE0)   // R/W  NAND Data PAD Driver Strength Control Register
#define DS_ND		BITMASK32(7, 0)	// NAND Data Pin Driver Strength Control

// REG_MISC_SSEL        (GCR_BA+0xE4)
#define SDRAM_MCLK	BITMASK32(9, 8)	// MCLK Pins Driving Strength Control and Mode.
#define SDRAM_MA	BITMASK32(7, 6)	// MA Pins Driving Strength Control and Mode.
#define SDRAM_MBA	BITMASK32(5, 4)	// MBA Pins Driving Strength  Control and Mode.
#define SDRAM_MCTL	BITMASK32(3, 2)	// MCTL Pins Driver Strength Control and Mode.
#define SDRAM_MD	BITMASK32(1, 0)	// MD Data Pins Driver Strength Control and Mode

// REG_GPGFUN0  (GCR_BA+0xE8)   // R/W  Multi Function Pin Control Register 0
#define MF_GPG7		BITMASK32(31, 28)	// GPG[7] Multi Function
#define MF_GPG6		BITMASK32(27, 24)	// GPG[6] Multi Function
#define MF_GPG5		BITMASK32(23, 20)	// GPG[5] Multi Function
#define MF_GPG4		BITMASK32(19, 16)	// GPG[4] Multi Function
#define MF_GPG3		BITMASK32(15, 12)	// GPG[3] Multi Function
#define MF_GPG2		BITMASK32(11, 8)	// GPG[2] Multi Function
#define MF_GPG1		BITMASK32(7, 4)	// GPG[1] Multi Function
#define MF_GPG0		BITMASK32(3, 0)	// GPG[0] Multi Function

// REG_GPGFUN1  (GCR_BA+0xEC)   // R/W  Multi Function Pin Control Register 0
#define MF_GPG15	BITMASK32(31, 28)	// GPG[15] Multi Function
#define MF_GPG14	BITMASK32(27, 24)	// GPG[14] Multi Function
#define MF_GPG13	BITMASK32(23, 20)	// GPG[13] Multi Function
#define MF_GPG12	BITMASK32(19, 16)	// GPG[12] Multi Function
#define MF_GPG11	BITMASK32(15, 12)	// GPG[11] Multi Function
#define MF_GPG10	BITMASK32(11, 8)	// GPG[10] Multi Function
#define MF_GPG9		BITMASK32(7, 4)	// GPG[9] Multi Function
#define MF_GPG8		BITMASK32(3, 0)	// GPG[8] Multi Function

// REG_GPHFUN   (GCR_BA+0xF0)   // R/W  Dummy GPIO
#define MF_GPH15	BITMASK32(31, 30)	// GPH[15] Multi Function
#define MF_GPH14	BITMASK32(29, 28)	// GPH[14] Multi Function
#define MF_GPH13	BITMASK32(27, 26)	// GPH[13] Multi Function
#define MF_GPH12	BITMASK32(25, 24)	// GPH[12] Multi Function
#define MF_GPH11	BITMASK32(23, 22)	// GPH[11] Multi Function
#define MF_GPH10	BITMASK32(21, 20)	// GPH[10] Multi Function
#define MF_GPH9		BITMASK32(19, 18)	// GPH[9] Multi Function
#define MF_GPH8		BITMASK32(17, 16)	// GPH[8] Multi Function
#define MF_GPH7		BITMASK32(15, 14)	// GPH[7] Multi Function
#define MF_GPH6		BITMASK32(13, 12)	// GPH[6] Multi Function
#define MF_GPH5		BITMASK32(11, 10)	// GPH[5] Multi Function
#define MF_GPH4		BITMASK32(9, 8)	// GPH[4] Multi Function
#define MF_GPH3		BITMASK32(7, 6)	// GPH[3] Multi Function
#define MF_GPH2		BITMASK32(5, 4)	// GPH[2] Multi Function
#define MF_GPH1		BITMASK32(3, 2)	// GPH[1] Multi Function
#define MF_GPH0		BITMASK32(1, 0)	// GPH[0] Multi Function

// REG_SHRPIN_TVDAC     (GCR_BA+0xF4)   // R/W  Share Pin With TV DAC
#define SMTVDACAEN	BIT32(31)

// REG_SHRPIN_AUDIO     (GCR_BA+0xF8)   // R/W  Share Pins with AUDIO ADC
#define MIC_AEN		BIT32(31)	// Analog and Digital Share I/O Pad Control Bit for SAR-ADC MICP, MICN
#define AIN2_AEN	BIT32(30)	// Analog and Digital Share I/O Pad Control Bit for SAR-ADC AIN2 Pins
#define AIN3_AEN	BIT32(29)	// Analog and Digital Share I/O Pad Control Bit for SAR-ADC AIN3 Pins
#define AIN4_AEN	BIT32(28)	// Analog and Digital Share I/O Pad Control Bit for SAR-ADC AIN4 Pins

// REG_SHRPIN_TOUCH     (GCR_BA+0xFC)   // R/W  Share Pins with TOUCH ADC
#define SAR_AHS_AEN	BIT32(31)	// Analog and Digital Share I/O Pad Control Bit for SAR-ADC AHS Pins.
#define TP_AEN		BIT32(30)	// Analog and Digital Share I/O Pad Control Bit for SAR-ADC XP, XM, YP, YM Pins
#define MIC_BIAS_AEN	BIT32(29)	// Analog and Digital Share I/O Pad Control Bit for SAR-ADC MIC_BIAS Pin

// REG_EXTRST_DEBOUNCE  (GCR_BA+0x100)  // R/W  External RESET Debounce Control Register
#define EXTRST_DEBOUNCE	BIT32(0)	// External RESET Debounce Control

// REG_DEBOUNCE_CNTR    (GCR_BA+0x104)  // R/W   External RESET Debounce Counter Register
#define DEBOUNCE_CNTR	BITMASK32(15, 0)	// External RESET Debounce Counter

#endif
