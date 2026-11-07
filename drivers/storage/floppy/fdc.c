/*
 *  linux/kernel/floppy.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * 02.12.91 - Changed to static variables to indicate need for reset
 * and recalibrate. This makes some things easier (output_byte reset
 * checking etc), and means less interrupt jumping in case of errors,
 * so the code is hopefully easier to understand.
 */

/*
 * This file is certainly a mess. I've tried my best to get it working,
 * but I don't like programming floppies, and I have only one anyway.
 * Urgel. I should check for more errors, and do more graceful error
 * recovery. Seems there are problems with several drives. I've tried to
 * correct them. No promises.
 */

/*
 * As with hd.c, all routines within this file can (and will) be called
 * by interrupts, so extreme caution is needed. A hardware interrupt
 * handler may not sleep, or a kernel panic will happen. Thus I cannot
 * call "floppy-on" directly, but have to set a special timer interrupt
 * etc.
 */

/*
 * 28.02.92 - made track-buffering routines, based on the routines written
 * by entropy@wintermute.wpi.edu (Lawrence Foard). Linus.
 */

/*
 * Automatic floppy-detection and formatting written by Werner Almesberger
 * (almesber@nessie.cs.id.ethz.ch), who also corrected some problems with
 * the floppy-change signal detection.
 */

/*
 * 1992/7/22 -- Hennus Bergman: Added better error reporting, fixed
 * FDC data overrun bug, added some preliminary stuff for vertical
 * recording support.
 *
 * 1992/9/17: Added DMA allocation & DMA functions. -- hhb.
 *
 * TODO: Errors are still not counted properly.
 */

/* 1992/9/20
 * Modifications for ``Sector Shifting'' by Rob Hooft (hooft@chem.ruu.nl)
 * modelled after the freeware MS/DOS program fdformat/88 V1.8 by
 * Christoph H. Hochst\"atter.
 * I have fixed the shift values to the ones I always use. Maybe a new
 * ioctl() should be created to be able to modify them.
 * There is a bug in the driver that makes it impossible to format a
 * floppy as the first thing after bootup.
 */

/*
 * 1993/4/29 -- Linus -- cleaned up the timer handling in the kernel, and
 * this helped the floppy driver as well. Much cleaner, and still seems to
 * work.
 */

// unitno identifica il controller e basta.
// i dischi sono indirizzati dalla struttura block_request_t che contiene il numero del disco (unitno) e il numero del blocco (start_block) da leggere/scrivere. Il controller floppy gestisce i dischi collegati ad esso, quindi unitno serve per selezionare il controller corretto, mentre start_block indica quale blocco del disco specifico deve essere letto o scritto.
// per operazioni sui dischi si usa ioctl

// rivedi risistema e semplifica

#define FLOPPY_DMA 2

#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <diegos/timers.h>
#include <diegos/drivers.h>
#include <diegos/hal_interrupts.h>
#include <platform/i8259.h>
#include <platform/i8237.h>
#include <platform/BIOS.h>
#include <platform/int86.h>
#include <processor/ports.h>

#include "fdc_private.h"

#define READ 0
#define WRITE 1

void (*do_floppy)(void) = NULL;

#define TYPE(x) ((x) >> 2)
#define DRIVE(x) ((x) & 0x03)

#define BLOCK_SIZE 512

/*
 * Maximum disk size (in kilobytes). This default is used whenever the
 * current disk size is unknown.
 */
#define MAX_DISK_SIZE 1440

#define FD_FILL_BYTE 0xF6	/* format fill byte */

#define NO_TRACK 255

/* DMA commands */
#define DMA_READ 0x46
#define DMA_WRITE 0x4A

/* FDC version return types */
#define FDC_TYPE_STD 0x80	/* normal 8272A clone FDC */
#define FDC_TYPE_82077 0x81	/* Intel 82077 */
#define FDC_TYPE_72065 0x90	/* NEC 72065 */

#define NR_DRIVES 2

/* Two combinations of diskette/drive are supported.
 *
 * # Diskette Drive  Sectors  Tracks   Rotation Data-rate
 * 0   720K    720K     9       80     300 RPM  250 kbps
 * 1   1.44M   1.44M   18	80     300 RPM  500 kbps
 *
 * In addition, 720K diskettes can be read in 1.44MB drives, but that does
 * not need a different set of parameters.  This combination uses
 *
 * 0   720K    1.44M    9       80     300 RPM  250 kbps
 */
static const struct density {
	uint8_t secpt;		/* sectors per track */
	uint8_t cyls;		/* tracks per side (cylinders) */
	uint8_t test;		/* sector to try for density test */
	uint8_t rate;		/* data rate (2=250, 1=300, 0=500 kbps) */
	int start_ms;		/* motor start (milliseconds) */
	uint8_t gap;		/* gap size */
	uint8_t spec1;		/* first specify byte (SRT/HUT) */
	char *name;		/* name of this density */
} fdensity[] = {
	{9, 80, 4 * 9, 2, 500, 0x2A, 0xDF, "720 KB 3.5\" drive"},	/*  720K / 720K  */
	{18, 80, 17, 0, 500, 0x1B, 0xCF, "1.44 MB 3.5\" drive"}	/* 1.44M / 1.44M */
};

