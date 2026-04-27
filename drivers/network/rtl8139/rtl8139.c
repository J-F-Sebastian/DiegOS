/**
 *
 *                                                   ,----..
 *        ,---,                                     /   /   \   .--.--.
 *      .'  .' `\    ,--,                          /   .     : /  /    '.
 *    ,---.'     \ ,--.'|                         .   /   ;.  \  :  /`. /
 *    |   |  .`\  ||  |,                ,----._,..   ;   /  ` ;  |  |--`
 *    :   : |  '  |`--'_       ,---.   /   /  ' /;   |  ; \ ; |  :  ;_
 *    |   ' '  ;  :,' ,'|     /     \ |   :     ||   :  | ; | '\  \    `.
 *    '   | ;  .  |'  | |    /    /  ||   | .\  ..   |  ' ' ' : `----.   \
 *    |   | :  |  '|  | :   .    ' / |.   ; ';  |'   ;  \; /  | __ \  \  |
 *    '   : | /  ; '  : |__ '   ;   /|'   .   . | \   \  ',  / /  /`--'  /
 *    |   | '` ,/  |  | '.'|'   |  / | `---`-'| |  ;   :    / '--'.     /
 *    ;   :  .'    ;  :    ;|   :    | .'__/\_: |   \   \ .'    `--'---'
 *    |   ,.'      |  ,   /  \   \  /  |   :    :    `---`
 *    '---'         ---`-'    `----'    \   \  /
 *                                       `--`-'
 */

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

/*
 * rtl8139.c
 *
 * This file contains a ethernet device driver for Realtek rtl8139 based
 * ethernet cards.
 *
 * Created:	Aug 2003 by Philip Homburg <philip@cs.vu.nl>
 * Changes:
 *   Aug 15, 2004   sync alarms replace watchdogs timers  (Jorrit N. Herder)
 *   May 02, 2004   flag alarms replace micro_elapsed()  (Jorrit N. Herder)
 *
 */

#include <types_common.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libs/pci.h>
#include <libs/pci_lib.h>
#include <processor/ports.h>
#include <diegos/interrupts.h>
#include <diegos/delays.h>
#include <diegos/drivers.h>
#include <diegos/net_buffers.h>
#include <diegos/if.h>
#include <libs/iomalloc.h>
#include <assert.h>
#include "rtl8139_private.h"
#include "rtl8139.h"

// 96*16
#define FRAME_SIZE 1536

enum rx_packet_hdr_status {
	/* Multicast address received */
	RX_HDR_MAR = 1 << 15,
	/* Physical address matched */
	RX_HDR_PAM = 1 << 14,
	/* Broadcast address received */
	RX_HDR_BAR = 1 << 13,
	/* Invalid symbol error */
	RX_HDR_ISE = 1 << 5,
	/* Runt packet received (less than 64 bytes) */
	RX_HDR_RUNT = 1 << 4,
	/* Long packet received (more than 4k bytes) */
	RX_HDR_LONG = 1 << 3,
	/* CRC Error (FCS) */
	RX_HDR_CRC = 1 << 2,
	/* Frame alignment error */
	RX_HDR_FAE = 1 << 1,
	/* Receive OK */
	RX_HDR_ROK = 1 << 0
};

#pragma pack(push,1)

struct rx_packet_hdr {
	uint16_t status;
	uint16_t pkt_len;
	uint8_t data[FRAME_SIZE - 4];
};

#pragma pack(pop)

/*
 * We need some struct for this
 */
static char *rx_buffer = NULL;
static unsigned rx_cur_address = 0;
static unsigned rx_ring_offset = 0;

static pci_bus_device_t *instance = NULL;

static const uint16_t vid_did[] = {
	0x10ec, 0x8139,
};

static uint16_t rtl_port = 0;

static const char *models[] = {
	"RTL8139",
	"RTL8139A",
	"RTL8139A-G / RTL8139C",
	"RTL8139B / RTL8130",
	"RTL8100",
	"RTL8100B/RTL8139D",
	"RTL8139C+",
	"RTL8101",
	"Unknown"
};

static const char *ROMs[] = {
	"No Boot ROM",
	"8k Boot ROM",
	"16k Boot ROM",
	"32k Boot ROM",
	"64k Boot ROM",
	"128k Boot ROM",
	"Unknown"
};

static const char *model_name = NULL;
static const char *rom_size = NULL;

