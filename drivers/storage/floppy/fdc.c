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

#define FLOPPY_DMA 2

#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <diegos/timers.h>
#include <diegos/drivers.h>
#include <diegos/interrupts.h>
#include <platform/i8259.h>
#include <platform/i8237.h>
#include <platform/BIOS.h>
#include <processor/ports.h>

#include "fdc_private.h"

//FIXME -- this is a hack to get the floppy driver working without a block layer. 
//The floppy driver should be rewritten to use the block layer, but that is a lot of work,
//and I don't have any floppies to test with, so for now, this will do.
struct request {
	int dev;		/* -1 if no request */
	int cmd;		/* READ or WRITE */
	int errors;
	unsigned long sector;
	unsigned long nr_sectors;
	unsigned long current_nr_sectors;
	char * buffer;
	struct request * next;
} *CURRENT;

#define READ 0
#define WRITE 1

void (*do_floppy)(void) = NULL;

#define TYPE(x) ((x) >> 2)
#define DRIVE(x) ((x) & 0x03)

#define BLOCK_SIZE 512

/*
 * Note that MAX_ERRORS=X doesn't imply that we retry every bad read
 * max X times - some types of errors increase the errorcount by 2 or
 * even 3, so we might actually retry only X/2 times before giving up.
 */
#define MAX_ERRORS 12

/*
 * Maximum disk size (in kilobytes). This default is used whenever the
 * current disk size is unknown.
 */
#define MAX_DISK_SIZE 1440

/*
 * Maximum number of sectors in a track buffer. Track buffering is disabled
 * if tracks are bigger.
 */
#define MAX_BUFFER_SECTORS 18

/*
 * The DMA channel used by the floppy controller cannot access data at
 * addresses >= 16MB
 *
 * Went back to the 1MB limit, as some people had problems with the floppy
 * driver otherwise. It doesn't matter much for performance anyway, as most
 * floppy accesses go through the track buffer.
 */
#define LAST_DMA_ADDR (0x100000 - BLOCK_SIZE)

#define FDCLRPRM 0 /* clear user-defined parameters */
#define FDSETPRM 1 /* set user-defined parameters for current media */
#define FDDEFPRM 2 /* set user-defined parameters until explicitly cleared */
#define FDGETPRM 3 /* get disk parameters */
#define	FDMSGON  4 /* issue kernel messages on media type change */
#define	FDMSGOFF 5 /* don't issue kernel messages on media type change */
#define FDFMTBEG 6 /* begin formatting a disk */
#define	FDFMTTRK 7 /* format the specified track */
#define FDFMTEND 8 /* end formatting a disk */
#define FDSETEMSGTRESH	10	/* set fdc error reporting treshold */
#define FDFLUSH  11 /* flush buffers for media; either for verifying media, or for
                       handling a media change without closing the file
		       descriptor */

#define FD_FILL_BYTE 0xF6 /* format fill byte */

#define FORMAT_NONE	0	/* no format request */
#define FORMAT_WAIT	1	/* format request is waiting */
#define FORMAT_BUSY	2	/* formatting in progress */
#define FORMAT_OKAY	3	/* successful completion */
#define FORMAT_ERROR	4	/* formatting error */

/* DMA commands */
#define DMA_READ	0x46
#define DMA_WRITE	0x4A

/* FDC version return types */
#define FDC_TYPE_STD	0x80	/* normal 8272A clone FDC */
#define FDC_TYPE_82077	0x81	/* Intel 82077 */
#define FDC_TYPE_72065	0x90	/* NEC 72065 */
/*
 * globals used by 'result()'
 */
#define MAX_REPLIES 7
static uint8_t reply_buffer[MAX_REPLIES];
#define ST0 (reply_buffer[0])
#define ST1 (reply_buffer[1])
#define ST2 (reply_buffer[2])
#define ST3 (reply_buffer[3])

struct floppy_struct {
	unsigned int	size,		/* nr of 512-byte sectors total */
			sect,		/* sectors per track */
			head,		/* nr of heads */
			track,		/* nr of tracks */
			stretch;	/* !=0 means double track steps */
	uint8_t	gap,			/* gap1 size */
		rate,			/* data rate. |= 0x40 for perpendicular */
		spec1,			/* stepping rate, head unload time */
		fmt_gap;		/* gap2 size */
	char 	* name; 		/* used only for predefined formats */
};

struct format_descr {
	int device,head,track;
};

/*
 * This struct defines the different floppy types.
 *
 * The 'stretch' tells if the tracks need to be doubled for some
 * types (ie 360kB diskette in 1.2MB drive etc). Others should
 * be self-explanatory.
 */
static struct floppy_struct floppy_type[] = {
    {0, 0, 0, 0, 0, 0x00, 0x00, 0x00, 0x00, NULL},	/* no testing */
    {720, 9, 2, 40, 0, 0x2A, 0x02, 0xDF, 0x50, NULL},	/* 360kB PC diskettes */
    {2400, 15, 2, 80, 0, 0x1B, 0x00, 0xDF, 0x54, NULL}, /* 1.2 MB AT-diskettes */
    {720, 9, 2, 40, 1, 0x2A, 0x02, 0xDF, 0x50, NULL},	/* 360kB in 720kB drive */
    {1440, 9, 2, 80, 0, 0x2A, 0x02, 0xDF, 0x50, NULL},	/* 3.5" 720kB diskette */
    {720, 9, 2, 40, 1, 0x23, 0x01, 0xDF, 0x50, NULL},	/* 360kB in 1.2MB drive */
    {1440, 9, 2, 80, 0, 0x23, 0x01, 0xDF, 0x50, NULL},	/* 720kB in 1.2MB drive */
    {2880, 18, 2, 80, 0, 0x1B, 0x00, 0xCF, 0x6C, NULL}, /* 1.44MB diskette */
};