/* Auto-detection: Disk type used until the next media change occurs. */
static const struct density *current_type[NR_DRIVES] = { NULL, NULL };

/*
 * Track buffer and block buffer.
 * Because this is written to by the DMA controller, it must
 * not contain a 64k byte boundary crossing, or data will be
 * corrupted/lost.
 * Hard-coded to ISA DMA BUFFER 2 at 0x20000,
 * which is below 16MB and doesn't cross a 64k boundary.
 */
static char *floppy_track_buffer = NULL;

static volatile unsigned flags = (DRV_IS_BLOCK);

//static void redo_fd_request(void);

enum floppy_FSM {
	UNDEFINED,
	RESET,
	CONFIGURE,
	RECALIBRATE,
	// PROBE,
	READY,
	SEEK,
	IOREADY
};

#define MAX_BUF_SIZE 9
#define STATUS(x) (controller_ctx.buffer_in[x])
#define PARAM(x) (controller_ctx.buffer_out[x])

static struct controller_context {
	uint8_t DOR;		/* digital output register */
	uint8_t version;	/* FDC version code */
	uint8_t drive;		/* drive number */
	volatile enum floppy_FSM fsm;	/* finite state machine state */
	int last_status;	/* last op status */
	uint8_t buffer_in[MAX_BUF_SIZE];
	uint8_t buffer_out[MAX_BUF_SIZE];
} controller_ctx = {
	DOR_DMA, 0, 255, UNDEFINED, -1, {0}, {0}
};

static struct disk_context {
	/* R/W position */

	/* head number */
	uint8_t head;
	/* track number */
	uint8_t cylinder;
	/* sector number */
	uint8_t sector;

	/* R/W operations */
	unsigned transfer_sectors;
	unsigned request_sectors;
	char *buffer;
	unsigned buffer_ofs;
	unsigned bytes;

	/* command to send to FDC */
	uint8_t command;

	/* seek operation */
	uint8_t seek_track;
	uint8_t current_track;

} disk_ctx[NR_DRIVES] = {
	{0, 0, 0, 0, 0, NULL, 0, 0, 0, NO_TRACK, NO_TRACK},
	{0, 0, 0, 0, 0, NULL, 0, 0, 0, NO_TRACK, NO_TRACK}
};

static wait_queue_t fdc_wait;

static void (*handler)(void) = NULL;

//static void floppy_ready(void);
static void seek_floppy(void);

static inline struct disk_context *get_disk_ctx(void)
{
	return (controller_ctx.drive < NR_DRIVES) ? &disk_ctx[controller_ctx.drive] : NULL;
}

static inline const struct density *get_floppy_type(void)
{
	return (controller_ctx.drive < NR_DRIVES) ? current_type[controller_ctx.drive] : NULL;
}

static inline uint8_t block_to_head(unsigned block)
{
	/* blocks are 0-based */
	return (block / get_floppy_type()->secpt) & 1;
}

static inline uint8_t block_to_cylinder(unsigned block)
{
	/* blocks are 0-based */
	return (block / (get_floppy_type()->secpt * 2));
}

static inline uint8_t block_to_sector(unsigned block)
{
	/*
	 * blocks are 0-based
	 */
	return (block % get_floppy_type()->secpt) + 1;
}

static inline unsigned block_to_multi_track(unsigned block)
{
	return (block % get_floppy_type()->secpt * 2);
}

/*
 * Check if the diskette in the drive has been changed or the drive selection changed.
 * Returns 1 if the diskette or the drive has changed, 0 otherwise.
 */
static inline int check_disk_change(uint16_t disk)
{
	if (controller_ctx.drive == disk) {
		return (in_byte(FDC_DIR) & 0x80) ? 1 : 0;
	}

	return 1;
}

static void floppy_motor_on(void)
{
	controller_ctx.DOR |= DOR_MOTOR0 << controller_ctx.drive;
	out_byte(FDC_DOR, controller_ctx.DOR);
}

static void floppy_motor_off(void)
{
	controller_ctx.DOR &= ~(DOR_MOTOR0 << controller_ctx.drive);
	out_byte(FDC_DOR, controller_ctx.DOR);
}

static void print_version(uint8_t fdc_version)
{
	switch (fdc_version) {
	case FDC_TYPE_STD:
		kdrvprintf("FDC: Standard PC-style floppy controller detected.\n");
		break;
	case FDC_TYPE_82077:
		kdrvprintf("FDC: 82077 or compatible floppy controller detected.\n");
		break;
	case FDC_TYPE_72065:
		kdrvprintf("FDC: NEC 72065 or compatible floppy controller detected.\n");
		break;
	default:
		kdrvprintf("FDC: Unknown floppy controller type #2x detected.\n", fdc_version);
		break;
	}
}