static unsigned status = DRV_IS_NET;

static void rtl_link_status(void);

static void rtl_init_config_info(void)
{
	uint8_t cfg0;
	uint32_t tcr;

	rtl_port = instance->BAR[0] & BAR_IO_SPACE_MASK;

	tcr = in_dword(rtl_port + RL_TCR);

	switch (tcr & (RL_TCR_HWVER_AM | RL_TCR_HWVER_BM)) {
	case RL_TCR_HWVER_RTL8139:
		model_name = models[0];
		break;

	case RL_TCR_HWVER_RTL8139A:
		model_name = models[1];
		break;

	case RL_TCR_HWVER_RTL8139AG:
		model_name = models[2];
		break;

	case RL_TCR_HWVER_RTL8139B:
		model_name = models[3];
		break;

	case RL_TCR_HWVER_RTL8100:
		model_name = models[4];
		break;

	case RL_TCR_HWVER_RTL8100B:
		model_name = models[5];
		break;

	case RL_TCR_HWVER_RTL8139CP:
		model_name = models[6];
		break;

	case RL_TCR_HWVER_RTL8101:
		model_name = models[7];
		break;

	default:
		model_name = models[8];
		break;
	}

	cfg0 = in_byte(rtl_port + RL_CONFIG0);
	if ((cfg0 & 0x7) < 6)
		rom_size = ROMs[cfg0 & 0x7];
	else
		rom_size = ROMs[6];

}

static void rtl_print_config_info(void)
{
	uint8_t rev_id = in_byte(rtl_port + RL_REVID);
	//uint8_t cfg0, cfg1, cfg3, cfg4;

	//cfg0 = in_byte(rtl_port + RL_CONFIG0);
	//cfg1 = in_byte(rtl_port + RL_CONFIG1);
	//cfg3 = in_byte(rtl_port + RL_CONFIG3);
	//cfg4 = in_byte(rtl_port + RL_CONFIG4);

	kdrvprintf("Detected Realtek %s with %s (PCI Rev ID: 0x%02x)\n", model_name, rom_size,
		   rev_id);

}

static void rtl_config_rx(void)
{
	uint32_t rcr;

	rcr = in_dword(rtl_port + RL_RCR);
	/*
	 * Set receive mode: keep reserved bits as is,
	 * no early threshold, no 8 bytes errored packets,
	 * no RX threshold, 32Kb RX buffer length, 1024 DMA bytes MAX,
	 * no wrapping, no errored packets, no runts.
	 */
	rcr &= (RL_RCR_RES0 | RL_RCR_RES1 | RL_RCR_RES2);
	rcr |= (RL_RCR_RXFTH_UNLIM | RL_RCR_RBLEN_32K | RL_RCR_MXDMA_1024);
	rcr |= RL_RCR_WRAP;
	/*
	 * Set receive mode flags.
	 */
	if (rtl8139_drv.ifflags & IFF_PROMISC)
		rcr |= RL_RCR_AB | RL_RCR_AM | RL_RCR_AAP;
	else {
		if (rtl8139_drv.ifflags & IFF_BROADCAST)
			rcr |= RL_RCR_AB;
		if (rtl8139_drv.ifflags & (IFF_MULTICAST | IFF_ALLMULTI))
			rcr |= RL_RCR_AM;
	}
	rcr |= RL_RCR_APM;
	out_dword(rtl_port + RL_RCR, rcr);

	out_dword(rtl_port + RL_RBSTART, (uintptr_t) (rx_buffer));
	rx_cur_address = 0;
	rx_ring_offset = 0;
}

static void rtl_config_tx(void)
{
	uint32_t tcr, tsd;
	unsigned i;

	tcr = in_dword(rtl_port + RL_TCR);

	/*
	 * Set transmit mode: keep reserved bits as is,
	 * standard interframe gap, 1024 DMA bytes MAX,
	 * append CRC (a.k.a. FCS).
	 */
	tcr &= RL_TCR_RES0 | RL_TCR_RES1 | RL_TCR_RES2 | RL_TCR_RES3;

	tcr |= RL_TCR_IFG_STD | RL_TCR_MXDMA_1024 | RL_TCR_CRC;

	out_dword(rtl_port + RL_TCR, tcr);

	for (i = RL_TSD0; i < RL_TSD0 + 16; i += 4) {
		tsd = in_dword(rtl_port + i);
		/*
		 * Keep reserved bits and OWN bit
		 */
		tsd &= RL_TSD_RES | RL_TSD_OWN;
		/*
		 * Set early TX threshold to 1024 bytes
		 */
		tsd = (1024 / 32) << RL_TSD_ERTXTH_S;
		out_dword(rtl_port + i, tsd);
	}
}

