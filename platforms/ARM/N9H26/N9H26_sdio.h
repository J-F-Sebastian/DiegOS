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

#ifndef _N9H26_SDIO_H_
#define _N9H26_SDIO_H_

#include <types_common.h>

/*
 * All registers are 32 bit wide
 */

#define N9H26SDIO_BASE 0xB0005000UL

/*
 * This is the interface to SDIO shared buffer
 */

enum N9H26SDIODMAC {
        SDIOFMI_FB_0,
        SDIOFMI_FB_1,
        SDIOFMI_FB_2,
        SDIOFMI_FB_3,
        SDIOFMI_FB_4,
        SDIOFMI_FB_5,
        SDIOFMI_FB_6,
        SDIOFMI_FB_7,
        SDIOFMI_FB_8,
        SDIOFMI_FB_9,
        SDIOFMI_FB_10,
        SDIOFMI_FB_11,
        SDIOFMI_FB_12,
        SDIOFMI_FB_13,
        SDIOFMI_FB_14,
        SDIOFMI_FB_15,
        SDIOFMI_FB_16,
        SDIOFMI_FB_17,
        SDIOFMI_FB_18,
        SDIOFMI_FB_19,
        SDIOFMI_FB_20,
        SDIOFMI_FB_21,
        SDIOFMI_FB_22,
        SDIOFMI_FB_23,
        SDIOFMI_FB_24,
        SDIOFMI_FB_25,
        SDIOFMI_FB_26,
        SDIOFMI_FB_27,
        SDIOFMI_FB_28,
        SDIOFMI_FB_29,
        SDIOFMI_FB_30,
        SDIOFMI_FB_31,
	SDIODMACCSR = 256,
	SDIODMACSAR,
	SDIODMACBCR,
	SDIODMACIER,
	SDIODMACISR,
	SDIOFMICR = 512,
	SDIOFMIIER,
	SDIOFMIISR,
	SDIOCR = 520,
	SDIOARG,
	SDIOIER,
	SDIOISR,
	SDIORSP0,
	SDIORSP1,
	SDIOBLEN,
	SDIOTMOUT
};

/*
 * SDIO Control Registers
 */

//#define     REG_SDIOFB_0		(SDIODMAC_BA+0x000)  /* Shared Buffer (FIFO) */

//#define	REG_SDIODMACCSR			(SDIODMAC_BA+0x400)  /* DMAC Control and Status Register */
#define		FMI_BUSY		BIT32(9)		// FMI DMA transfer is in progress
#define 	SG_EN			BIT32(3)		// DMAC Scatter-gather function enable
#define 	DMAC_SWRST		BIT32(1)		// DMAC software reset enable
#define 	DMAC_EN			BIT32(0)		// DMAC enable

//#define     REG_SDIODMACSAR	    (SDIODMAC_BA+0x408)  /* DMAC Transfer Starting Address Register */
#define 	PAD_ORDER		BIT32(0)            // Physical Address Descriptor table fetching in order or out of order
//#define     REG_SDIODMACBCR		(SDIODMAC_BA+0x40C)  /* DMAC Transfer Byte Count Register */
//#define     REG_SDIODMACIER		(SDIODMAC_BA+0x410)  /* DMAC Interrupt Enable Register */
#define		WEOT_IE			BIT32(1)		// Wrong EOT encounterred interrupt enable
#define 	TABORT_IE		BIT32(0)		// DMA R/W target abort interrupt enable

//#define     REG_SDIODMACISR		(SDIODMAC_BA+0x414)  /* DMAC Interrupt Status Register */
#define		WEOT_IF			BIT32(1)		// Wrong EOT encounterred interrupt flag
#define 	TABORT_IF		BIT32(0)		// DMA R/W target abort interrupt flag

#endif