/*
 * Auto-detection. Each drive type has a pair of formats which are
 * used in succession to try to read the disk. If the FDC cannot lock onto
 * the disk, the next format is tried. This uses the variable 'probing'.
 */
static struct floppy_struct floppy_types[] = {
    {720, 9, 2, 40, 0, 0x2A, 0x02, 0xDF, 0x50, "360k/PC"},  /* 360kB PC diskettes */
    {720, 9, 2, 40, 0, 0x2A, 0x02, 0xDF, 0x50, "360k/PC"},  /* 360kB PC diskettes */
    {2400, 15, 2, 80, 0, 0x1B, 0x00, 0xDF, 0x54, "1.2M"},   /* 1.2 MB AT-diskettes */
    {720, 9, 2, 40, 1, 0x23, 0x01, 0xDF, 0x50, "360k/AT"},  /* 360kB in 1.2MB drive */
    {1440, 9, 2, 80, 0, 0x2A, 0x02, 0xDF, 0x50, "720k"},    /* 3.5" 720kB diskette */
    {1440, 9, 2, 80, 0, 0x2A, 0x02, 0xDF, 0x50, "720k"},    /* 3.5" 720kB diskette */
    {2880, 18, 2, 80, 0, 0x1B, 0x00, 0xCF, 0x6C, "1.44M"},  /* 1.44MB diskette */
    {1440, 9, 2, 80, 0, 0x2A, 0x02, 0xDF, 0x50, "720k/AT"}, /* 3.5" 720kB diskette */
};

/* Auto-detection: Disk type used until the next media change occurs. */
struct floppy_struct *current_type[4] = {NULL, NULL, NULL, NULL};

/* This type is tried first. */
struct floppy_struct *base_type[4];

/*
 * User-provided type information. current_type points to
 * the respective entry of this array.
 */
//struct floppy_struct user_params[4];

static int floppy_sizes[] = {
    MAX_DISK_SIZE, MAX_DISK_SIZE, MAX_DISK_SIZE, MAX_DISK_SIZE,
    360, 360, 360, 360,
    1200, 1200, 1200, 1200,
    360, 360, 360, 360,
    720, 720, 720, 720,
    360, 360, 360, 360,
    720, 720, 720, 720,
    1440, 1440, 1440, 1440};

/*
 * The driver is trying to determine the correct media format
 * while probing is set. rw_interrupt() clears it after a
 * successful access.
 */
static int probing = 0;

/*
 * (User-provided) media information is _not_ discarded after a media change
 * if the corresponding keep_data flag is non-zero. Positive values are
 * decremented after each probe.
 */
static int keep_data[4] = {0, 0, 0, 0};

/*
 * Announce successful media type detection and media information loss after
 * disk changes.
 * Also used to enable/disable printing of overrun warnings.
 */
//static int ftd_msg[4] = {0, 0, 0, 0};

/* Prevent "aliased" accesses. */

//static int fd_ref[4] = {0, 0, 0, 0};
//static int fd_device[4] = {0, 0, 0, 0};

/* Synchronization of FDC access. */
static volatile int format_status = FORMAT_NONE, fdc_busy = 0;
static wait_queue_t fdc_wait, format_done;

/* Errors during formatting are counted here. */
static int format_errors;

/* Format request descriptor. */
static struct format_descr format_req;

/*
 * Current device number. Taken either from the block header or from the
 * format request descriptor.
 */
#define CURRENT_DEVICE (format_status == FORMAT_BUSY ? format_req.device : (CURRENT->dev))

/* Current error count. */
#define CURRENT_ERRORS (format_status == FORMAT_BUSY ? format_errors : (CURRENT->errors))

/*
 * Threshold for reporting FDC errors to the console.
 * Setting this to zero may flood your screen when using
 * ultra cheap floppies ;-)
 */
static unsigned short min_report_error_cnt[4] = {2, 2, 2, 2};

/*
 * Rate is 0 for 500kb/s, 1 for 300kbps, 2 for 250kbps
 * Spec1 is 0xSH, where S is stepping rate (F=1ms, E=2ms, D=3ms etc),
 * H is head unload time (1=16ms, 2=32ms, etc)
 *
 * Spec2 is (HLD<<1 | ND), where HLD is head load time (1=2ms, 2=4 ms etc)
 * and ND is set means no DMA. Hardcoded to 6 (HLD=6ms, use DMA).
 */

/*
 * Track buffer and block buffer (in case track buffering doesn't work).
 * Because these are written to by the DMA controller, they must
 * not contain a 64k byte boundary crossing, or data will be
 * corrupted/lost.
 * Hard-coded to ISA DMA BUFFER 1 at 0x10000,
 * which is below 16MB and doesn't cross a 64k boundary.
 */
static char *tmp_floppy_area = (char *) 0x10000;
static char *floppy_track_buffer = (char *) 0x10000 + BLOCK_SIZE;

#define NR_DRIVES 2

static timer_t *fdc_timer = NULL;
static timer_t *select_timer[NR_DRIVES] = {NULL, NULL};
static timer_t *motor_on_timer[NR_DRIVES] = {NULL, NULL};
static timer_t *motor_off_timer[NR_DRIVES] = {NULL, NULL};

static void redo_fd_request(void);

/*
 * These are global variables, as that's the easiest way to give
 * information to interrupts. They are the data used for the current
 * request.
 */
#define NO_TRACK 255

//static unsigned int changed_floppies = 0, fake_change = 0;
static unsigned int changed_floppies = 0;

enum {
	INITIAL_RESET = (1 << 0),
	NEED_CONFIGURE = (1 << 1),
	RECALIBRATE = (1 << 2),
	RESET = (1 << 3),
	RECOVER = (1 << 4),
	SEEK = (1 << 5),
	READ_TRACK = (1 << 6)
};