static void rtl_set_interrupt_mask(void)
{
	uint16_t mask;

	mask = in_word(rtl_port + RL_IMR);

	/*
	 * Set interrupt mask: keep reserved bits as is,
	 * enable all interrupts except cable length change.
	 */
	mask &= RL_IMR_RES;

	mask |= RL_IMR_ROK |
	    RL_IMR_RER |
	    RL_IMR_TOK |
	    RL_IMR_TER | RL_IMR_RXOVW | RL_IMR_PUN | RL_IMR_FOVW | RL_IMR_TIMEOUT | RL_IMR_SERR;

	out_word(rtl_port + RL_IMR, mask);
}

static void rtl_clear_interrupt_mask(void)
{
	uint16_t mask;

	mask = in_word(rtl_port + RL_IMR);

	/*
	 * Clear interrupt mask: keep reserved bits as is,
	 * disable all interrupts.
	 */
	mask &= RL_IMR_RES;

	out_word(rtl_port + RL_IMR, mask);
}

static void rtl_clear_rx(void)
{
	unsigned timeout = 10000;
	uint8_t cr;

	/* Reset the receiver */
	cr = in_byte(rtl_port + RL_CR);
	cr &= ~RL_CR_RE;
	out_byte(rtl_port + RL_CR, cr);
	while ((in_byte(rtl_port + RL_CR) & RL_CR_RE) && timeout--) {
		udelay(100);
	}

	if (in_byte(rtl_port + RL_CR) & RL_CR_RE)
		kdrvprintf("cannot disable receiver");

	out_byte(rtl_port + RL_CR, cr | RL_CR_RE);

	rtl_config_rx();
}

static void rtl_receive_packets(void)
{
	struct rx_packet_hdr *rx_pkt;
	unsigned pkt_len;
	unsigned status;
	struct packet *pkt;

	while (((in_byte(rtl_port + RL_CR) & RL_CR_BUFE) == 0)) {
		rx_pkt = (struct rx_packet_hdr *)(rx_buffer + rx_ring_offset);
		pkt_len = rx_pkt->pkt_len;
		status = rx_pkt->status;

		kdrvprintf("rtl8139: received packet of length %u, status=0x%04x\n", pkt_len,
			   status);

		/* Check for errors */
		if (status & RX_HDR_ROK) {
			/* Allocate a packet structure */
			if (netbuf_get(&pkt, pkt_len)) {
				kdrvprintf("rtl8139: cannot allocate packet buffer\n");
				rtl_clear_rx();
				return;
			}

			/* Copy data to the packet structure */
			netbuf_copy_eth(rx_pkt->data, pkt, pkt_len);
			/* Pass the packet to the network stack */
			if (netbuf_in(pkt)) {
				kdrvprintf("rtl8139: stop!\n");
				rtl_clear_rx();
				return;
			}
		}

		/* Update the current buffer read pointer */
		rx_cur_address += (pkt_len + 4 + 3) & (~3);
		rx_ring_offset += (pkt_len + 4 + 3) & (~3);
		if (rx_ring_offset >= RL_RCR_RBLEN_32K_SIZE) {
			rx_ring_offset -= RL_RCR_RBLEN_32K_SIZE;
		}
		kdrvprintf("rtl8139: %d %d\n", rx_cur_address, rx_ring_offset);

		out_word(rtl_port + RL_CAPR, rx_cur_address - RL_CAPR_DATA_OFF);
	}
}