static void floppy_select(unsigned int nr)
{
	if (controller_ctx.drive == nr)
		return;

	controller_ctx.version = 0;
	controller_ctx.drive = nr;
	controller_ctx.fsm = UNDEFINED;
	controller_ctx.last_status = -1;
	memset(controller_ctx.buffer_in, 0, sizeof(controller_ctx.buffer_in));
	memset(controller_ctx.buffer_out, 0, sizeof(controller_ctx.buffer_out));

	controller_ctx.DOR &= ~(DOR_DRIVE_MASK | DOR_MOTOR_MASK);
	controller_ctx.DOR |= controller_ctx.drive & DOR_DRIVE_MASK;
	out_byte(FDC_DOR, controller_ctx.DOR);

	get_disk_ctx()->head = 0;
	get_disk_ctx()->cylinder = 0;
	get_disk_ctx()->sector = 0;
	get_disk_ctx()->seek_track = NO_TRACK;
	get_disk_ctx()->current_track = NO_TRACK;
	get_disk_ctx()->command = 0;
}

static void setup_DMA(void)
{
	uintptr_t addr = (uintptr_t) floppy_track_buffer;
	unsigned int transfer_bytes;
	unsigned int copy_bytes;

	get_disk_ctx()->transfer_sectors =
	    get_floppy_type()->secpt * 2 - get_disk_ctx()->sector + 1;
	if (get_disk_ctx()->request_sectors < get_disk_ctx()->transfer_sectors) {
		get_disk_ctx()->transfer_sectors = get_disk_ctx()->request_sectors;
	}

	transfer_bytes = get_disk_ctx()->transfer_sectors * BLOCK_SIZE;
	copy_bytes =
	    (transfer_bytes < get_disk_ctx()->bytes) ? transfer_bytes : get_disk_ctx()->bytes;

	if (get_disk_ctx()->command == FD_WRITE)
		memcpy(floppy_track_buffer,
		       get_disk_ctx()->buffer + get_disk_ctx()->buffer_ofs, copy_bytes);

	lock();
	disable_dma(FLOPPY_DMA);
	clear_dma_ff(FLOPPY_DMA);
	set_dma_mode(FLOPPY_DMA,
		     (get_disk_ctx()->command == FD_READ) ? DMA_MODE_READ : DMA_MODE_WRITE);
	set_dma_addr(FLOPPY_DMA, addr);
	set_dma_count(FLOPPY_DMA, transfer_bytes);
	enable_dma(FLOPPY_DMA);
	unlock();
}

/*
 * output_byte sends a byte to the FDC.
 * Check the status register 0x3F4 for the direction and ready bits.
 * When the status is READY and direction bit is 0, the FDC is ready to accept a byte.
 */
static void send_data(char byte)
{
	int counter = 10000;
	uint8_t status;

	do {
		status = in_byte(FDC_STATUS);
		status &= STATUS_READY | STATUS_DIR;
	} while (--counter && (status != STATUS_READY));

	if (counter) {
		out_byte(FDC_DATA, byte);
	} else {
		kdrvprintf("Unable to send byte to FDC\n");
	}
}

/*
 * send_command sends a command to the FDC.
 * Check the status register 0x3F4 for the direction and ready bits.
 * When the status is READY and direction bit is 0, the FDC is ready to accept a byte.
 * All bytes in the output buffer are sent to the FDC.
 */
static int send_command(unsigned count)
{
	unsigned counter = 10000;
	unsigned pos = 0;
	uint8_t status;

	while (pos < count) {
		do {
			status = in_byte(FDC_STATUS);
			status &= STATUS_READY | STATUS_DIR;
		} while (--counter && (status != STATUS_READY));

		if (counter) {
			out_byte(FDC_DATA, PARAM(pos++));
		} else {
			kdrvprintf("Unable to send byte %d to FDC\n", pos);
			return -1;
		}
	}

	return 0;
}

static int receive_result(void)
{
	unsigned pos = 0, counter = 10000, status;

	while (--counter && (pos < MAX_BUF_SIZE)) {
		status = in_byte(FDC_STATUS) & (STATUS_DIR | STATUS_READY | STATUS_BUSY);
		if (status == STATUS_READY) {
			return pos;
		}
		if (status == (STATUS_DIR | STATUS_READY | STATUS_BUSY)) {
			STATUS(pos++) = in_byte(FDC_DATA);
		}
	}

	if (counter == 0) {
		kdrvprintf("Timeout while receiving result from FDC\n");
	} else {
		kdrvprintf("Receive buffer overrun\n");
	}
	return -1;
}

