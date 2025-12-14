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

#ifndef _N9H26_CLK_H_
#define _N9H26_CLK_H_

#include <types_common.h>

/*
 * All registers are 32 bit wide
 */

#define N9H26CLK_BASE 0xB0000200UL

enum N9H26CLK {
	PWRCON,
	AHBCLK,
	APBCLK,
	CLKDIV0,
	CLKDIV1,
	CLKDIV2,
	CLKDIV3,
	CLKDIV4,
	APLLCON,
	UPLLCON,
	MPLLCON,
	CLK_TREG,
	AHBCLK2,
	CLKDIV5,
	CLKDIV6,
	CLKDIV7,
	CLKDIV8
};

/*
 * Clock controller Registers
 */
// REG_PWRCON   (CLK_BA+0x00)   // R/W  System Power Down Control Register
#define PRE_SCALAR	BITMASK32(23, 8)	// Pre-Scalar counter
#define SEN1_OFF_ST	BIT32(5)	// Sensor 1 clock level if clock off state
#define SEN0_OFF_ST	BIT32(4)	// Sensor 0 clock level if clock off state
#define INT_EN		BIT32(3)	// Power On Interrupt Enable
#define INTSTS		BIT32(2)	// Power Down interrupt status
#define XIN_CTL		BIT32(1)	// Crystal pre-divide control for Wake-up from power down mode
#define XTAL_EN		BIT32(0)	// Crystal (Power Down) Control

// REG_AHBCLK   (CLK_BA+0x04)   // R/W¡@AHB Clock Enable Control Register
#define SDIO_CKE	BIT32(31)	// SDIO Clock Enable Control
#define ADO_CKE		BIT32(30)	// Audio DAC Engine Clock Enable Control0 = Disable1 = Enable
#define SEN_CKE		BIT32(29)	// Sensor Interface Clock Enable Control0 = Disable1 = Enable
#define SEN0_CKE	SEN_CKE
#define VIN_CKE		BIT32(28)	// Capture Clock Enable Control (Also is Capture engine clock enable control)0 = Disable1 = Enable
#define VIN0_CKE	VIN_CKE
#define VPOST_CKE	BIT32(27)	// VPOST Clock Enable Control (Also is VPOST engine clock enable control)0 = Disable1 = Enable
#define I2S_CKE		BIT32(26)	// I2S Controller Clock Enable Control0 = Disable1 = Enable
#define SPU_CKE		BIT32(25)	// SPU Clock Enable Control0 = Disable1 = Enable
#define HCLK4_CKE	BIT32(24)	// HCLK4 Clock Enable Control0 = Disable1 = Enable
#define SD_CKE		BIT32(23)	// SD Card Controller Engine Clock Enable Control0 = Disable1 = ENable
#define NAND_CKE	BIT32(22)	// NAND Controller Clock Enable Control0 = Disable1 = ENable
#define SIC_CKE		BIT32(21)	// SIC Clock Enable Control0 = Disable1 = ENable
	//#define OVG_CKE                       BIT20                   // Graphic Processing Unit Clock Enable Control0 = Disable1 = ENable