static void rtl_interrupts(uint16_t isr)
{
	BOOL link_up, was_link_up;

	if (isr & RL_ISR_ROK) {
		kdrvprintf("rtl8139: received packet\n");
		rtl_receive_packets();
	}

	if (isr & RL_ISR_RER) {
		kdrvprintf("rtl8139: receive error\n");
	}

	if (isr & RL_ISR_RXOVW) {
		kdrvprintf("rtl8139: receive buffer overflow\n");
		rtl_clear_rx();
	}

	if (isr & RL_ISR_PUN) {
		isr &= ~RL_ISR_PUN;

		/* 
		 * Either the link status changed or there was a TX fifo
		 * underrun.
		 */
		link_up = (in_byte(rtl_port + RL_MSR) & RL_MSR_LINKB) ? FALSE : TRUE;
		was_link_up = (rtl8139_drv.ifflags & IFF_UP) ? TRUE : FALSE;
		if (link_up != was_link_up) {
			if (!link_up) {
				kdrvprintf("rtl8139: link down\n");
				rtl8139_drv.ifflags &= ~IFF_UP;
			} else {
				kdrvprintf("rtl8139: link up\n");
				rtl8139_drv.ifflags |= IFF_UP;
			}
			rtl_link_status();
		}
	}

	if (isr & RL_ISR_FOVW) {
		kdrvprintf("rtl8139: FIFO overflow\n");
	}

	if (isr & RL_ISR_TIMEOUT) {
		kdrvprintf("rtl8139: transmit timeout\n");
	}

	if (isr & RL_ISR_SERR) {
		kdrvprintf("rtl8139: system error\n");
	}

	if (isr & RL_ISR_TOK) {
		kdrvprintf("rtl8139: transmit OK\n");
	}

	if (isr & RL_ISR_TER) {
		kdrvprintf("rtl8139: transmit error\n");
	}
}

/*
 * Interrupt handler
 */
static BOOL rtl_int_handler(void)
{
	/* Ack interrupt */
	uint16_t isr = in_word(rtl_port + RL_ISR);
	out_word(rtl_port + RL_ISR, isr);

	kdrvprintf("rtl8139: interrupt, ISR=0x%04x\n", isr);

	rtl_interrupts(isr);

	return (TRUE);
}

static void mii_print_techab(uint16_t techab)
{
	int fs, ft;
	char message[128] = { 0 };

	if ((techab & MII_ANA_SEL_M) != MII_ANA_SEL_802_3) {
		kdrvprintf("strange selector 0x%x, value 0x%x\n",
			   techab & MII_ANA_SEL_M, (techab & MII_ANA_TAF_M) >> MII_ANA_TAF_S);
		return;
	}

	fs = 1;
	if (techab & (MII_ANA_100T4 | MII_ANA_100TXFD | MII_ANA_100TXHD)) {
		strcat(message, "100 Mbps: ");
		fs = 0;
		ft = 1;
		if (techab & MII_ANA_100T4) {
			strcat(message, "T4");
			ft = 0;
		}
		if (techab & (MII_ANA_100TXFD | MII_ANA_100TXHD)) {
			if (!ft)
				strcat(message, ", ");
			ft = 0;
			strcat(message, "TX-");
			switch (techab & (MII_ANA_100TXFD | MII_ANA_100TXHD)) {
			case MII_ANA_100TXFD:
				strcat(message, "FD");
				break;
			case MII_ANA_100TXHD:
				strcat(message, "HD");
				break;
			default:
				strcat(message, "FD/HD");
				break;
			}
		}
	}
	if (techab & (MII_ANA_10TFD | MII_ANA_10THD)) {
		if (!fs)
			strcat(message, ", ");
		strcat(message, "10 Mbps: ");
		fs = 0;
		strcat(message, "T-");
		switch (techab & (MII_ANA_10TFD | MII_ANA_10THD)) {
		case MII_ANA_10TFD:
			strcat(message, "FD");
			break;
		case MII_ANA_10THD:
			strcat(message, "HD");
			break;
		default:
			strcat(message, "FD/HD");
			break;
		}
	}
	if (techab & MII_ANA_PAUSE_SYM) {
		if (!fs)
			strcat(message, ", ");
		fs = 0;
		strcat(message, "pause(SYM)");
	}
	if (techab & MII_ANA_PAUSE_ASYM) {
		if (!fs)
			strcat(message, ", ");
		fs = 0;
		strcat(message, "pause(ASYM)");
	}
	if (techab & MII_ANA_TAF_RES) {
		if (!fs)
			strcat(message, ", ");
		fs = 0;
		char temp[32];
		snprintf(temp, sizeof(temp), "0x%x", (techab & MII_ANA_TAF_RES) >> MII_ANA_TAF_S);
		strcat(message, temp);
	}
	kdrvprintf("%s\n", message);
}

