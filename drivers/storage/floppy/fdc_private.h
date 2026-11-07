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

#ifndef _FDC_PRIVATE_H_
#define _FDC_PRIVATE_H_

/*
 * Floppy Disk Controller registers.
 * Accessed by ports.
 * List taken from "The Undocumented PC" page 439
 */
/* Diskette Controller Status Register A */
/* Type Input Platform PS/2 */
#define FDC_STATUS_A    0x3f0
/* Diskette Controller Diagnostic Register */
/* Type Input Platform Limited */
#define FDC_DIAG        0x3f1
/* Diskette Controller Status Register B */
/* Type Input Platform PS/2 */
#define FDC_STATUS_B    0x3f1
/* Diskette Controller Digital Output Register */
/* Type I/O Platform All */
#define FDC_DOR		0x3f2
/* Diskette Controller Tape Drive Register */
/* Type I/O Platform Limited */
#define FDC_TAPE        0x3f3
/* Diskette Controller Drive Status Register */
/* Type Input Platform PS/2 */
#define FDC_DRVSTATUS   0x3f3
/* Diskette Controller Main Status Register */
/* Type Input Platform All */
#define FDC_STATUS	0x3f4
/* Diskette Controller Data */
/* Type I/O Platform All */
#define FDC_DATA	0x3f5
/* Diskette Controller Reserved */
/* Type I/O Platform All */
#define FDC_RESERVED	0x3f6
/* Diskette Controller Digital Input Register */
/* Type Input Platform AT+ */
#define FDC_DIR         0x3f7
/* Diskette Controller Configuration Control Register */
/* Type Output Platform AT+ */
#define FDC_CFGCTRL	0x3f7

/* Bits of digital output register */
#define DOR_DRIVE0	0x00	/* drive 0 select */
#define DOR_DRIVE1	0x01	/* drive 1 select */
#define DOR_DRIVE2	0x02	/* drive 2 select */
#define DOR_DRIVE3	0x03	/* drive 3 select */
#define DOR_DRIVE_MASK	0x03	/* drive select mask */
#define DOR_RESET	0x04	/* clear reset controller / set normal operation */
#define DOR_DMA		0x08	/* enable DMA and interrupts */
#define DOR_MOTOR0	0x10	/* drive 0 motor on */
#define DOR_MOTOR1	0x20	/* drive 1 motor on */
#define DOR_MOTOR2	0x40	/* drive 2 motor on */
#define DOR_MOTOR3	0x80	/* drive 3 motor on */
#define DOR_MOTOR_MASK	0xF0	/* mask for motor bits */

/* Bits of main status register */
#define STATUS_DISKB(x)	(0x01 << x)	/* Disk X busy in recalibrate or seek */
#define STATUS_BUSYMASK	0x0F	/* drive busy mask */
#define STATUS_BUSY	0x10	/* FDC busy in reading or writing */
#define STATUS_DMA	0x20	/* 0- DMA mode */
#define STATUS_DIR	0x40	/* 0- cpu->fdc */
#define STATUS_READY	0x80	/* Data reg ready */

/* Bits of FD_ST0 */
#define ST0_DS		0x03	/* drive select mask 0,1,2,3 */
#define ST0_HEAD	0x04	/* Head 0 side 0 1 side 1 */
#define ST0_NR		0x08	/* Not Ready */
#define ST0_ECE		0x10	/* Equipment check error - seek or recalibrate failed */
#define ST0_SE		0x20	/* Seek end - seek or recalibrate completed */
#define ST0_ABNORM      0x40	/* Abnormal termination */
#define ST0_INVCMD	0x80	/* Invalid command  */
#define ST0_INTR	0xC0	/* Interrupt code mask */

/* Bits of FD_ST1 */
#define ST1_MAM		0x01	/* Missing Address Mark (2 index pulses without address mark of read below) */
#define ST1_WP		0x02	/* Write Protect error */
#define ST1_ND		0x04	/* No Data error - unreadable */
#define ST1_OR		0x10	/* Overrun or underrun */
#define ST1_CRC		0x20	/* CRC error in data or addr */
#define ST1_EOC		0x80	/* Sector past end of track error */

/* Bits of FD_ST2 */
#define ST2_MAM		0x01	/* Missing Addess Mark (different than 0xF8 or 0xFE) */
#define ST2_BC		0x02	/* Bad Cylinder (sector ID is 0xFF) */
#define ST2_SNS		0x04	/* Scan Not Satisfied */
#define ST2_SEH		0x08	/* Scan Equal Hit */
#define ST2_WC		0x10	/* Wrong Cylinder */
#define ST2_CRC		0x20	/* CRC error in data field */
#define ST2_CM		0x40	/* Control Mark error (data instead of deleted data and reverse) */

/* Values for FD_COMMAND */
#define FD_SPECIFY              0x03	/* specify Step Rate Timer, Head Load/Unload Timer */
#define FD_SENSED		0x04	/* Sense Drive Status */
#define FD_RECALIBRATE		0x07	/* move to track 0 */
#define FD_SENSEI		0x08	/* Sense Interrupt Status */
#define FD_SEEK			0x0F	/* seek track */
#define FD_VERSION		0x10	/* get version code */
#define FD_PERPENDICULAR	0x12	/* perpendicular r/w mode */
#define FD_CONFIGURE		0x13	/* configure FIFO operation */
#define FD_READID		0x4A	/* read ID */
#define FD_FORMAT		0x4D	/* format one track */
#define FD_WRITE		0xC5	/* write normal data with MT, MFM */
#define FD_READND		0xC6	/* read normal and deleted data with MT, MFM */
#define FD_WRITED		0xC9	/* write deleted data with MT, MFM */
#define FD_READD		0xCC	/* read deleted data with MT, MFM */
#define FD_READ			0xE6	/* read with MT, MFM, SKip deleted */

/* Bits of DIR */
#define DIR_CHG                 0x80	/* Disk Change - 1 if diskette has been changed since last reset */

/* Interrupt vector */
#define FLOPPY_IVT    (32 + FLOPPY_IRQ)

#endif