#define GVE_CKE		BIT32(19)	// GE4P Clock Enable Control0 = Disable1 = ENable
#define USBD_CKE	BIT32(18)	// USB Device Clock Enable Control0 = Disable1 = Enable
#define USBH_CKE	BIT32(17)	// USB Host Controller Clock Enable Control0 = Disable1 = Enable
#define HCLK3_CKE	BIT32(16)	// HCLK3 Clock Enable Control.0 = Disable1 = Enable
#define VDE_CKE		BIT32(15)	// Video Decoder Clock Enable Control
#define EDMA4_CKE	BIT32(14)	// EDMA Controller Channel 4 Clock Enable Control
#define EDMA3_CKE	BIT32(13)	// EDMA Controller Channel 3 Clock Enable Control
#define EDMA2_CKE	BIT32(12)	// EDMA Controller Channel 2 Clock Enable Control
#define EDMA1_CKE	BIT32(11)	// EDMA Controller Channel 1 Clock Enable Control
#define EDMA0_CKE	BIT32(10)	// EDMA Controller Channel 0 Clock Enable Control
#define IPSEC_CKE	BIT32(9)	// AES Clock Enable Control. 0 = Disable; 1 = Enable.
#define HCLK1_CKE	BIT32(8)	// HCLK1 Clock Enable Control.0 = Disable1 = Enable
#define JPG_CKE		BIT32(7)	// JPEG Clock Enable
#define VPE_CKE		BIT32(6)	// Video Engine Clock Enable Control
#define BLT_CKE		BIT32(5)	// Bitblt Engine Clock Enable Control
#define DRAM_CKE	BIT32(4)	// SDRAM and SDRAM Controller Clock Enable Control.0 = Disable1 = Enable
#define SRAM_CKE	BIT32(3)	// SRAM Controller Clock Enable Control.0 = Disable1 = Enable
#define HCLK_CKE	BIT32(2)	// HCLK Clock Enable Control. (This clock is used for DRAM controller, SRAM controller and AHB-to-AHB bridge)0 = Disable1 = Enable
#define APBCLK_CKE	BIT32(1)	// APB Clock Enable Control.0 = Disable1 = Enable
#define CPU_CKE	 	BIT32(0)	// CPU Clock Enable Control

// REG_APBCLK   (CLK_BA+0x08)   // R/W¡@APB Clock Enable Control Register
#define KPI_CKE		BIT32(25)	// KPI Clock Enable Control
#define TIC_CKE		BIT32(24)	// TIC Clock Enable
#define TMR3_CKE	BIT32(17)	//Timer3 Clock Enable Control
#define TMR2_CKE	BIT32(16)	//Timer2 Clock Enable Control
#define WDCLK_CKE	BIT32(15)	// Watch Dog Clock Enable Control (Also is Watch Dog engine clock enable control)
#define RSC_CKE		BIT32(12)	// Reed-Solomon Codec Clock Enable Control
#define CONVENC_CKE	BIT32(11)	// Convolution Encoder Clock Enable Control
#define TOUCH_CKE	BIT32(10)	// TOUCH Clock Enable Control
#define TMR1_CKE	BIT32(9)	// Timer1 Clock Enable Control0 = Disable1 = Enable
#define TMR0_CKE	BIT32(8)	// Timer0 Clock Enable Control0 = Disable1 = Enable
#define SPIMS1_CKE	BIT32(7)	// SPIM (Master Only) Clock Enable Control0 = Disable1 = Enable
#define SPIMS0_CKE	BIT32(6)	// SPIMS (Master / Slave) Clock Enable Control0 = Disable1 = Enable
#define PWM_CKE		BIT32(5)	// PWM Clock Enable Control0 = Disable1 = Enable
#define UART1_CKE	BIT32(4)	// UART1 Clock Enable Control0 = Disable1 = Enable
#define UART0_CKE	BIT32(3)	// UART0 Clock Enable Control0 = Disable1 = Enable
#define RTC_CKE		BIT32(2)	// RTC Clock Enable Control (NOT X32K clock enable control)0 = Disable1 = Enable
#define I2C_CKE		BIT32(1)	// I2C Clock Enable Control0 = Disable1 = Enable
#define ADC_CKE		BIT32(0)	// ADC Clock Enable Control (Also is ADC engine clock enable control)0 = Disable1 = Enable

