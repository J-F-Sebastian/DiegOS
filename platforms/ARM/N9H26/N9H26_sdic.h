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

#ifndef _N9H26_SDIC_H_
#define _N9H26_SDIC_H_

#include <types_common.h>

/*
 * All registers are 32 bit wide
 */

#define N9H26SDIC_BASE 0xB0003000UL

/*
 * This is the interface to SDRAM memory controller
 */

enum N9H26SDIC {
	SDOPM,
	SDCMD,
	SDREF,
	SDSIZE0 = 4,
	SDSIZE1,
	SDMR,
	SDEMR,
	SDEMR2,
	SDEMR3,
	SDTIME,
	CKDQSDS = 13,
	WDLLMODE = 21,
	RDLLMODE,
	DBGREG1 = 28
};

/*
 * SDRAM controller registers
 */
//#define REG_SDOPM     (SDRAM_BA + 0x00)       // R/W  SDRAM Controller Operation Mode Control Register
#define AUTO_DQSPHASE	BITMASK32(30, 28)	// DQS Phase Detector Output Value
#define RDDATASEL	BIT32(24)	// Enable Hardware Auto Wait Cycle
#define HW_RWC_EN	BIT32(22)	// Enable Hardware Auto-Read Wait Cycle Setting
#define PHASE_SHIFT_EN	BIT32(21)	// Select Shift half-cycle data to be sampled data
#define DQS_PHASE_RST	BIT32(20)	// DQS_PHASE_RESET signal
#define OEDELAY		BIT32(19)	// Output Enable Delay Half MCLK
#define LOWFREQ		BIT32(18)	// Low Frequency Mode
#define PREACTBNK     	BIT32(17)	// Pre-Active Bank
#define AUTOPDN       	BIT32(16)	// Auto Power Down Mode
#define SEL_SCLKI	BIT32(15)
#define RDBUFTH       	BITMASK32(10, 8)	// The AHB read SDRAM read buffer threshold control
#define SD_TYPE       	BITMASK32(6, 5)	// SDRAM Type
#define PCHMODE      	BIT32(4)	// SDRAM Type
#define OPMODE        	BIT32(3)	// Open Page Mode
#define MCLKMODE      	BIT32(2)	// Auto Pre-Charge Mode
#define DRAM_EN      	BIT32(1)	// SDRAM Controller Enable

//#define REG_SDCMD     (SDRAM_BA + 0x04)       // R/W  SDRAM Command Register
#define AUTOEXSELFREF	BIT32(5)	// Auto Exit Self-Refresh
#define SELF_REF       	BIT32(4)	// Self-Refresh Command
#define REF_CMD       	BIT32(3)	// Auto Refresh Command
#define PALL_CMD       	BIT32(2)	// Pre-Charge All Bank Command
#define CKE_H          	BIT32(1)	// CKE High
#define INITSTATE	BIT32(0)	// Initial State

//#define REG_SDREF     (SDRAM_BA + 0x08)       // R/W  SDRAM Controller Refresh Control Register
#define REF_EN		BIT32(15)	// Refresh Period Counter Enable
#define REFRATE		BITMASK32(14, 0)	// Refresh Count Value

//#define REG_SDSIZE0   (SDRAM_BA + 0x10)       // R/W  SDRAM 0 Size Register
//#define REG_SDSIZE1   (SDRAM_BA + 0x14)       // R/W  SDRAM 0 Size Register
#define BASADDR		BITMASK32(28, 21)	// Base Address
#define BUSWD		BIT32(3)	// SDRAM Data Bus width
#define DRAMSIZE	BITMASK32(2, 0)	// Size of SDRAM Device

//#define REG_SDMR              (SDRAM_BA + 0x18)       // R/W  SDRAM Mode Register
#define SDMR_CONFIGURE		BITMASK32(13, 7)	// SDRAM Dependent Configuration
#define LATENCY			BITMASK32(6, 4)	// CAS Latency
#define BRSTTYPE		BIT32(3)	// Burst Type
#define BRSTLENGTH		BITMASK32(2, 0)	// Burst Length

//#define REG_SDEMR     (SDRAM_BA + 0x1C)       // R/W  SDRAM Extended Mode Register
#define SDEMR_CONFIGURE		BITMASK32(13, 2)	// SDRAM Dependent Configuration
#define DRVSTRENGTH		BIT32(1)	// Output Drive Strength
#define DLLEN			BIT32(0)	// DLL Enable

//#define REG_SDEMR2            (SDRAM_BA + 0x20)       // R/W  SDRAM Extended Mode Register 2
#define SDEMR2_CONFIGURE	BITMASK32(13, 0)	// SDRAM Dependent Configuration

//#define REG_SDEMR3            (SDRAM_BA+ 0x24)        // R/W  SDRAM Extended Mode Register 3
#define SDEMR3_CONFIGURE	BITMASK32(13, 0)	// SDRAM Dependent Configuration

//#define REG_SDTIME            (SDRAM_BA + 0x28)       // R/W  SDRAM Timing Control Register
#define TWTR			BITMASK32(30, 29)	// Internal Write to Read Command Delay
#define TRRD			BITMASK32(28, 27)	// Active Bank a to Active Bank b Command Delay
#define TRC			BITMASK32(26, 22)	// Active to Active Command Delay
#define TXSR			BITMASK32(21, 17)	// Exit SELF REFRESH to ACTIVE Command Delay
#define TRFC			BITMASK32(16, 12)	// AUTO REFRESH Period
#define TRAS			BITMASK32(11, 8)	// ACTIVE to PRECHARGE Command Delay
#define TRCD			BITMASK32(7, 5)	// Active to READ or WRITE Delay
#define TRP			BITMASK32(4, 2)	// PRECHARGE Command Period
#define TWR 			BITMASK32(1, 0)	// WRITE Recovery Time

//#define REG_CKDQSDS   (SDRAM_BA + 0x34)       // R/W  Clock and DQS Delay Selection Register
#define READ_WAIT_CYCLE		BITMASK32(25, 24)	// DQS1 Input Delay Selection 1
#define DATACLK_DELAYSEL	BITMASK32(20, 16)	// DQS1 Input Delay Selection 0
#define DS1_SKEW		BITMASK32(15, 12)	// DQS0 Input Delay Selection 1
#define DS0_SKEW		BITMASK32(11, 8)	// DQS0 Input Delay Selection 0
#define MCLK_ODS		BITMASK32(2, 0)	// MCLK Output Delay Selection

//#define REG_DLLMODE           (SDRAM_BA + 0x54)// R   SDRAM test Gold data
#define DLL_EN		BIT32(3)	// Enable DLL
#define DLL_PARAM	BITMASK32(2, 0)	// DLL DQS Delay Selection

//#define REG_DBGREG            (SDRAM_BA + 0x70)// R   SDRAM test Gold data
#define SRF_STATE	BIT32(28)	// SDIC Currently Stays in Self-Refresh state
#define FSM_CSTATE	BITMASK32(27, 0)	// FSM Current State

#endif