#if 0
static int initial_reset_flag = 0;
static int need_configure = 1; /* for 82077 */
static int recalibrate = 0;
static int reset = 0;
static int recover = 0; /* recalibrate immediately after resetting */
static int seek = 0;
static int read_track = 0; /* flag to indicate if we want to read entire track */
#endif

static unsigned fdc_flags = NEED_CONFIGURE;

static uint8_t running = 0;
static uint8_t current_DOR = DOR_RESET | DOR_DMA;
static int buffer_track = -1;
static int buffer_drive = -1;
static int cur_spec1 = -1;
static int cur_rate = -1;
static struct floppy_struct *floppy = floppy_type;
static uint8_t current_drive = 255;
static uint8_t sector = 0;
static uint8_t head = 0;
static uint8_t track = 0;
static uint8_t seek_track = 0;
static uint8_t current_track = NO_TRACK;
static uint8_t command = 0;
static uint8_t fdc_version = FDC_TYPE_STD; /* FDC version code */

static void floppy_ready(void);

static void timer_default(void *unused)
{
	/* Do nothing. */
}

static void select_callback(void *unused)
{
	floppy_ready();
}

static void print_version(unsigned unitno)
{
	switch (fdc_version) {
	case FDC_TYPE_STD:
		kdrvprintf("FDC%d : Standard PC-style floppy controller detected.\n", unitno);
		break;
	case FDC_TYPE_82077:
		kdrvprintf("FDC%d : 82077 or compatible floppy controller detected.\n", unitno);
		break;
	case FDC_TYPE_72065:
		kdrvprintf("FDC%d : NEC 72065 or compatible floppy controller detected.\n", unitno);
		break;
	default:
		kdrvprintf("FDC%d : Unknown floppy controller type 0x%x detected.\n", unitno, fdc_version);
		break;
	}
}

static void floppy_select(unsigned int nr)
{
	if (current_drive == (current_DOR & 3))
	{
		floppy_ready();
		return;
	}
	fdc_flags |= SEEK;
	current_track = NO_TRACK;
	current_DOR &= 0xFC;
	current_DOR |= current_drive;
	out_byte(FDC_DOR, current_DOR);

	if (EOK != timer_set(select_timer[nr], FALSE))
	{
		kdrvprintf("%s: timer_set failed\n", __func__);
		return;
	}

	if (EOK != timer_update_cb(select_timer[nr], select_callback, NULL))
	{
		kdrvprintf("%s: timer_update_cb failed\n", __func__);
		return;
	}

	if (EOK != timer_update(select_timer[nr], 2000, FALSE))
	{
		kdrvprintf("%s: timer_update failed\n", __func__);
		return;
	}
}

static void motor_on_callback(unsigned long nr)
{
	running |= DOR_MOTOR0 << nr;
	floppy_select(nr);
}

static void motor_off_callback(unsigned long nr)
{
	uint8_t mask = ~(DOR_MOTOR0 << nr);
	lock();
	running &= mask;
	current_DOR &= mask;
	out_byte(FDC_DOR, current_DOR);
	unlock();
}

static void floppy_on(unsigned int nr)
{
	uint8_t mask = DOR_MOTOR0 << nr;

	timer_set(motor_off_timer[nr], FALSE);
	if (mask & running)
		floppy_select(nr);
	if (!(mask & current_DOR))
	{
		timer_set(motor_on_timer[nr], FALSE);
		timer_update(motor_on_timer[nr], 1000, FALSE);
		timer_set(motor_on_timer[nr], TRUE);
	}
	current_DOR &= 0xFC;
	current_DOR |= mask;
	current_DOR |= nr;
	out_byte(FDC_DOR, current_DOR);
}

static void floppy_off(unsigned int nr)
{
	timer_set(motor_off_timer[nr], FALSE);
	timer_update(motor_off_timer[nr], 3000, FALSE);
	timer_set(motor_off_timer[nr], TRUE);
}

void request_done(int uptodate)
{
	//if (format_status != FORMAT_BUSY)
	//	end_request(uptodate);
	//else
	//{
		format_status = uptodate ? FORMAT_OKAY : FORMAT_ERROR;
		thread_io_resume(&format_done);
	//}
}

#if 0
/*
 * floppy-change is never called from an interrupt, so we can relax a bit
 * here, sleep etc. Note that floppy-on tries to set current_DOR to point
 * to the desired drive, but it will probably not survive the sleep if
 * several floppies are used at the same time: thus the loop.
 */
int floppy_change(struct buffer_head *bh)
{
	unsigned int mask = 1 << (bh->b_dev & 0x03);

	if (MAJOR(bh->b_dev) != MAJOR_NR)
	{
		kdrvprintf("floppy_changed: not a floppy\n");
		return 0;
	}
	if (fake_change & mask)
	{
		buffer_track = -1;
		fake_change &= ~mask;
		/* omitting the next line breaks formatting in a horrible way ... */
		changed_floppies &= ~mask;
		return 1;
	}
	if (changed_floppies & mask)
	{
		buffer_track = -1;
		changed_floppies &= ~mask;
		fdc_flags |= RECALIBRATE;
		return 1;
	}
	if (!bh)
		return 0;
	if (bh->b_dirt)
		ll_rw_block(WRITE, 1, &bh);
	else
	{
		buffer_track = -1;
		bh->b_uptodate = 0;
		ll_rw_block(READ, 1, &bh);
	}
	wait_on_buffer(bh);
	if (changed_floppies & mask)
	{
		changed_floppies &= ~mask;
		fdc_flags |= RECALIBRATE;
		return 1;
	}
	return 0;
}

#endif

