/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2026 Diego Gallizioli
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

#ifndef _I8237_H_
#define _I8237_H_

#include <processor/ports.h>

/*
 * 8237 IC is a DMA controller
 */

#define MAX_DMA_CHANNELS	8

/* 8237 DMA controllers */
#define IO_DMA1_BASE	0x00	/* 8 bit slave DMA, channels 0..3 */
#define IO_DMA2_BASE	0xC0	/* 16 bit master DMA, ch 4(=slave input)..7 */

/* DMA controller registers */
#define DMA1_CMD_REG		0x08	/* command register (w) */
#define DMA1_STAT_REG		0x08	/* status register (r) */
#define DMA1_REQ_REG            0x09	/* request register (w) */
#define DMA1_MASK_REG		0x0A	/* single-channel mask (w) */
#define DMA1_MODE_REG		0x0B	/* mode register (w) */
#define DMA1_CLEAR_FF_REG	0x0C	/* clear pointer flip-flop (w) */
#define DMA1_TEMP_REG           0x0D	/* Temporary Register (r) */
#define DMA1_RESET_REG		0x0D	/* Master Clear (w) */
#define DMA1_CLR_MASK_REG       0x0E	/* Clear Mask */
#define DMA1_MASK_ALL_REG       0x0F	/* all-channels mask (w) */

#define DMA2_CMD_REG		0xD0	/* command register (w) */
#define DMA2_STAT_REG		0xD0	/* status register (r) */
#define DMA2_REQ_REG            0xD2	/* request register (w) */
#define DMA2_MASK_REG		0xD4	/* single-channel mask (w) */
#define DMA2_MODE_REG		0xD6	/* mode register (w) */
#define DMA2_CLEAR_FF_REG	0xD8	/* clear pointer flip-flop (w) */
#define DMA2_TEMP_REG           0xDA	/* Temporary Register (r) */
#define DMA2_RESET_REG		0xDA	/* Master Clear (w) */
#define DMA2_CLR_MASK_REG       0xDC	/* Clear Mask */
#define DMA2_MASK_ALL_REG       0xDE	/* all-channels mask (w) */

#define DMA_ADDR_0              0x00	/* DMA address registers */
#define DMA_ADDR_1              0x02
#define DMA_ADDR_2              0x04
#define DMA_ADDR_3              0x06
#define DMA_ADDR_4              0xC0
#define DMA_ADDR_5              0xC4
#define DMA_ADDR_6              0xC8
#define DMA_ADDR_7              0xCC

#define DMA_CNT_0               0x01	/* DMA count registers */
#define DMA_CNT_1               0x03
#define DMA_CNT_2               0x05
#define DMA_CNT_3               0x07
#define DMA_CNT_4               0xC2
#define DMA_CNT_5               0xC6
#define DMA_CNT_6               0xCA
#define DMA_CNT_7               0xCE

#define DMA_PAGE_0              0x87	/* DMA page registers */
#define DMA_PAGE_1              0x83
#define DMA_PAGE_2              0x81
#define DMA_PAGE_3              0x82
#define DMA_PAGE_5              0x8B
#define DMA_PAGE_6              0x89
#define DMA_PAGE_7              0x8A

#define DMA_MODE_READ	0x44	/* I/O to memory, no autoinit, increment, single mode */
#define DMA_MODE_WRITE	0x48	/* memory to I/O, no autoinit, increment, single mode */
#define DMA_MODE_CASCADE 0xC0	/* pass thru DREQ->HRQ, DACK<-HLDA only */

/*
 * Enable a specific DMA channel
 */
inline void enable_dma(unsigned int dmanr)
{
	if (dmanr <= 3)
		out_byte(dmanr, DMA1_MASK_REG);
	else
		out_byte(dmanr & 3, DMA2_MASK_REG);
}

/*
 * Disable a specific DMA channel
 */
inline void disable_dma(unsigned int dmanr)
{
	if (dmanr <= 3)
		out_byte(dmanr | 4, DMA1_MASK_REG);
	else
		out_byte((dmanr & 3) | 4, DMA2_MASK_REG);
}

/*
 * Clear the DMA flip/flop
 */
