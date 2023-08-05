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

#ifndef _N9H26_AIC_H_
#define _N9H26_AIC_H_

/*
 * All registers are 32 bit wide
 */

#define N9H26AIC_BASE 0xB8000000UL

/*
 * The following table lists all interrupts from various peripheral interface modules or external devices.
 * Channel    Name         Mode               Source
 *    1       WDT_INT      Positive Level     Watch Dog Timer Interrupt
 *    2       GPIO_INT0    Positive Level     GPIO Interrupt 0
 *    3       GPIO_INT1    Positive Level     GPIO Interrupt 1
 *    4       GPIO_INT2    Positive Level     GPIO Interrupt 2
 *    5       GPIO_INT3    Positive Level     GPIO Interrupt 3
 *    6       IPSEC_INT    Positive Level     AES Interrupt
 *    7       SPU_INT      Positive Level     SPU Interrupt
 *    8       I2S_INT      Positive Level     I2S Interrupt
 *    9       VPOST_INT    Positive Level     VPOST Interrupt
 *   10       VIN0_INT     Positive Level     Video In 0 Interrupt
 *   11       MDCT_INT     Positive Level     MDCT Interrupt
 *   12       BLT_INT      Positive Level     BLT Interrupt
 *   13       GVE_VPE_INT  Positive Level     Graphics Video Engine Interrupt
 *   14       HUART_INT    Positive Level     High Speed UART Interrupt
 *   15       TMR0_INT     Positive Level     Timer 0 Interrupt
 *   16       TMR1_INT     Positive Level     Timer 1 Interrupt
 *   17       UDC_INT      Positive Level     USB Device Controller Interrupt
 *   18       SIC_INT      Positive Level     Storage Interrupt Controller Interrupt
 *   19       SDIO_INT     Positive Level     Secure Digital Input / Output Control Interrupt
 *   20       UHC_INT      Positive Level     USB Host Controller Interrupt
 *   21       EHCI_INT     Positive Level     Enhanced Host Controller Interface Interrupt
 *   22       OHCI_INT     Positive Level     Host Controller Interface Interrupt
 *   23       EDMA0_INT    Positive Level     Enhanced DMA 0 Interrupt
 *   24       EDMA1_INT    Positive Level     Enhanced DMA 1 Interrupt
 *   25       SPIMS0_INT   Positive Level     SPI Master / Slave 0 Interrupt
 *   26       SPIMS1_INT   Positive Level     SPI Master / Slave 1 Interrupt
 *   27       AUDIO_INT    Positive Level     Audio Record Interrupt
 *   28       TOUCH_INT    Positive Level     Touch Controller Interrupt
 *   29       RTC_INT      Positive Level     RTC Interrupt
 *   30       UART_INT     Positive Level     UART Interrupt
 *   31       PWM_INT      Positive Level     PWM Interrupt
 *   32       JPG_INT      Positive Level     JPEG Codec Interrupt
 *   33       VDE_INT      Positive Level     H264 Decode Interrupt
 *   34       VEN_INT      Positive Level     H264 Encode Interrupt
 *   35       SDIC_INT     Positive Level     SDIC Interrupt
 *   36       EMCTX_INT    Positive Level     EMC TX Interrupt
 *   37       EMCRX_INT    Positive Level     EMC RX Interrupt
 *   38       I2C_INT      Positive Level     I2C Interrupt
 *   39       KPI_INT      Positive Level     Keypad Interrupt
 *   40       RSC_INT      Positive Level     RS Codec Interrupt
 *   41       VTB_INT      Positive Level     Convolution / Viterbi Codec Interrupt
 *   42       ROT_INT      Positive Level     Rotate Engine Interrupt
 *   43       PWR_INT      Positive Level     System Wake-Up Interrupt
 *   44       LVD_INT      Positive Level     Low Voltage Detector Interrupt
 *   45       VIN1_INT     Positive Level     Video In 1 Interrupt
 *   46       TMR2_INT     Positive Level     Timer 2 Interrupt
 *   47       TMR3_INT     Positive Level     Timer 3 Interrupt
 */

enum N9H26AIC {
	AIC_SCR1,
	AIC_SCR2,
	AIC_SCR3,
	AIC_SCR4,
	AIC_SCR5,
	AIC_SCR6,
	AIC_SCR7,
	AIC_SCR8,
	AIC_SCR9,
	AIC_SCR10,
	AIC_SCR11,
	AIC_SCR12,
	AIC_IRSR = 64,
	AIC_ISRSRH,
	AIC_IASR,
	AIC_IASRH,
	AIC_ISR,
	AIC_ISRH,
	AIC_IPER,
	AIC_ISNR = 72,
	AIC_OISR,
	AIC_IMR,
	AIC_IMRH,
	AIC_MECR,
	AIC_MECRH,
	AIC_MDCR,
	AIC_MDCRH,
	AIC_SSCR,
	AIC_SSCRH,
	AIC_SCCR,
	AIC_SCCRH,
	AIC_EOSCR,
	AIC_TEST = 88
};

#endif