static void setup_DMA(void)
{
	uintptr_t addr;
	unsigned long count;

	if (command == FD_FORMAT) {
		addr = (uintptr_t)tmp_floppy_area;
		count = floppy->sect * 4;
	} else {
		addr = (uintptr_t)CURRENT->buffer;
		count = 1024;
	}

	if (fdc_flags & READ_TRACK)	{
		/* mark buffer-track bad, in case all this fails.. */
		buffer_drive = buffer_track = -1;
		count = floppy->sect * floppy->head * 512;
		addr = (uintptr_t)floppy_track_buffer;
	} else if (addr >= LAST_DMA_ADDR) {
		addr = (uintptr_t)tmp_floppy_area;
		if (command == FD_WRITE)
			memcpy(tmp_floppy_area, CURRENT->buffer, BLOCK_SIZE);
	}

	lock();
	disable_dma(FLOPPY_DMA);
	clear_dma_ff(FLOPPY_DMA);
	set_dma_mode(FLOPPY_DMA, (command == FD_READ) ? DMA_MODE_READ : DMA_MODE_WRITE);
	set_dma_addr(FLOPPY_DMA, addr);
	set_dma_count(FLOPPY_DMA, count);
	enable_dma(FLOPPY_DMA);
	unlock();
}

static void output_byte(char byte)
{
	int counter;
	uint8_t status;

	if (fdc_flags & RESET)
		return;
	for (counter = 0; counter < 10000; counter++)
	{
		status = in_byte(FDC_STATUS) & (STATUS_READY | STATUS_DIR);
		if (status == STATUS_READY)
		{
			out_byte(FDC_DATA, byte);
			return;
		}
	}
	current_track = NO_TRACK;
	fdc_flags |= RESET;
	kdrvprintf("Unable to send byte to FDC\n");
}

static int result(void)
{
	int i = 0, counter, status;

	if (fdc_flags & RESET)
		return -1;
	for (counter = 0; counter < 10000; counter++)
	{
		status = in_byte(FDC_STATUS) & (STATUS_DIR | STATUS_READY | STATUS_BUSY);
		if (status == STATUS_READY)
		{
			return i;
		}
		if (status == (STATUS_DIR | STATUS_READY | STATUS_BUSY))
		{
			if (i >= MAX_REPLIES)
			{
				kdrvprintf("floppy_stat reply overrun\n");
				break;
			}
			reply_buffer[i++] = in_byte(FDC_DATA);
		}
	}
	fdc_flags |= RESET;
	current_track = NO_TRACK;
	kdrvprintf("Getstatus times out\n");
	return -1;
}

static void bad_flp_intr(void)
{
	int errors;

	current_track = NO_TRACK;
	if (format_status == FORMAT_BUSY)
		errors = ++format_errors;
	else if (!CURRENT)
	{
		kdrvprintf("no current request\n");
		fdc_flags |= RESET | RECALIBRATE;
		return;
	}
	else
		errors = ++CURRENT->errors;
	if (errors > MAX_ERRORS)
	{
		request_done(0);
	}
	if (errors > MAX_ERRORS / 2)
		fdc_flags |= RESET;
	else
		fdc_flags |= RECALIBRATE;
}

/* Set perpendicular mode as required, based on data rate, if supported.
 * 82077 Untested! 1Mbps data rate only possible with 82077-1.
 * TODO: increase MAX_BUFFER_SECTORS, add floppy_type entries.
 */
static void perpendicular_mode(uint8_t rate)
{
	if (fdc_version == FDC_TYPE_82077)
	{
		output_byte(FD_PERPENDICULAR);
		if (rate & 0x40)
		{
			uint8_t r = rate & 0x03;
			if (r == 0)
				output_byte(2); /* perpendicular, 500 kbps */
			else if (r == 3)
				output_byte(3); /* perpendicular, 1Mbps */
			else
			{
				kdrvprintf("Invalid data rate for perpendicular mode!\n");
				fdc_flags |= RESET;
			}
		}
		else
			output_byte(0); /* conventional mode */
	}
	else
	{
		if (rate & 0x40)
		{
			kdrvprintf("perpendicular mode not supported by this FDC.\n");
			fdc_flags |= RESET;
		}
	}
} /* perpendicular_mode */

/*
 * This has only been tested for the case fdc_version == FDC_TYPE_STD.
 * In case you have a 82077 and want to test it, you'll have to compile
 * with `FDC_FIFO_UNTESTED' defined. You may also want to add support for
 * recognizing drives with vertical recording support.
 */
static void configure_fdc_mode(void)
{
	if ((fdc_flags & NEED_CONFIGURE) && (fdc_version == FDC_TYPE_82077))
	{
		/* Enhanced version with FIFO & vertical recording. */
		output_byte(FD_CONFIGURE);
		output_byte(0);
		output_byte(0x1A); /* FIFO on, polling off, 10 byte threshold */
		output_byte(0);	   /* precompensation from track 0 upwards */
		fdc_flags &= ~NEED_CONFIGURE;
		kdrvprintf("FIFO enabled\n");
	}
	if (cur_spec1 != floppy->spec1)
	{
		cur_spec1 = floppy->spec1;
		output_byte(FD_SPECIFY);
		output_byte(cur_spec1); /* hut etc */
		output_byte(6);		/* Head load time =6ms, DMA */
	}
	if (cur_rate != floppy->rate)
	{
		/* use bit 6 of floppy->rate to indicate perpendicular mode */
		perpendicular_mode(floppy->rate);
		out_byte(FDC_CFGCTRL, (cur_rate = (floppy->rate)) & ~0x40);
	}
} /* configure_fdc_mode */

static void tell_sector(int nr)
{
	if (nr != 7)
	{
		kdrvprintf(" -- FDC reply errror");
		fdc_flags |= RESET;
	}
	else
		kdrvprintf(": track %d, head %d, sector %d", reply_buffer[3],
		       reply_buffer[4], reply_buffer[5]);
} /* tell_sector */