#if 0
static void bad_flp_intr(void)
{
	int errors;

	controller_ctx.current_track = NO_TRACK;
	if (format_status == FORMAT_BUSY)
		errors = ++format_errors;
	else if (!CURRENT) {
		kdrvprintf("no current request\n");
		fdc_flags |= RESET | RECALIBRATE;
		return;
	} else
		errors = ++CURRENT->errors;
	if (errors > MAX_ERRORS) {
		request_done(0);
	}
	if (errors > MAX_ERRORS / 2)
		fdc_flags |= RESET;
	else
		fdc_flags |= RECALIBRATE;
}
#endif
static void tell_sector(int nr)
{
	if (nr != 7) {
		kdrvprintf(" -- FDC reply error");
	} else
		kdrvprintf(": track %d, head %d, sector %d", STATUS(3), STATUS(4), STATUS(5));
}				/* tell_sector */

/*
 * Ok, this interrupt is called after a DMA read/write has succeeded
 * or failed, so we check the results, and copy any buffers.
 * hhb: Added better error reporting.
 */
static void rw_interrupt(void)
{
	int nr;
	// char bad;

	nr = receive_result();
	/* check IC to find cause of interrupt */
	switch (STATUS(0) & ST0_INTR) {
	case ST0_ABNORM:	/* error occured during command execution */
		//     bad = 1;
		if (STATUS(1) & ST1_WP) {
			kdrvprintf("Drive %d is write protected\n", controller_ctx.drive);
			// request_done(0);
			//              bad = 0;
		} else if (STATUS(1) & ST1_OR) {
			kdrvprintf("Over/Underrun - retrying\n");
			/* could continue from where we stopped, but ... */
			//              bad = 0;
		} else {
			kdrvprintf("%d: ", STATUS(0) & ST0_DS);
			if (STATUS(0) & ST0_ECE) {
				kdrvprintf("Recalibrate failed!");
			} else if (STATUS(2) & ST2_CRC) {
				kdrvprintf("data CRC error");
				tell_sector(nr);
			} else if (STATUS(1) & ST1_CRC) {
				kdrvprintf("CRC error");
				tell_sector(nr);
			} else if ((STATUS(1) & (ST1_MAM | ST1_ND)) || (STATUS(2) & ST2_MAM)) {
			} else if (STATUS(2) & ST2_WC) {	/* seek error */
				kdrvprintf("wrong cylinder");
			} else if (STATUS(2) & ST2_BC) {	/* cylinder marked as bad */
				kdrvprintf("bad cylinder");
			} else {
				kdrvprintf("unknown error. ST[0..3] are: 0x%x 0x%x 0x%x 0x%x\n",
					   STATUS(0), STATUS(1), STATUS(2), STATUS(3));
			}
			kdrvprintf("\n");
		}
		// if (bad)
		//      bad_flp_intr();
		// redo_fd_request();
		return;
	case 2:		/* invalid command given */
		kdrvprintf("Invalid FDC command given!\n");
		// request_done(0);
		return;
	case 3:
		kdrvprintf("Abnormal termination caused by polling\n");
		// bad_flp_intr();
		// redo_fd_request();
		return;
	default:		/* (0) Normal command termination */
		break;
	}

	// if (probing)
	//{
	//      int drive = CURRENT->disk;
	//
	//              kdrvprintf("Auto-detected floppy type %s in fd%d\n", floppy->name, drive);
	//              current_type[drive] = floppy;
	//              floppy_sizes[drive] = floppy->size >> 1;
	//              probing = 0;
	//      }
	//      if (command == FD_READ)
	//      {
	//              memcpy((char *)CURRENT->buffer + request_offset,
	//                     floppy_track_buffer, BLOCK_SIZE);
	//      }
	//      request_offset += BLOCK_SIZE;
	//      if (request_offset < CURRENT->bytes)
	//              redo_fd_request();
	//      else
	//              request_done(1);
}

/*
 * Update R/W parameters in case of SUCCESS ONLY !!!
 */
static void update_rw_floppy(void)
{
	unsigned transfer_bytes = get_disk_ctx()->transfer_sectors * BLOCK_SIZE;
	unsigned copy_bytes =
	    (transfer_bytes < get_disk_ctx()->bytes) ? transfer_bytes : get_disk_ctx()->bytes;

	if (get_disk_ctx()->command == FD_READ)
		memcpy(get_disk_ctx()->buffer + get_disk_ctx()->buffer_ofs,
		       floppy_track_buffer, copy_bytes);

	get_disk_ctx()->buffer_ofs += copy_bytes;
	get_disk_ctx()->request_sectors -= get_disk_ctx()->transfer_sectors;
	get_disk_ctx()->bytes -= copy_bytes;

	/*
	 * More data to read or write, update CHS and seek accordingly.
	 * Note: since we use multitrack feature, if we have more data to
	 * process we are seeking the next cylinder and we are starting from
	 * sector 1, head 0 !!!
	 */
	if (get_disk_ctx()->request_sectors) {
		get_disk_ctx()->head = 0;
		get_disk_ctx()->cylinder++;
		get_disk_ctx()->sector = 1;
		get_disk_ctx()->seek_track++;
	}
}

/*
 * We try to read tracks, but if we get too many errors, we
 * go back to reading just one sector at a time.
 *
 * This means we should be able to read a sector even if there
 * are other bad sectors on this track.
 */