// REG_CLKDIV0  (CLK_BA+0x0C)   // R/W¡@Clock Divider Register0
#define SENSOR0_N1	BITMASK32(27, 24)	// Sensor clock divide number from sensor clock source
#define SENSOR0_S	BITMASK32(23, 22)	// Sensor clock source select 00 = XIN. 01 = X32K. 10 = APLL. 11 = UPLL
#define SENSOR0_N0	BITMASK32(21, 19)	// Sensor clock pre-divider number from Sensor clock source if Sensor clock source select is APLL or UPLL
#define KPI_N0		BITMASK32(18, 12)	// KPI Engine Clock Divider Bits [3:0]
#define SYSTEM_FRACTL	BITMASK32(11, 8)	// SYSTEM clock divide number from system clock source
#define SYSTEM_N1	SYSTEM_FRACTL	// SYSTEM clock divide number from system clock source
#define KPI_S		BIT32(5)	// KPI Engine Clock Source Selection
#define SYSTEM_S	BITMASK32(4, 3)	// System clock source select 00 = XIN. 01 = X32K. 10 = APLL. 11 = UPLL
#define SYSTEM_N0	BITMASK32(2, 0)	// SYSTEM clock pre-divider number from system clock source if System clock source select is APLL or UPLL

// REG_CLKDIV1  (CLK_BA+0x10)   // R/W¡@Clock Divider Register1
#define ADO_N1		BITMASK32(31, 24)	// Audio DAC engine clock divide number from Audio DAC engine clock source
#define ADO_S		BITMASK32(20, 19)	// Audio DAC engine clock source select 00 = XIN. 01 = X32K. 10 = APLL. 11 = UPLL
#define ADO_N0		BITMASK32(18, 16)	// Audio DAC engine clock pre-divide number
#define VPOST_N1	BITMASK32(15, 8)	// VPOST engine clock divide number from VPOST engine clock source
#define VPOST_S		BITMASK32(4, 3)	// VPOST engine clock source select 00 = XIN. 01 = X32K. 10 = APLL. 11 = UPLL
#define VPOST_N0	BITMASK32(2, 0)	// VPOST engine clock pre-divide number

// REG_CLKDIV2  (CLK_BA+0x14)   // R/W¡@Clock Divider Register2
#define SD_N1		BITMASK32(31, 24)	// SD engine clock divide number from SD engine clock source
#define SD_S		BITMASK32(20, 19)	// SD engine clock source select  00 = XIN. 01 = X32K. 10 = APLL. 11 = UPLL
#define SD_N0		BITMASK32(18, 16)	// SD engine clock pre-divide number
#define U20PHY_SS	BITMASK32(22, 21)	// USB Device
#define U20PHY_N	BITMASK32(15, 12)	// USB Device
#define USB_N1		BITMASK32(11, 8)	// USB host engine clock divide number from USB engine clock source
#define U20PHY_DS	BITMASK32(7, 5)	// USB Device
#define USB_S		BITMASK32(4, 3)	// USB host engine clock source select 00 = XIN. 01 = X32K. 10 = APLL. 11 = UPLL
#define USB_N0		BITMASK32(2, 0)	// USB host engine clock Pre-divide number

// REG_CLKDIV3  (CLK_BA+0x18)   // R/W¡@Clock Divider Register3
#define ADC_N1		BITMASK32(31, 24)	// ADC engine clock divide number from ADC engine clock source
#define ADC_S		BITMASK32(20, 19)	// ADC engine clock source select 00 = XIN. 01 = X32K. 10 = APLL. 11 = UPLL
#define ADC_N0		BITMASK32(18, 16)	// ADC engine clock pre-divide number from ADC engine clock source
#define UART1_N1	BITMASK32(15, 13)	// UART1 engine clock divide number from UART1 engine clock source
#define UART1_S		BITMASK32(12, 11)	// UART1 engine clock source select 00 = XIN. 01 = X32K. 10 = APLL. 11 = UPLL
#define UART1_N0	BITMASK32(10, 8)	// UART1 engine clock pre-divide number from UART1 engine clock source
#define UART0_N1	BITMASK32(7, 5)	// UART0 engine clock divide number from UART1 engine clock source
#define UART0_S		BITMASK32(4, 3)	// UART0 engine clock source select 00 = XIN. 01 = X32K. 10 = APLL. 11 = UPLL
#define UART0_N0	BITMASK32(2, 0)	// UART0 engine clock pre-divide number from UART1 engine clock source