/*
 * Ok, this interrupt is called after a DMA read/write has succeeded
 * or failed, so we check the results, and copy any buffers.
 * hhb: Added better error reporting.
 */
static void rw_interrupt(void)
{
	char *buffer_area;
	int nr;
	char bad;

	nr = result();
	/* check IC to find cause of interrupt */
	switch ((ST0 & ST0_INTR) >> 6)
	{
	case 1: /* error occured during command execution */
		bad = 1;
		if (ST1 & ST1_WP)
		{
			kdrvprintf("Drive %d is write protected\n", current_drive);
			request_done(0);
			bad = 0;
		}
		else if (ST1 & ST1_OR)
		{
			//if (ftd_msg[ST0 & ST0_DS])
				kdrvprintf("Over/Underrun - retrying\n");
			/* could continue from where we stopped, but ... */
			bad = 0;
		}
		else if (CURRENT_ERRORS > min_report_error_cnt[ST0 & ST0_DS])
		{
			kdrvprintf("%d: ", ST0 & ST0_DS);
			if (ST0 & ST0_ECE)
			{
				kdrvprintf("Recalibrate failed!");
			}
			else if (ST2 & ST2_CRC)
			{
				kdrvprintf("data CRC error");
				tell_sector(nr);
			}
			else if (ST1 & ST1_CRC)
			{
				kdrvprintf("CRC error");
				tell_sector(nr);
			}
			else if ((ST1 & (ST1_MAM | ST1_ND)) || (ST2 & ST2_MAM))
			{
				if (!probing)
				{
					kdrvprintf("sector not found");
					tell_sector(nr);
				}
				else
					kdrvprintf("probe failed...");
			}
			else if (ST2 & ST2_WC)
			{ /* seek error */
				kdrvprintf("wrong cylinder");
			}
			else if (ST2 & ST2_BC)
			{ /* cylinder marked as bad */
				kdrvprintf("bad cylinder");
			}
			else
			{
				kdrvprintf("unknown error. ST[0..3] are: 0x%x 0x%x 0x%x 0x%x\n", ST0, ST1, ST2, ST3);
			}
			kdrvprintf("\n");
		}
		if (bad)
			bad_flp_intr();
		redo_fd_request();
		return;
	case 2: /* invalid command given */
		kdrvprintf("Invalid FDC command given!\n");
		request_done(0);
		return;
	case 3:
		kdrvprintf("Abnormal termination caused by polling\n");
		bad_flp_intr();
		redo_fd_request();
		return;
	default: /* (0) Normal command termination */
		break;
	}

	if (probing)
	{
		int drive = CURRENT->dev;

		//if (ftd_msg[drive])
			kdrvprintf("Auto-detected floppy type %s in fd%d\n",
			       floppy->name, drive);
		current_type[drive] = floppy;
		floppy_sizes[drive] = floppy->size >> 1;
		probing = 0;
	}
	if (fdc_flags & READ_TRACK)
	{
		buffer_track = seek_track;
		buffer_drive = current_drive;
		buffer_area = floppy_track_buffer +
			      ((sector - 1 + head * floppy->sect) << 9);
		memcpy(buffer_area, CURRENT->buffer, BLOCK_SIZE);
	}
	else if (command == FD_READ &&
		 (unsigned long)(CURRENT->buffer) >= LAST_DMA_ADDR)
		memcpy(tmp_floppy_area, CURRENT->buffer, BLOCK_SIZE);
	request_done(1);
	redo_fd_request();
}

/*
 * We try to read tracks, but if we get too many errors, we
 * go back to reading just one sector at a time.
 *
 * This means we should be able to read a sector even if there
 * are other bad sectors on this track.
 */
void setup_rw_floppy(void)
{
	setup_DMA();
	do_floppy = rw_interrupt;
	output_byte(command);
	if (command != FD_FORMAT)
	{
		if (fdc_flags & READ_TRACK)
		{
			output_byte(current_drive);
			output_byte(track);
			output_byte(0);
			output_byte(1);
		}
		else
		{
			output_byte(head << 2 | current_drive);
			output_byte(track);
			output_byte(head);
			output_byte(sector);
		}
		output_byte(2); /* sector size = 512 */
		output_byte(floppy->sect);
		output_byte(floppy->gap);
		output_byte(0xFF); /* sector size (0xff when n!=0 ?) */
	}
	else
	{
		output_byte(head << 2 | current_drive);
		output_byte(2);
		output_byte(floppy->sect);
		output_byte(floppy->fmt_gap);
		output_byte(FD_FILL_BYTE);
	}
	if (fdc_flags & RESET)
		redo_fd_request();
}

/*
 * This is the routine called after every seek (or recalibrate) interrupt
 * from the floppy controller. Note that the "unexpected interrupt" routine
 * also does a recalibrate, but doesn't come here.
 */
static void seek_interrupt(void)
{
	/* sense drive status */
	output_byte(FD_SENSEI);
	if (result() != 2 || (ST0 & 0xF8) != 0x20 || ST1 != seek_track)
	{
		kdrvprintf("seek failed\n");
		fdc_flags |= RECALIBRATE;
		bad_flp_intr();
		redo_fd_request();
		return;
	}
	current_track = ST1;
	setup_rw_floppy();
}

/*
 * This routine is called when everything should be correctly set up
 * for the transfer (ie floppy motor is on and the correct floppy is
 * selected).
 */
static void transfer(void)
{
	if ((command == FD_READ) && (CURRENT_ERRORS < 4) &&
		     (floppy->sect <= MAX_BUFFER_SECTORS)) {
		fdc_flags |= READ_TRACK;
	} else {
		fdc_flags &= ~READ_TRACK;
	}

	configure_fdc_mode();

	if (fdc_flags & RESET)
	{
		redo_fd_request();
		return;
	}
	if (!(fdc_flags & SEEK))
	{
		setup_rw_floppy();
		return;
	}

	do_floppy = seek_interrupt;
	output_byte(FD_SEEK);
	if (fdc_flags & READ_TRACK)
		output_byte(current_drive);
	else
		output_byte((head << 2) | current_drive);
	output_byte(seek_track);
	if (fdc_flags & RESET)
		redo_fd_request();
}