static void setup_rw_floppy(void)
{
	setup_DMA();
	do_floppy = rw_interrupt;
	/*
	 * Standard parameters
	 */
	/* command */
	PARAM(0) = get_disk_ctx()->command;
	/* driver select 0-3 */
	PARAM(1) = controller_ctx.drive | (get_disk_ctx()->head << 2);
	/* cylinder number 0-79 */
	PARAM(2) = get_disk_ctx()->cylinder;
	/* head number */
	PARAM(3) = get_disk_ctx()->head;
	/* first sector */
	PARAM(4) = get_disk_ctx()->sector;
	/* sector size */
	PARAM(5) = 2;		/* sector size = 512 */
	/* Last sector in track */
	PARAM(6) = get_floppy_type()->secpt;
	/* gap length */
	PARAM(7) = get_floppy_type()->gap;
	/* custom sector size, set to 0xFF  */
	PARAM(8) = 0xFF;	/* sector size (0xff when n!=0 ?) */
	send_command(9);
}

static void prepare_rw_floppy(block_request_t *req)
{
	get_disk_ctx()->head = block_to_head(req->start_block);
	get_disk_ctx()->cylinder = block_to_cylinder(req->start_block);
	get_disk_ctx()->sector = block_to_sector(req->start_block);
	get_disk_ctx()->request_sectors = (req->bytes + BLOCK_SIZE - 1) / BLOCK_SIZE;
	get_disk_ctx()->command = (req->cmd == 1) ? FD_WRITE : FD_READ;
	get_disk_ctx()->seek_track = get_disk_ctx()->cylinder;
	get_disk_ctx()->buffer = req->buffer;
	get_disk_ctx()->bytes = req->bytes;
}

/*
 * This routine is called when everything should be correctly set up
 * for the transfer (ie floppy motor is on and the correct floppy is
 * selected).
 */
static int do_transfer(void)
{
	unsigned retries = 3;

	while (retries && get_disk_ctx()->bytes) {
		seek_floppy();

		if (ETIMEDOUT == thread_io_wait_timed(&fdc_wait, 100)) {
			kdrvprintf("%s: seek timeout\n", __FUNCTION__);
			if (retries-- == 0)
				return (EIO);
		} else {
			retries = 3;
		}

		update_rw_floppy();
		setup_rw_floppy();
		// MISSING error handling
	}

	return (retries) ? EOK : EIO;
}

/*
 * This is the routine called after every seek (or recalibrate) interrupt
 * from the floppy controller. Note that the "unexpected interrupt" routine
 * also does a recalibrate, but doesn't come here.
 */
static void seek_interrupt(void)
{
	static const uint8_t mask = ST0_INTR | ST0_SE | ST0_ECE | ST0_NR;
	int ret;
	uint8_t s0, s1;

	/* sense drive status */
	send_data(FD_SENSEI);
	ret = receive_result();
	s0 = STATUS(0) & mask;
	s1 = STATUS(1);
	if (ret != 2 || (s0 != ST0_SE) || (s1 != get_disk_ctx()->seek_track)) {
		kdrvprintf("seek failed\n");
		controller_ctx.fsm = RECALIBRATE;
	} else {
		get_disk_ctx()->current_track = s1;
		controller_ctx.fsm = IOREADY;
	}
	thread_io_resume(&fdc_wait);
}

static void unexpected_floppy_interrupt(void)
{
	// current_track = NO_TRACK;
	//  FIXME put back the code
	send_data(FD_SENSEI);
	(void)receive_result();
	kdrvprintf("unexpected interrupt %#x %#x\n", STATUS(0), STATUS(1));
}

static void recal_interrupt(void)
{
	static const uint8_t mask = ST0_ABNORM | ST0_SE;
	int ret;
	uint8_t /*s0, */ s1;

	send_data(FD_SENSEI);
	ret = receive_result();
	//s0 = STATUS(0);
	s1 = STATUS(1);

	if (ret != 2) {
		controller_ctx.fsm = UNDEFINED;
		controller_ctx.last_status = -1;
	} else if ((s1 & mask) == mask) {
		controller_ctx.fsm = UNDEFINED;
		controller_ctx.last_status = -1;
	}
	/* Recalibrate until track 0 is reached. Might help on some errors. */
	else if (s1 & ST0_ECE) {
		controller_ctx.last_status = -1;	/* FIXME: should limit nr of recalibrates */
	} else {
		controller_ctx.last_status = 0;
		controller_ctx.fsm = READY;
	}
	thread_io_resume(&fdc_wait);
}

/*
 * Must do 4 FD_SENSEIs after reset because of ``drive polling''.
 */
static void reset_interrupt(void)
{
	short i;

	for (i = 0; i < 4; i++) {
		send_data(FD_SENSEI);
		(void)receive_result();
	}
	controller_ctx.fsm = RESET;
	controller_ctx.last_status = 0;
	thread_io_resume(&fdc_wait);
}

/*
 * reset is done by pulling bit 2 of DOR low for a while.
 */
