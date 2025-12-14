/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2025 Diego Gallizioli
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

#ifndef _N9H26_INTERRUPTS_H_
#define _N9H26_INTERRUPTS_H_

#define NR_IRQ_VECTORS 48

/* Hardware interrupt numbers. */
#define WDT_INT 1
#define GPIO_INT0 2
#define GPIO_INT1 3
#define GPIO_INT2 4
#define GPIO_INT3 5
#define IPSEC_INT 6
#define SPU_INT 7
#define I2S_INT 8
#define VPOST_INT 9
#define VIN0_INT 10
#define MDCT_INT 11
#define BLT_INT 12
#define GVE_VPE_INT 13
#define HUART_INT 14
#define TMR0_INT 15
#define TMR1_INT 16
#define UDC_INT 17
#define SIC_INT 18
#define SDIO_INT 19
#define UHC_INT 20
#define EHCI_INT 21
#define OHCI_INT 22
#define EDMA0_INT 23
#define EDMA1_INT 24
#define SPIMS0_INT 25
#define SPIMS1_INT 26
#define AUDIO_INT 27
#define TOUCH_INT 28
#define RTC_INT 29
#define UART_INT 30
#define PWM_INT 31
#define JPG_INT 32
#define VDE_INT 33
#define VEN_INT 34
#define SDIC_INT 35
#define EMCTX_INT 36
#define EMCRX_INT 37
#define I2C_INT 38
#define KPI_INT 39
#define RSC_INT 40
#define VTB_INT 41
#define ROT_INT 42
#define PWR_INT 43
#define LVD_INT 44
#define VIN1_INT 45
#define TMR2_INT 46
#define TMR3_INT 47

#endif