/*
 * Special case - used after a unexpected interrupt (or reset)
 */

static void recalibrate_floppy(void);

static void recal_interrupt(void)
{
	output_byte(FD_SENSEI);
	current_track = NO_TRACK;
	if (result() != 2 || (ST0 & 0xE0) == 0x60)
		fdc_flags |= RESET;
	/* Recalibrate until track 0 is reached. Might help on some errors. */
	if ((ST0 & 0x10) == 0x10)
		recalibrate_floppy(); /* FIXME: should limit nr of recalibrates */
	else
		redo_fd_request();
}

static void unexpected_floppy_interrupt(void)
{
	current_track = NO_TRACK;
	output_byte(FD_SENSEI);
	kdrvprintf("unexpected interrupt\n");
	if (result() != 2 || (ST0 & 0xE0) == 0x60)
		fdc_flags |= RESET;
	else
		fdc_flags |= RECALIBRATE;
}

static void recalibrate_floppy(void)
{
	fdc_flags &= ~RECALIBRATE;
	current_track = 0;
	do_floppy = recal_interrupt;
	output_byte(FD_RECALIBRATE);
	output_byte(head << 2 | current_drive);
	if (fdc_flags & RESET)
		redo_fd_request();
}

/*
 * Must do 4 FD_SENSEIs after reset because of ``drive polling''.
 */
static void reset_interrupt(void)
{
	short i;

	for (i = 0; i < 4; i++)
	{
		output_byte(FD_SENSEI);
		(void)result();
	}
	output_byte(FD_SPECIFY);
	output_byte(cur_spec1); /* hut etc */
	output_byte(6);		/* Head load time =6ms, DMA */
	configure_fdc_mode();	/* reprogram fdc */
	if (fdc_flags & INITIAL_RESET)
	{
		fdc_flags &= ~INITIAL_RESET;
		fdc_flags |= RECALIBRATE;
		fdc_flags &= ~RESET;
		return;
	}
	if (!(fdc_flags & RECOVER))
		redo_fd_request();
	else
	{
		recalibrate_floppy();
		fdc_flags &= ~RECOVER;
	}
}

/*
 * reset is done by pulling bit 2 of DOR low for a while.
 */
static void reset_floppy(void)
{
	int i;

	do_floppy = reset_interrupt;
	fdc_flags &= ~RESET;
	current_track = NO_TRACK;
	cur_spec1 = -1;
	cur_rate = -1;
	fdc_flags |= RECALIBRATE | NEED_CONFIGURE;
	if (!(fdc_flags & INITIAL_RESET))
		kdrvprintf("Reset-floppy called\n");
	lock();
	out_byte(FDC_DOR, current_DOR & ~0x04);
	for (i = 0; i < 1000; i++)
		__asm__("nop");
	out_byte(FDC_DOR, current_DOR);
	unlock();
}

static void floppy_shutdown(void)
{
	lock();
	do_floppy = NULL;
	request_done(0);
	fdc_flags |= RECOVER;
	reset_floppy();
	unlock();
	redo_fd_request();
}

static void shake_done(void)
{
	current_track = NO_TRACK;
	if (in_byte(FDC_DIR) & 0x80)
		request_done(0);
	redo_fd_request();
}

static int retry_recal(void (*proc)(void))
{
	output_byte(FD_SENSEI);
	if (result() == 2 && (ST0 & 0x10) != 0x10)
		return 0;
	do_floppy = proc;
	output_byte(FD_RECALIBRATE);
	output_byte(head << 2 | current_drive);
	return 1;
}

static void shake_zero(void)
{
	if (!retry_recal(shake_zero))
		shake_done();
}

static void shake_one(void)
{
	if (retry_recal(shake_one))
		return;
	do_floppy = shake_done;
	output_byte(FD_SEEK);
	output_byte(head << 2 | current_drive);
	output_byte(1);
}

static void floppy_ready(void)
{
	if (in_byte(FDC_DIR) & 0x80)
	{
		changed_floppies |= 1 << current_drive;
		buffer_track = -1;
		if (keep_data[current_drive])
		{
			if (keep_data[current_drive] > 0)
				keep_data[current_drive]--;
		}
		else
		{
			//if (ftd_msg[current_drive] && current_type[current_drive] != NULL)
			if (current_type[current_drive] != NULL)
				kdrvprintf("Disk type is undefined after disk "
				       "change in fd%d\n",
				       current_drive);
			current_type[current_drive] = NULL;
			floppy_sizes[current_drive] = MAX_DISK_SIZE;
		}
		/* Forcing the drive to seek makes the "media changed" condition go away.
		 * There should be a cleaner solution for that ...
		 */
		if (!(fdc_flags & (RESET | RECALIBRATE)))
		{
			if (current_track && current_track != NO_TRACK)
				do_floppy = shake_zero;
			else
				do_floppy = shake_one;
			output_byte(FD_RECALIBRATE);
			output_byte(head << 2 | current_drive);
			return;
		}
	}
	if (fdc_flags & RESET)
	{
		reset_floppy();
		return;
	}
	if (fdc_flags & RECALIBRATE)
	{
		recalibrate_floppy();
		return;
	}
	transfer();
}