static void reset_controller(void)
{
	int i;

	do_floppy = reset_interrupt;
	controller_ctx.fsm = UNDEFINED;

	out_byte(FDC_DOR, controller_ctx.DOR & ~DOR_RESET);
	for (i = 0; i < 1000; i++)
		__asm__ volatile ("nop");
	out_byte(FDC_DOR, controller_ctx.DOR);
}

static void configure_controller(void)
{
	// do_floppy = reset_interrupt; should we set some default handler here?
	controller_ctx.buffer_out[0] = FD_SPECIFY;
	controller_ctx.buffer_out[1] = get_floppy_type()->spec1;	/* hut etc */
	controller_ctx.buffer_out[2] = 2;	/* Head load time =4ms, DMA */
	send_command(3);
	controller_ctx.fsm = CONFIGURE;
	controller_ctx.last_status = 0;
}

static void recalibrate_floppy(void)
{
	do_floppy = recal_interrupt;
	controller_ctx.buffer_out[0] = FD_RECALIBRATE;
	controller_ctx.buffer_out[1] = controller_ctx.drive;
	send_command(2);
	controller_ctx.fsm = RECALIBRATE;
}

static void seek_floppy(void)
{
	controller_ctx.fsm = SEEK;

	if (get_disk_ctx()->seek_track == get_disk_ctx()->current_track) {
		return;
	};

	do_floppy = seek_interrupt;
	controller_ctx.buffer_out[0] = FD_SEEK;
	controller_ctx.buffer_out[1] = controller_ctx.drive;
	controller_ctx.buffer_out[2] = get_disk_ctx()->seek_track;
	send_command(3);
}

static int run_fsm_to_ready(uint8_t drive)
{
	unsigned retries = 3;
	// enum floppy_FSM last_state;

	kdrvprintf("%s: AAAA %d\n", __FUNCTION__, controller_ctx.fsm);
	/*
	 * Disk changed or we selected a different driver,
	 * we need to restart the FSM from UNDEFINED
	 */
	if (check_disk_change(drive)) {
		kdrvprintf("%s: BBBB %d\n", __FUNCTION__, controller_ctx.fsm);
		floppy_select(drive);
	}

	kdrvprintf("%s: CCCC %d\n", __FUNCTION__, controller_ctx.fsm);

	while (controller_ctx.fsm != READY) {
		// last_state = controller_ctx.fsm;

		switch (controller_ctx.fsm) {
		case UNDEFINED:
			reset_controller();
			break;
		case RESET:
			configure_controller();
			break;
		case CONFIGURE:
			floppy_motor_on();
			thread_io_wait_timed(&fdc_wait, get_floppy_type()->start_ms);
			recalibrate_floppy();
			break;
		case RECALIBRATE:
			// recalibrate_floppy(); // already done
			break;
			//              case PROBE:
			// probe_floppy(); // already done
			//                      break;
		case READY:
			return (EOK);
		default:
			kdrvprintf("%s: unknown state %d\n", __FUNCTION__, controller_ctx.fsm);
			return (EIO);
		}

		if (ETIMEDOUT == thread_io_wait_timed(&fdc_wait, 100)) {
			kdrvprintf("do_fd_request: controller reset timeout\n");
			if (retries-- == 0)
				return (EIO);
		} else {
			retries = 3;
		}
	}

	return (EOK);
}

static int do_fd_request(block_request_t *req)
{
	int retcode;

	if (!req || req->disk > NR_DRIVES)
		return (EINVAL);
	if (!req->bytes)
		return (EOK);
	if (!req->buffer || (req->bytes % BLOCK_SIZE))
		return (EINVAL);

	req->errors = 0;

	run_fsm_to_ready(req->disk);
	prepare_rw_floppy(req);
	setup_rw_floppy();
	retcode = do_transfer();

	if (retcode != EOK && !req->errors)
		req->errors = 1;
	else if (retcode == EOK)
		req->errors = 0;

	return (retcode);
}