static void mii_print_stat_speed(uint16_t stat, uint16_t extstat)
{
	int fs, ft;
	char message[128] = { 0 };

	fs = 1;
	if (stat & MII_STATUS_EXT_STAT) {
		if (extstat & (MII_ESTAT_1000XFD | MII_ESTAT_1000XHD |
			       MII_ESTAT_1000TFD | MII_ESTAT_1000THD)) {
			strcat(message, "1000 Mbps: ");
			fs = 0;
			ft = 1;
			if (extstat & (MII_ESTAT_1000XFD | MII_ESTAT_1000XHD)) {
				ft = 0;
				strcat(message, "X-");
				switch (extstat & (MII_ESTAT_1000XFD | MII_ESTAT_1000XHD)) {
				case MII_ESTAT_1000XFD:
					strcat(message, "FD");
					break;
				case MII_ESTAT_1000XHD:
					strcat(message, "HD");
					break;
				default:
					strcat(message, "FD/HD");
					break;
				}
			}
			if (extstat & (MII_ESTAT_1000TFD | MII_ESTAT_1000THD)) {
				if (!ft)
					strcat(message, ", ");
				ft = 0;
				strcat(message, "T-");
				switch (extstat & (MII_ESTAT_1000TFD | MII_ESTAT_1000THD)) {
				case MII_ESTAT_1000TFD:
					strcat(message, "FD");
					break;
				case MII_ESTAT_1000THD:
					strcat(message, "HD");
					break;
				default:
					strcat(message, "FD/HD");
					break;
				}
			}
		}
	}
	if (stat & (MII_STATUS_100T4 |
		    MII_STATUS_100XFD | MII_STATUS_100XHD |
		    MII_STATUS_100T2FD | MII_STATUS_100T2HD)) {
		if (!fs)
			strcat(message, ", ");
		fs = 0;
		strcat(message, "100 Mbps: ");
		ft = 1;
		if (stat & MII_STATUS_100T4) {
			strcat(message, "T4");
			ft = 0;
		}
		if (stat & (MII_STATUS_100XFD | MII_STATUS_100XHD)) {
			if (!ft)
				strcat(message, ", ");
			ft = 0;
			strcat(message, "TX-");
			switch (stat & (MII_STATUS_100XFD | MII_STATUS_100XHD)) {
			case MII_STATUS_100XFD:
				strcat(message, "FD");
				break;
			case MII_STATUS_100XHD:
				strcat(message, "HD");
				break;
			default:
				strcat(message, "FD/HD");
				break;
			}
		}
		if (stat & (MII_STATUS_100T2FD | MII_STATUS_100T2HD)) {
			if (!ft)
				strcat(message, ", ");
			ft = 0;
			strcat(message, "T2-");
			switch (stat & (MII_STATUS_100T2FD | MII_STATUS_100T2HD)) {
			case MII_STATUS_100T2FD:
				strcat(message, "FD");
				break;
			case MII_STATUS_100T2HD:
				strcat(message, "HD");
				break;
			default:
				strcat(message, "FD/HD");
				break;
			}
		}
	}
	if (stat & (MII_STATUS_10FD | MII_STATUS_10HD)) {
		if (!fs)
			strcat(message, ", ");
		strcat(message, "10 Mbps: ");
		fs = 0;
		strcat(message, "T-");
		switch (stat & (MII_STATUS_10FD | MII_STATUS_10HD)) {
		case MII_STATUS_10FD:
			strcat(message, "FD");
			break;
		case MII_STATUS_10HD:
			strcat(message, "HD");
			break;
		default:
			strcat(message, "FD/HD");
			break;
		}
	}
	kdrvprintf("%s\n", message);
}

/*
 * Detect link status changes and configure the card accordingly.
 */