static void setup_format_params(void)
{
	uint8_t *here = (uint8_t *)tmp_floppy_area;
	unsigned int count;
	int head_shift, track_shift, total_shift;

	/* allow for about 30ms for data transport per track */
	head_shift = floppy->sect / 6;
	/* a ``cylinder'' is two tracks plus a little stepping time */
	track_shift = 2 * head_shift + 1;
	/* count backwards */
	total_shift = floppy->sect -
		      ((track_shift * track + head_shift * head) % floppy->sect);

	/* XXX: should do a check to see this fits in tmp_floppy_area!! */
	for (count = 0; count < floppy->sect; count++)
	{
		*here++ = track;
		*here++ = head;
		*here++ = 1 + ((count + total_shift) % floppy->sect);
		*here++ = 2; /* 512 bytes */
	}
}

static void redo_fd_request(void)
{
	unsigned int block;
	char *buffer_area;
	int device;

	if (CURRENT && CURRENT->dev < 0)
		return;

repeat:
	if (format_status == FORMAT_WAIT)
		format_status = FORMAT_BUSY;
	if (format_status != FORMAT_BUSY)
	{
		if (!CURRENT)
		{
			if (!fdc_busy)
				kdrvprintf("FDC access conflict!");
			fdc_busy = 0;
			thread_io_resume(&fdc_wait);
			//CLEAR_INTR;
			return;
		}
		//if (MAJOR(CURRENT->dev) != MAJOR_NR)
			//panic("request list destroyed");
		//if (CURRENT->bh)
		//{
			//if (!CURRENT->bh->b_lock)
				//panic("block not locked");
		//}
	}
	fdc_flags &= ~SEEK;
	probing = 0;
	device = CURRENT_DEVICE;
	if (device > 3)
		floppy = (device >> 2) + floppy_type;
	else
	{ /* Auto-detection */
		floppy = current_type[device & 3];
		if (!floppy)
		{
			probing = 1;
			floppy = base_type[device & 3];
			if (!floppy)
			{
				request_done(0);
				goto repeat;
			}
			if (CURRENT_ERRORS & 1)
				floppy++;
		}
	}
	if (format_status != FORMAT_BUSY)
	{
		if (current_drive != CURRENT->dev)
		{
			current_track = NO_TRACK;
			current_drive = CURRENT->dev;
		}
		block = CURRENT->sector;
		if (block + 2 > floppy->size)
		{
			request_done(0);
			goto repeat;
		}
		sector = block % floppy->sect;
		block /= floppy->sect;
		head = block % floppy->head;
		track = block / floppy->head;
		seek_track = track << floppy->stretch;
		if (CURRENT->cmd == READ)
			command = FD_READ;
		else if (CURRENT->cmd == WRITE)
			command = FD_WRITE;
		else
		{
			kdrvprintf("do_fd_request: unknown command\n");
			request_done(0);
			goto repeat;
		}
	}
	else
	{
		if (current_drive != (format_req.device & 3))
			current_track = NO_TRACK;
		current_drive = format_req.device & 3;
		if (((unsigned)format_req.track) >= floppy->track ||
		    (format_req.head & 0xfffe) || probing)
		{
			request_done(0);
			goto repeat;
		}
		head = format_req.head;
		track = format_req.track;
		seek_track = track << floppy->stretch;
		if (seek_track == buffer_track)
			buffer_track = -1;
		command = FD_FORMAT;
		setup_format_params();
	}
	timer_update(fdc_timer, 10*1000, FALSE);
	timer_set(fdc_timer, TRUE);
	if ((seek_track == buffer_track) &&
	    (current_drive == buffer_drive))
	{
		buffer_area = floppy_track_buffer +
			      ((sector + head * floppy->sect) << 9);
		if (command == FD_READ)
		{
			memcpy(CURRENT->buffer, buffer_area, BLOCK_SIZE);
			request_done(1);
			goto repeat;
		}
		else if (command == FD_WRITE)
			memcpy(buffer_area, CURRENT->buffer, BLOCK_SIZE);
	}
	if (seek_track != current_track)
		fdc_flags |= SEEK;
	sector++;
	timer_set(motor_off_timer[current_drive], FALSE);
	floppy_on(current_drive);
}

void do_fd_request(void)
{
	lock();
	while (fdc_busy)
		thread_io_wait(&fdc_wait);
	fdc_busy = 1;
	unlock();
	redo_fd_request();
}