#if 0
static int fd_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long param)
{
	int i, drive, cnt, okay;
	struct floppy_struct *this_floppy;

	switch (cmd) {
		RO_IOCTLS(inode->i_rdev, param);
	}
	drive = MINOR(inode->i_rdev);
	switch (cmd) {
	case FDFMTBEG:
		if (!suser())
			return -EPERM;
		return 0;
	case FDFMTEND:
		if (!suser())
			return -EPERM;
		lock();
		fake_change |= 1 << (drive & 3);
		unlock();
		drive &= 3;
		cmd = FDCLRPRM;
		break;
	case FDGETPRM:
		if (drive > 3)
			this_floppy = &floppy_type[drive >> 2];
		else if ((this_floppy = current_type[drive & 3]) == NULL)
			return -ENODEV;
		i = verify_area(VERIFY_WRITE, (void *)param, sizeof(struct floppy_struct));
		if (i)
			return i;
		for (cnt = 0; cnt < sizeof(struct floppy_struct); cnt++)
			put_fs_byte(((char *)this_floppy)[cnt], (char *)param + cnt);
		return 0;
	case FDFMTTRK:
		if (!suser())
			return -EPERM;
		if (fd_ref[drive & 3] != 1)
			return -EBUSY;
		lock();
		while (format_status != FORMAT_NONE)
			thread_io_wait(&format_done);
		for (cnt = 0; cnt < sizeof(struct format_descr); cnt++)
			((char *)&format_req)[cnt] = get_fs_byte((char *)param + cnt);
		format_req.device = drive;
		format_status = FORMAT_WAIT;
		format_errors = 0;
		while (format_status != FORMAT_OKAY && format_status != FORMAT_ERROR) {
			if (fdc_busy)
				thread_io_wait(&fdc_wait);
			else {
				fdc_busy = 1;
				redo_fd_request();
			}
		}
		while (format_status != FORMAT_OKAY && format_status != FORMAT_ERROR)
			thread_io_wait(&format_done);
		unlock();
		okay = format_status == FORMAT_OKAY;
		format_status = FORMAT_NONE;
		floppy_off(drive & 3);
		thread_io_resume(&format_done);
		return okay ? 0 : -EIO;
	case FDFLUSH:
		if (!permission(inode, 2))
			return -EPERM;
		lock();
		fake_change |= 1 << (drive & 3);
		unlock();
		check_disk_change(inode->i_rdev);
		return 0;
	}
	if (!suser())
		return -EPERM;
	if (drive < 0 || drive > 3)
		return -EINVAL;
	switch (cmd) {
	case FDCLRPRM:
		current_type[drive] = NULL;
		floppy_sizes[drive] = MAX_DISK_SIZE;
		keep_data[drive] = 0;
		break;
	case FDSETPRM:
	case FDDEFPRM:
		memcpy_fromfs(user_params + drive, (void *)param, sizeof(struct floppy_struct));
		current_type[drive] = &user_params[drive];
		floppy_sizes[drive] = user_params[drive].size >> 1;
		if (cmd == FDDEFPRM)
			keep_data[drive] = -1;
		else {
			lock();
			while (fdc_busy)
				thread_io_wait(&fdc_wait);
			fdc_busy = 1;
			unlock();
			out_byte(FDC_DOR, (current_DOR & 0xfc) | drive | (0x10 << drive));
			for (cnt = 0; cnt < 1000; cnt++)
				__asm__("nop");
			if (in_byte(FDC_DIR) & 0x80)
				keep_data[drive] = 1;
			else
				keep_data[drive] = 0;
			out_byte(FDC_DOR, current_DOR);
			fdc_busy = 0;
			thread_io_resume(&fdc_wait);
		}
		break;
	case FDMSGON:
		ftd_msg[drive] = 1;
		break;
	case FDMSGOFF:
		ftd_msg[drive] = 0;
		break;
	case FDSETEMSGTRESH:
		min_report_error_cnt[drive] = (unsigned short)(param & 0x0f);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
#endif

#define CMOS_READ(addr) ({    \
	out_byte(0x70, addr); \
	in_byte(0x71);        \
})

static void config_types(void)
{
	uint8_t disk_type0 = CMOS_READ(0x10) & 0xF;
	uint8_t disk_type1 = (CMOS_READ(0x10) >> 4) & 0xF;

	/*
	 *
	 * CMOS values are (nibble)
	 *
	 * 0 No drive
	 * 1 360K 5.25"
	 * 2 1.2M 5.25"
	 * 3 720K 3.5"
	 * 4 1.44M 3.5"
	 * 5 2.88M 3.5"
	 *
	 * We support only 720K and 1.44M diskettes, so we ignore values other than 3 and 4.
	 */

	if (disk_type0 != 3 && disk_type0 != 4) {
		kdrvprintf("disk 0 is unknown type %d\n", disk_type0);
		disk_type0 = 0;
		current_type[0] = NULL;
	} else {
		current_type[0] = &fdensity[disk_type0 - 3];
	}

	if (disk_type1 != 3 && disk_type1 != 4) {
		kdrvprintf("disk 1 is unknown type %d\n", disk_type1);
		disk_type1 = 0;
		current_type[1] = NULL;
	} else {
		current_type[1] = &fdensity[disk_type1 - 3];
	}

	kdrvprintf("disk 0 is %s\n", current_type[0] ? current_type[0]->name : "unknown");
	kdrvprintf("disk 1 is %s\n", current_type[1] ? current_type[1]->name : "unknown");
}

static BOOL floppy_interrupt(void)
{
	if (!handler)
		handler = unexpected_floppy_interrupt;
	handler();

	return (TRUE);
}