static void rtl_link_status(void)
{
	uint16_t mii_ctrl, mii_status, mii_ana, mii_anlpa, mii_ane, mii_extstat;
	uint8_t msr;
	int link_up;
	char message[64] = { 0 };

	msr = in_byte(rtl_port + RL_MSR);
	link_up = !(msr & RL_MSR_LINKB);

	if (!link_up) {
		kdrvprintf("%s: link down\n", rtl8139_drv.cmn.name);
		return;
	}

	mii_ctrl = in_word(rtl_port + RL_BMCR);
	mii_status = in_word(rtl_port + RL_BMSR);
	mii_ana = in_word(rtl_port + RL_ANAR);
	mii_anlpa = in_word(rtl_port + RL_ANLPAR);
	mii_ane = in_word(rtl_port + RL_ANER);
	mii_extstat = 0;

	strcat(message, rtl8139_drv.ifname);

	if (mii_ctrl & (MII_CTRL_LB | MII_CTRL_PD | MII_CTRL_ISO)) {
		strcat(message, "PHY:");
		if (mii_ctrl & MII_CTRL_LB) {
			strcat(message, " loopback mode");
		}
		if (mii_ctrl & MII_CTRL_PD) {
			strcat(message, " powered down");
		}
		if (mii_ctrl & MII_CTRL_ISO) {
			strcat(message, " isolated");
		}
		kdrvprintf("%s %s\n", rtl8139_drv.cmn.name, message);
		return;
	}

	if (!(mii_ctrl & MII_CTRL_ANE)) {
		strcat(message, " manual config: ");
		switch (mii_ctrl & (MII_CTRL_SP_LSB | MII_CTRL_SP_MSB)) {
		case MII_CTRL_SP_10:
			strcat(message, "10 Mbps");
			break;
		case MII_CTRL_SP_100:
			strcat(message, "100 Mbps");
			break;
		case MII_CTRL_SP_1000:
			strcat(message, "1000 Mbps");
			break;
		case MII_CTRL_SP_RES:
			strcat(message, "reserved speed");
			break;
		}
		if (mii_ctrl & MII_CTRL_DM)
			strcat(message, ", full duplex");
		else
			strcat(message, ", half duplex");
		kdrvprintf("%s\n", message);
		return;
	}

	mii_print_stat_speed(mii_status, mii_extstat);

	if (!(mii_status & MII_STATUS_ANC))
		kdrvprintf("%s: auto-negotiation not complete\n", rtl8139_drv.ifname);
	if (mii_status & MII_STATUS_RF)
		kdrvprintf("%s: remote fault detected\n", rtl8139_drv.ifname);
	if (!(mii_status & MII_STATUS_ANA)) {
		kdrvprintf("%s: local PHY has no auto-negotiation ability\n", rtl8139_drv.ifname);
	}
	if (!(mii_status & MII_STATUS_LS))
		kdrvprintf("%s: link down\n", rtl8139_drv.ifname);
	if (mii_status & MII_STATUS_JD)
		kdrvprintf("%s: jabber condition detected\n", rtl8139_drv.ifname);
	if (!(mii_status & MII_STATUS_EC)) {
		kdrvprintf("%s: no extended register set\n", rtl8139_drv.ifname);
		goto resspeed;
	}
	if (!(mii_status & MII_STATUS_ANC))
		goto resspeed;

	kdrvprintf("%s: local cap.: ", rtl8139_drv.ifname);
	mii_print_techab(mii_ana);

	if (mii_ane & MII_ANE_PDF)
		kdrvprintf("%s: parallel detection fault\n", rtl8139_drv.ifname);
	if (!(mii_ane & MII_ANE_LPANA)) {
		kdrvprintf("%s: link-partner does not support auto-negotiation\n",
			   rtl8139_drv.ifname);
		goto resspeed;
	}

	kdrvprintf("%s: remote cap.: ", rtl8139_drv.ifname);
	mii_print_techab(mii_anlpa);
 resspeed:

	kdrvprintf("link up at %d Mbps, ", (msr & RL_MSR_SPEED_10) ? 10 : 100);
	kdrvprintf("%s duplex\n", ((mii_ctrl & MII_CTRL_DM) ? "full" : "half"));
}

static void rtl_reset_hw(void)
{
	unsigned timeout = 10000;
	uint8_t cfg1;

	/* Reset the device */
	out_byte(rtl_port + RL_CR, RL_CR_RST);
	while ((in_byte(rtl_port + RL_CR) & RL_CR_RST) && timeout--) {
		udelay(100);
	}

	cfg1 = in_byte(rtl_port + RL_CONFIG1);
	cfg1 &= ~0x01;
	out_byte(rtl_port + RL_CONFIG1, cfg1);
}