#if  0
static int fd_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,
		    unsigned long param)
{
	int i, drive, cnt, okay;
	struct floppy_struct *this_floppy;

	switch (cmd)
	{
		RO_IOCTLS(inode->i_rdev, param);
	}
	drive = MINOR(inode->i_rdev);
	switch (cmd)
	{
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
			put_fs_byte(((char *)this_floppy)[cnt],
				    (char *)param + cnt);
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
			((char *)&format_req)[cnt] = get_fs_byte(
			    (char *)param + cnt);
		format_req.device = drive;
		format_status = FORMAT_WAIT;
		format_errors = 0;
		while (format_status != FORMAT_OKAY && format_status !=
							   FORMAT_ERROR)
		{
			if (fdc_busy)
				thread_io_wait(&fdc_wait);
			else
			{
				fdc_busy = 1;
				redo_fd_request();
			}
		}
		while (format_status != FORMAT_OKAY && format_status !=
							   FORMAT_ERROR)
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
	switch (cmd)
	{
	case FDCLRPRM:
		current_type[drive] = NULL;
		floppy_sizes[drive] = MAX_DISK_SIZE;
		keep_data[drive] = 0;
		break;
	case FDSETPRM:
	case FDDEFPRM:
		memcpy_fromfs(user_params + drive,
			      (void *)param,
			      sizeof(struct floppy_struct));
		current_type[drive] = &user_params[drive];
		floppy_sizes[drive] = user_params[drive].size >> 1;
		if (cmd == FDDEFPRM)
			keep_data[drive] = -1;
		else
		{
			lock();
			while (fdc_busy)
				thread_io_wait(&fdc_wait);
			fdc_busy = 1;
			unlock();
			out_byte(FDC_DOR, (current_DOR & 0xfc) | drive |
				   (0x10 << drive));
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

#define CMOS_READ(addr) ({  \
	out_byte(0x70, addr); \
	in_byte(0x71);        \
})

static struct floppy_struct *find_base(int drive, int code)
{
	struct floppy_struct *base;

	if (code > 0 && code < 5)
	{
		base = &floppy_types[(code - 1) * 2];
		kdrvprintf("fd%d is %s", drive, base->name);
		return base;
	}
	kdrvprintf("fd%d is unknown type %d", drive, code);
	return NULL;
}

static void config_types(void)
{
	kdrvprintf("Floppy drive(s): ");
	base_type[0] = find_base(0, (CMOS_READ(0x10) >> 4) & 15);
	if (((CMOS_READ(0x14) >> 6) & 1) == 0)
		base_type[1] = NULL;
	else
	{
		kdrvprintf(", ");
		base_type[1] = find_base(1, CMOS_READ(0x10) & 15);
	}
	base_type[2] = base_type[3] = NULL;
	kdrvprintf("\n");
}

#if 0
/*
 * floppy_open check for aliasing (/dev/fd0 can be the same as
 * /dev/PS0 etc), and disallows simultaneous access to the same
 * drive with different device numbers.
 */
static int floppy_open(struct inode *inode, struct file *filp)
{
	int drive;
	int old_dev;

	drive = inode->i_rdev & 3;
	old_dev = fd_device[drive];
	if (fd_ref[drive])
		if (old_dev != inode->i_rdev)
			return -EBUSY;
	fd_ref[drive]++;
	fd_device[drive] = inode->i_rdev;
	buffer_drive = buffer_track = -1;
	if (old_dev && old_dev != inode->i_rdev)
		invalidate_buffers(old_dev);
	if (filp && filp->f_mode)
		check_disk_change(inode->i_rdev);
	return 0;
}

static void floppy_release(struct inode *inode, struct file *filp)
{
	sync_dev(inode->i_rdev);
	if (!fd_ref[inode->i_rdev & 3]--)
	{
		kdrvprintf("floppy_release with fd_ref == 0");
		fd_ref[inode->i_rdev & 3] = 0;
	}
}

#endif

/*
 * The version command is not supposed to generate an interrupt, but
 * my FDC does, except when booting in SVGA screen mode.
 * When it does generate an interrupt, it doesn't return any status bytes.
 * It appears to have something to do with the version command...
 *
 * This should never be called, because of the reset after the version check.
 */
#if 0
static void ignore_interrupt(void)
{
	kdrvprintf("weird interrupt ignored (%d)\n", result());
	fdc_flags &= ~RESET;
	CLEAR_INTR; /* ignore only once */
}

static void floppy_interrupt(int unused)
{
	if (!handler)
		handler = unexpected_floppy_interrupt;
	handler();
}
#endif

static int fdc_init(unsigned unitno)
{
	char name[5] = {'F', 'D', 'C', '0' + unitno, 0};

	if (unitno >= NR_DRIVES)
		return (ENXIO);

	if (fdc_timer == NULL) {
		fdc_timer = timer_create(name, 1000, FALSE, timer_default, NULL);
		if (fdc_timer == NULL) {
			return (ENOMEM);
		}
	}

	name[0] = 'S';
	name[1] = 'E';
	name[2] = 'L';
	select_timer[unitno] = timer_create(name, 1000, FALSE, timer_default, NULL);
	if (select_timer[unitno] == NULL)
	{
		timer_done(fdc_timer);
		return (ENOMEM);
	}

	name[0] = 'M';
	name[1] = 'O';
	name[2] = 'N';
	motor_on_timer[unitno] = timer_create(name, 1000, FALSE, timer_default, NULL);
	if (motor_on_timer[unitno] == NULL)
	{
		timer_done(select_timer[unitno]);
		timer_done(fdc_timer);
		return (ENOMEM);
	}

	name[0] = 'M';
	name[1] = 'O';
	name[2] = 'F';
	motor_off_timer[unitno] = timer_create(name, 1000, FALSE, timer_default, NULL);
	if (motor_off_timer[unitno] == NULL)
	{
		timer_done(select_timer[unitno]);
		timer_done(motor_on_timer[unitno]);
		timer_done(fdc_timer);
		return (ENOMEM);
	}
	
	if ((EOK != thread_io_wait_init(&fdc_wait)) || (EOK != thread_io_wait_init(&format_done))) {
		timer_done(fdc_timer);
		timer_done(select_timer[unitno]);
		timer_done(motor_on_timer[unitno]);
		timer_done(motor_off_timer[unitno]);
		return (EPERM);
	}

	add_int_cb(f_handler, FLOPPY_IRQ);

	out_byte(FDC_DOR, current_DOR);

	config_types();

	output_byte(FD_VERSION);	/* get FDC version code */
	if (result() != 1) {
		kdrvprintf("FDC failed to return version byte\n");
		fdc_version = FDC_TYPE_STD;
	} else {
		fdc_version = reply_buffer[0];
	}
	print_version(unitno);

	/* Not all FDCs seem to be able to handle the version command
	 * properly, so force a reset for the standard FDC clones,
	 * to avoid interrupt garbage.
	 */

	if (fdc_version == FDC_TYPE_STD)
	{
		fdc_flags |= INITIAL_RESET;
		reset_floppy();
	}

	return (EOK);
}

static int fdc_start(unsigned unitno)
{
	if (unitno >= NR_DRIVES)
		return (ENXIO);

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
	.start_fn = driver_def_ok,
	.stop_fn = driver_def_ok,
	.done_fn = NULL,
	.ioctrl_fn = NULL,
	.status_fn = NULL,
	.poll_fn = NULL},
    .write_fn = NULL,
    .read_fn = NULL,
    .write_multi_fn = NULL,
    .read_multi_fn = NULL};