static int fdc_init(unsigned unitno)
{
	if (unitno)
		return (ENXIO);

	if (flags & DRV_STATUS_RUN) {
		return (EOK);
	}

	floppy_track_buffer = (char *)ISA_DMA_real_buffer(1);
	if (!floppy_track_buffer)
		return (ENOMEM);

	if (EOK != thread_io_wait_init(&fdc_wait)) {
		return (EPERM);
	}

	add_int_cb(floppy_interrupt, FLOPPY_IVT);

	config_types();

	/*
	 * We should detect the controller at this point, but the controller
	 * will throw an unexpected interrupt and this would happen
	 * during the kernel boot when interrupts are off.
	 * As soon as interrupts goes on the unexpected interrupt will be handled
	 * and the controller will be reset and initialized.
	 * But - the system will not be operational.
	 * Detection is moved into the start routine, which is called after the kernel is up and running.
	 * Reset the controller.
	 */
	out_byte(FDC_DOR, 0);

	return (EOK);
}

static int fdc_start(unsigned unitno)
{
	if (unitno)
		return (ENXIO);

	if (flags & DRV_STATUS_RUN) {
		return (EOK);
	}

	enable_int(FLOPPY_IVT);

	out_byte(FDC_DOR, DOR_RESET | DOR_DMA);

	send_data(FD_VERSION);	/* get FDC version code */

	if (receive_result() != 1) {
		kdrvprintf("FDC failed to return version byte\n");
		controller_ctx.version = FDC_TYPE_STD;
	} else {
		controller_ctx.version = STATUS(0);
	}

	print_version(controller_ctx.version);

	flags |= DRV_STATUS_RUN;

	return (EOK);
}

static int fdc_stop(unsigned unitno)
{
	int retval;

	if (unitno > 1)
		return (ENXIO);

	retval = thread_io_resume(&fdc_wait);

	flags &= ~(DRV_STATUS_RUN | DRV_STATUS_BUSY);
	floppy_motor_off();
	disable_int(FLOPPY_IVT);

	return (retval);
}

static unsigned fdc_status(unsigned unitno)
{
	if (unitno > 1)
		return (0);

	return (flags);
}

static int fdc_done(unsigned unitno)
{
	int retval = EOK;

	if (unitno > 1)
		return (ENXIO);

	retval = thread_io_resume(&fdc_wait);

	flags |= DRV_STATUS_DONE;

	if (del_int_cb(FLOPPY_IVT)) {
		retval = EFAULT;
	}

	controller_ctx.DOR = DOR_DMA;
	controller_ctx.drive = 255;
	controller_ctx.fsm = UNDEFINED;
	controller_ctx.last_status = -1;
	controller_ctx.version = 0;
	out_byte(FDC_DOR, controller_ctx.DOR);

	return (retval);
}

static int fdc_write(block_request_t *req, unsigned unitno)
{
	if (unitno > 1)
		return (ENXIO);

	if (!req || req->cmd != WRITE)
		return (EINVAL);

	return (do_fd_request(req));
}

static int fdc_read(block_request_t *req, unsigned unitno)
{
	if (unitno > 1)
		return (ENXIO);

	if (!req || req->cmd != READ)
		return (EINVAL);

	return (do_fd_request(req));
}

static int fdc_write_multi(block_request_t **reqs, unsigned reqnum, unsigned unitno)
{
	unsigned i;
	int retcode = EOK;

	if (unitno > 1)
		return (ENXIO);

	if (reqnum && !reqs)
		return (EINVAL);

	for (i = 0; i < reqnum; i++) {
		int result;

		if (!reqs[i] || reqs[i]->cmd != WRITE) {
			if (reqs[i])
				reqs[i]->errors = 1;
			if (retcode == EOK)
				retcode = EINVAL;
			continue;
		}

		result = do_fd_request(reqs[i]);
		if (result != EOK && retcode == EOK)
			retcode = result;
	}

	return (retcode);
}

static int fdc_read_multi(block_request_t **reqs, unsigned reqnum, unsigned unitno)
{
	unsigned i;
	int retcode = EOK;

	if (unitno > 1)
		return (ENXIO);

	if (reqnum && !reqs)
		return (EINVAL);

	for (i = 0; i < reqnum; i++) {
		int result;

		if (!reqs[i] || reqs[i]->cmd != READ) {
			if (reqs[i])
				reqs[i]->errors = 1;
			if (retcode == EOK)
				retcode = EINVAL;
			continue;
		}

		result = do_fd_request(reqs[i]);
		if (result != EOK && retcode == EOK)
			retcode = result;
	}

	return (retcode);
}

/*
 * This has been placed at the end so we do not need to
 * forward static declarations etc. etc.
 * Moreover, it is an initialization, no more than this...
 */
block_driver_t fdc_drv = {
	.cmn = {
		.name = "fd",
		.init_fn = fdc_init,
		.start_fn = fdc_start,
		.stop_fn = fdc_stop,
		.done_fn = fdc_done,
		.ioctrl_fn = NULL,
		.status_fn = fdc_status,
		.poll_fn = NULL},
	.block_size = BLOCK_SIZE,
	.block_count = 0,
	.write_fn = fdc_write,
	.read_fn = fdc_read,
	.write_multi_fn = fdc_write_multi,
	.read_multi_fn = fdc_read_multi
};