// REG_CLKDIV4  (CLK_BA+0x1C)   // R/W¡@Clock Divider Register4
#define VIN1_N		BITMASK32(29, 27)	// JPG  engine clock divide number from HCLK4
#define JPG_N		BITMASK32(26, 24)	// JPG  engine clock divide number from HCLK3
#define GPIO_N		BITMASK32(23, 17)	// GPIO engine clock divide number from GPIO engine clock source
#define CAP_N		BITMASK32(14, 12)	// Capture engine clock divide number from HCLK4 clock.Engine Clock frequency = HCLK4 / (CAP_N + 1)
#define VIN0_N		CAP_N
#define CHG_APB		BIT32(11)
#define APB_N		BITMASK32(10, 8)	// APB clock divide number from HCLK1 clock. The HCLK1 clock frequency is the lower of system clock divided by 2 or the CPU clockThe APB clock frequency = (HCLK1 frequency) / (APB_N + 1)
#define HCLK234_N	BITMASK32(7, 4)	// HCLK2, HCLK3 and HCLK4 clock divide number from HCLK clock. The HCLK clock frequency is the system clock frequency divided by two.The HCLK2,3,4 clock frequency = (HCLK frequency) / (HCLK234_N + 1)
#define CPU_N		BITMASK32(3, 0)	// CPU clock divide number from System clock.The CPU clock frequency = (System frequency) / (CPU_N + 1)

// REG_APLLCON  (CLK_BA+0x20)   // R/W¡@APLL Control Register
// REG_UPLLCON  (CLK_BA+0x24)   // R/W¡@UPLL Control Register
// REG_MPLLCON  (CLK_BA+0x28)   // R/W¡@UPLL Control Register
#define BP		BIT32(15)	// PLL By Pass Control
#define PD		BIT32(14)	// Power Down Mode
#define OUT_DV		BITMASK32(12, 11)	// PLL Output Divider Control
#define IN_DV		BITMASK32(10, 7)	// PLL Input Divider Control
#define FB_DV		BITMASK32(6, 0)	// PLL Feedback Divider Control

// REG_CLK_TREG (CLK_BA+0x30)
#define TEST_CKE	BIT32(7)	// Test Colck Output Enable.
#define SW_CLK		BIT32(6)	// Software Generated Clock.
#define TEST_CLK_SEL	BITMASK32(5, 0)	// Test Clock Select.

// REG_AHBCLK2  (CLK_BA+0x34)   // R/W¡@AHB Clock Enable Control Register2
#define CRC1_CKE	BIT32(19)	// CRC1 Clock Enable Control
#define ROTE_CKE	BIT32(18)	// Rotate Engine Clock Enable Control
#define EDMA26_CKE	BIT32(17)	// EDMA#2 Controller Channel 6 Clock Enable Control
#define EDMA25_CKE	BIT32(16)	// EDMA#2 Controller Channel 5 Clock Enable Control
#define EDMA24_CKE	BIT32(15)	// EDMA#2 Controller Channel 4 Clock Enable Control
#define EDMA23_CKE	BIT32(14)	// EDMA#2 Controller Channel 3 Clock Enable Control
#define EDMA22_CKE	BIT32(13)	// EDMA#2 Controller Channel 2 Clock Enable Control
#define EDMA21_CKE	BIT32(12)	// EDMA#2 Controller Channel 1 Clock Enable Control
#define EDMA20_CKE	BIT32(11)	// EDMA#2 Controller Channel 0 Clock Enable Control
#define VENC_CKE	BIT32(10)	// H.264 encoder Clock Enable Control
#define VDEC_CKE	BIT32(9)	// H.264 decoder Clock Enable Control
#define AAC_CKE		BIT32(8)	// AAC Controller Clock enable control
#define EMAC_CKE	BIT32(7)	// EMAC Clock Enable Control
#define CRC_CKE		BIT32(6)	// CRC Clock Enable Control
#define H20PHY_CKE	BIT32(5)	// USB2.0 Host PHY Clock Enable Control
#define OHCI_CKE	BIT32(4)	// USB2.0 OHCI Clock Enable Control
#define SEN1_CKE	BIT32(3)	// EDMA Controller Channel 6 Clock Enable Control
#define VIN1_CKE	BIT32(2)	// EDMA Controller Channel 5 Clock Enable Control
#define EDMA6_CKE	BIT32(1)	// EDMA Controller Channel 6 Clock Enable Control
#define EDMA5_CKE	BIT32(0)	// EDMA Controller Channel 5 Clock Enable Control