inline void clear_dma_ff(unsigned int dmanr)
{
	if (dmanr <= 3)
		out_byte(0, DMA1_CLEAR_FF_REG);
	else
		out_byte(0, DMA2_CLEAR_FF_REG);
}

/*
 * Set mode (above) for a specific DMA channel
 */
inline void set_dma_mode(unsigned int dmanr, char mode)
{
	if (dmanr <= 3)
		out_byte(mode | dmanr, DMA1_MODE_REG);
	else
		out_byte(mode | (dmanr & 3), DMA2_MODE_REG);
}

/*
 * Set the page register bits of the transfer address.
 */
inline void set_dma_page(unsigned int dmanr, char pagenr)
{
	switch (dmanr & 7) {
	case 0:
		out_byte(pagenr, DMA_PAGE_0);
		break;
	case 1:
		out_byte(pagenr, DMA_PAGE_1);
		break;
	case 2:
		out_byte(pagenr, DMA_PAGE_2);
		break;
	case 3:
		out_byte(pagenr, DMA_PAGE_3);
		break;
	case 5:
		out_byte(pagenr & 0xfe, DMA_PAGE_5);
		break;
	case 6:
		out_byte(pagenr & 0xfe, DMA_PAGE_6);
		break;
	case 7:
		out_byte(pagenr & 0xfe, DMA_PAGE_7);
		break;
	}
}

/*
 * Set transfer address & page bits for specific DMA channel.
 * Assumes DMA flip/flop is clear (cal clear_dma_ff() to clear flip/flop)
 */
inline void set_dma_addr(unsigned int dmanr, unsigned int a)
{
	set_dma_page(dmanr, a >> 16);
	if (dmanr <= 3) {
		out_byte(a & 0xff, ((dmanr & 3) << 1) + IO_DMA1_BASE);
		out_byte((a >> 8) & 0xff, ((dmanr & 3) << 1) + IO_DMA1_BASE);
	} else {
		out_byte((a >> 1) & 0xff, ((dmanr & 3) << 2) + IO_DMA2_BASE);
		out_byte((a >> 9) & 0xff, ((dmanr & 3) << 2) + IO_DMA2_BASE);
	}
}

/*
 * Set transfer size (max 64k for DMA1..3, 128k for DMA5..7) for
 * a specific DMA channel.
 * You must ensure the parameters are valid.
 * "The Undocumented PC" says "the number of transfers is one more
 * than the initial word count"! This is taken into account.
 * Assumes DMA flip/flop is clear (cal clear_dma_ff() to clear flip/flop)
 * NOTE 2: "count" represents _bytes_ and must be even for channels 5-7.
 */
inline void set_dma_count(unsigned int dmanr, unsigned int count)
{
	count--;
	if (dmanr <= 3) {
		out_byte(count & 0xff, ((dmanr & 3) << 1) + 1 + IO_DMA1_BASE);
		out_byte((count >> 8) & 0xff, ((dmanr & 3) << 1) + 1 + IO_DMA1_BASE);
	} else {
		out_byte((count >> 1) & 0xff, ((dmanr & 3) << 2) + 2 + IO_DMA2_BASE);
		out_byte((count >> 9) & 0xff, ((dmanr & 3) << 2) + 2 + IO_DMA2_BASE);
	}
}

/*
 * Get DMA residue count. After a DMA transfer, this
 * should return zero. Reading this while a DMA transfer is
 * still in progress will return unpredictable results.
 * If called before the channel has been used, it may return 1.
 * Otherwise, it returns the number of _bytes_ left to transfer.
 *
 * Assumes DMA flip/flop is clear.
 */
inline unsigned short get_dma_residue(unsigned int dmanr)
{
	unsigned int io_port = (dmanr <= 3) ? ((dmanr & 3) << 1) + 1 + IO_DMA1_BASE
	    : ((dmanr & 3) << 2) + 2 + IO_DMA2_BASE;

	/* using short to get 16-bit wrap around */
	unsigned short count;

	count = 1 + in_byte(io_port);
	count += in_byte(io_port) << 8;

	return (dmanr <= 3) ? count : (count << 1);
}

#endif