static int rtl_init(unsigned unitno)
{
	unsigned i = 0;
	bdf_addr_t addr;
	uint16_t cr;

	if (unitno) {
		return (ENXIO);
	}

	for (i = 0; i < NELEMENTS(vid_did); i += 2) {
		instance = pci_bus_find_device(vid_did[i], vid_did[i + 1], NULL);

		if (instance) {

			/*
			 * RX Buffer size + one packet (no wrapping) + 16
			 */
			rx_buffer = iomalloc(RL_RCR_RBLEN_32K_SIZE + 1536 + 16, 64 * 1024);

			if (!rx_buffer) {
				kdrvprintf("rtl8139: cannot allocate RX buffer\n");
				return (ENOMEM);
			}

			kdrvprintf("%u:%u.%u %#x:%#x ==> %s\n",
				   instance->bus,
				   instance->device,
				   instance->function,
				   instance->vendorid, instance->deviceid, "Realtek RTL8139");

			addr = pci_create_bdf(instance->bus, instance->device, instance->function);
			/* Enable bus mastering if necessary. */
			pci_read_config_reg16(addr, 4, &cr);

			if (!(cr & COMMAND_BUS_MASTER))
				pci_write_config_reg16(addr, 4, cr | COMMAND_BUS_MASTER);

			kdrvprintf("using I/O address %p, IRQ %d,%d\n", rtl_port, instance->int_pin,
				   instance->int_line);

			rtl_init_config_info();
			rtl_print_config_info();

			/*
			 * Interrupts need to be relocated following the x86 DiegOS mapping.
			 * This could be done directly in the PCI library code, but for the sake
			 * of future portability we do it here.
			 */
			if (EOK != add_int_cb(rtl_int_handler, 0x20 + instance->int_line)) {
				return (EPERM);
			}

			/*
			 * Configure a convenience MAC address, need to provision this
			 */
			out_byte(rtl_port + RL_9346CR, RL_9346CR_EEM_CONFIG);
			out_dword(rtl_port + RL_IDR, 0x04050607);
			out_dword(rtl_port + RL_IDR + 4, 0x00000203);
			out_byte(rtl_port + RL_9346CR, RL_9346CR_EEM_NORMAL);

			return (EOK);
		}
	}

	return (ENXIO);
}

static int rtl_start(unsigned unitno)
{
	uint8_t cr;

	if (unitno) {
		return (ENXIO);
	}

	rtl_reset_hw();
	rtl_set_interrupt_mask();
	rtl_config_tx();
	rtl_config_rx();

	/*
	 * Fire up the communications!
	 */
	cr = in_byte(rtl_port + RL_CR);
	cr |= RL_CR_RE | RL_CR_TE;
	out_byte(rtl_port + RL_CR, cr);

	enable_int(0x20 + instance->int_line);

	status &= DRV_IS_MASK;
	status |= DRV_STATUS_RUN;

	return (EOK);
}

static int rtl_stop(unsigned unitno)
{
	uint8_t cr;

	if (unitno) {
		return (ENXIO);
	}

	/*
	 * Mask off interrupts
	 */
	rtl_clear_interrupt_mask();

	/*
	 * Stop the communications!
	 */
	cr = in_byte(rtl_port + RL_CR);
	cr &= ~(RL_CR_RE | RL_CR_TE);
	out_byte(rtl_port + RL_CR, cr);

	disable_int(0x20 + instance->int_line);

	status &= DRV_IS_MASK;
	status |= DRV_STATUS_STOP;

	return (EOK);
}

static int rtl_done(unsigned unitno)
{
	if (unitno) {
		return (ENXIO);
	}

	rtl_reset_hw();

	status &= DRV_IS_MASK;
	status |= DRV_STATUS_DONE;

	return (EOK);
}

static unsigned rtl_status(unsigned unitno)
{
	return (status);
}

net_driver_t rtl8139_drv = {
	.cmn = {
		.name = "8139",
		.init_fn = rtl_init,
		.start_fn = rtl_start,
		.stop_fn = rtl_stop,
		.done_fn = rtl_done,
		.status_fn = rtl_status,
		.poll_fn = NULL}
	,
	.ifname = "eth",
	.mtu = 1500,
	.iftype = 0,
	.ifflags = IFF_BROADCAST,
	.tx_fn = NULL,
	.rx_fn = NULL,
	.tx_multi_fn = NULL,
	.rx_multi_fn = NULL,
	.rx_peak_fn = NULL
};

/*
 * $PchId: ne2000.c,v 1.4 1996/01/19 23:30:34 philip Exp $
 */