// REG_CLKDIV5  (CLK_BA+0x38)   // R/W¡@AHB Clock Enable Control Register2
#define TOUCH_N1	BITMASK32(31, 27)	// TOUCH Engine Clock Divider Bits [4:0]
#define TOUCH_S		BITMASK32(26, 25)	// TOUCH Engine Clock Source Selectione Control
#define TOUCH_N0	BITMASK32(24, 22)	// TOUCH Engine Clock Divider If PLL
#define SENSOR1_N1	BITMASK32(21, 18)	// Sensor Clock Divider Bits [4:0]
#define SENSOR1_S	BITMASK32(17, 16)	// Sensor Clock Source Selectione Control
#define SENSOR1_N0	BITMASK32(15, 13)	// Sensor Clock Divider If PLL
#define PWM_N1		BITMASK32(12, 5)	// PWM Clock Divider Bits [4:0]
#define PWM_S		BITMASK32(4, 3)	// PWM Clock Source Selectione Control
#define PWM_N0		BITMASK32(2, 0)	// PWM Clock Divider If PLL

// REG_CLKDIV6  (CLK_BA+0x3C)   // R/W¡@AHB Clock Enable Control Register2
#define H20PHY_N1	BITMASK32(27, 24)	// USB2.0 Host PHY Clock Divider 1
#define H20PHY_S	BITMASK32(20, 19)	// H20PHY Clock Source Divide Selection
#define H20PHY_N0	BITMASK32(18, 16)	// USB2.0 Host PHY Clock Divider 0
#define OHCI_N1		BITMASK32(11, 8)	// OHCI Clock Divider 1
#define OHCI_S		BITMASK32(4, 3)	// OHCI Clock Source Selection
#define OHCI_N0		BITMASK32(2, 0)	// OHCI Clock Divider 0

// REG_CLKDIV7 (CLK_BA+0x40)    // R/W¡@Clock Divider Register7
#define I2S_N1		BITMASK32(23, 16)	// I2S engine clock divide number 1
#define I2S_S		BITMASK32(12, 11)	// I2S engine clock source select 00 = XIN. 01 = X32K. 10 = APLL. 11 = UPLL
#define I2S_N0		BITMASK32(10, 8)	// I2S engine clock pre-divide number
#define DRAM_N1		BITMASK32(7, 5)	// DRAM Clock Divider 1
#define DRAM_S		BITMASK32(4, 3)	// DRAM Clock
#define DRAM_N0		BITMASK32(2, 0)	// DRAM Clock Divider 0

// REG_CLKDIV8 (CLK_BA+0x44)    // R/W¡@Clock Divider Register8
#define SDIO_N1		BITMASK32(12, 5)	// SD engine clock divide number from SD engine clock source
#define SDIO_S		BITMASK32(4, 3)	// SD engine clock source select  00 = XIN. 01 = X32K. 10 = APLL. 11 = UPLL
#define SDIO_N0		BITMASK32(2, 0)	// SD engine clock pre-divide number

#endif
