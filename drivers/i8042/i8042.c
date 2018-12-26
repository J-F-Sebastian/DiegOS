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

#include <types_common.h>
#include <ia32/ports.h>
#include <pc/i8259.h>
#include <errno.h>
#include <diegos/kernel.h>
#include <diegos/poll.h>
#include <diegos/interrupts.h>
#include <string.h>
#include <stdio.h>
#include <libs/cbuffers.h>

#include "i8042_private.h"
#include "i8042.h"

#define KBD_BUF_SIZE    (64)
#define MSE_BUF_SIZE    (64*3)

static char kbd_rx_buf[KBD_BUF_SIZE];
static char mse_rx_buf[MSE_BUF_SIZE];
static struct cbuffer kbd_rx_cbuf;
static struct cbuffer mse_rx_cbuf;

static volatile unsigned flags_kbd = DRV_IS_CHAR | DRV_READ_BLOCK;
static volatile unsigned flags_mse = DRV_IS_CHAR | DRV_READ_BLOCK;
static wait_queue_t wq_kbd;
static wait_queue_t wq_mse;

static uint8_t readmask = OBF;

static inline void kbd_wait (unsigned loops)
{
    while (loops--)
    {
        if ((in_byte(i8042_CMD_PORT) & IBF) == 0)
        {
            break;
        }
    }
}

static inline void send_command (uint8_t cmd)
{
    kbd_wait(1000);
    out_byte(i8042_CMD_PORT, cmd);
}

static BOOL kbd_interrupt (void)
{
    uint8_t readval;

    send_command(DISABLE_PORT1);
    kbd_wait(1000);

    /* check if we are waiting for keyboard or mouse data */
    readval = in_byte(i8042_CMD_PORT);
    if ((readval & readmask) != OBF)
    {
        send_command(ENABLE_PORT1);
        return (TRUE);
    }

    /* If the queue is full, drop it !!! */
    if (cbuffer_free_space(&kbd_rx_cbuf))
    {
        kbd_rx_buf[kbd_rx_cbuf.tail] = in_byte(i8042_DATA_PORT);
        cbuffer_add(&kbd_rx_cbuf);
    }

    (void)thread_io_resume(&wq_kbd);

    return (TRUE);
}

static int kbd_init(unsigned unitno)
{
    /*
     * There can't be more than one instance !!!
     */
    if (unitno) {
        return (ENXIO);
    }

    /* Start detecting the devices */

    flags_kbd |= (DRV_READ_BLOCK);

    if (EOK != add_int_cb(kbd_interrupt, KEYBOARD_IRQ)) {
        return (EGENERIC);
    }

    cbuffer_init(&kbd_rx_cbuf, KBD_BUF_SIZE, 1);

    return (EOK);
}

static int kbd_start(unsigned unitno)
{
    /*
     * Enable interrupts
     */
    if (unitno) {
        return (EINVAL);
    }

    if (flags_kbd & DRV_STATUS_RUN) {
        return (EOK);
    }

    if (EOK != thread_io_wait_init(&wq_kbd)) {
        return (EPERM);
    }

    flags_kbd &= ~DRV_STATUS_STOP;
    flags_kbd |= DRV_STATUS_RUN;

    enable_int(KEYBOARD_IRQ);

    return (EOK);
}

static int kbd_stop(unsigned unitno)
{
    /*
     * Disable all interrupts
     */
    if (unitno) {
        return (EINVAL);
    }

    if (flags_kbd & DRV_STATUS_STOP) {
        return (EOK);
    }

    flags_kbd &= ~DRV_STATUS_RUN;
    flags_kbd |= DRV_STATUS_STOP;

    disable_int(KEYBOARD_IRQ);

    return (EOK);
}

static int kbd_done(unsigned unitno)
{
    if (unitno) {
        return (ENXIO);
    }

    flags_kbd = DRV_STATUS_DONE;
    return (EOK);
}

static int kbd_read (void *buf, unsigned bytes, unsigned unitno)
{
    return (ENXIO);
}

static unsigned kbd_status (unsigned unitno)
{
    if (!unitno) {
        return (flags_kbd);
    }

    return (0);
}

static short kbd_poll (poll_table_t *table)
{
    short ret = 0;

    if (!cbuffer_is_empty(&kbd_rx_cbuf)) {
        ret |= (POLLIN | POLLRDNORM);
    }

    poll_wait(&wq_kbd, table);

    return (ret);
}

char_driver_t i8042_kbd_drv = {
	.cmn = {
		.name = "kbd",
		.init_fn = kbd_init,
		.start_fn = kbd_start,
		.stop_fn = kbd_stop,
		.done_fn = kbd_done,
		.ioctrl_fn = NULL,
		.status_fn = kbd_status,
		.poll_fn = kbd_poll
	},
	.write_fn = NULL,
    .read_fn = kbd_read,
    .write_multi_fn = NULL,
    .read_multi_fn = NULL    
};

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

static BOOL mse_interrupt (void)
{
    uint8_t readval;

    /* check if we are waiting for keyboard or mouse data */
    readval = in_byte(i8042_CMD_PORT);
    if ((readval & readmask) != readmask)
    {
        return (TRUE);
    }

    /* If the queue is full, drop it !!! */
    if (cbuffer_free_space(&mse_rx_cbuf))
    {
        mse_rx_buf[mse_rx_cbuf.tail] = in_byte(i8042_DATA_PORT);
        cbuffer_add(&mse_rx_cbuf);
    }

    if ((cbuffer_in_use(&mse_rx_cbuf) % 3) == 0)
    {
        (void)thread_io_resume(&wq_mse);
    }

    return (TRUE);
}

static int mse_init (unsigned unitno)
{
    uint16_t *BIOS_area = (uint16_t *)0x410;

    /*
     * There can't be more than one instance !!!
     */
    if (unitno) {
        return (ENXIO);
    }

    /* Start detecting the device */
    /* Location 0x410 of BIOS area carries lots of status flags */
    if ((*BIOS_area & 0x4) == 0)
    {
        /* No mouse */
        return (ENXIO);
    }

    readmask |= TO;

    flags_mse |= (DRV_READ_BLOCK);

    if (EOK != add_int_cb(mse_interrupt, PS2MOUSE_IRQ)) {
        return (EGENERIC);
    }

    cbuffer_init(&mse_rx_cbuf, MSE_BUF_SIZE, 1);

    return (EOK);
}

static int mse_start (unsigned unitno)
{
    /*
     * Enable interrupts
     */
    if (unitno) {
        return (EINVAL);
    }

    if (flags_mse & DRV_STATUS_RUN) {
        return (EOK);
    }

    if (EOK != thread_io_wait_init(&wq_mse)) {
        return (EPERM);
    }

    flags_mse &= ~DRV_STATUS_STOP;
    flags_mse |= DRV_STATUS_RUN;

    enable_int(PS2MOUSE_IRQ);

    return (EOK);
}

static int mse_stop (unsigned unitno)
{
    /*
     * Disable all interrupts
     */
    if (unitno) {
        return (EINVAL);
    }

    if (flags_mse & DRV_STATUS_STOP) {
        return (EOK);
    }

    flags_mse &= ~DRV_STATUS_RUN;
    flags_mse |= DRV_STATUS_STOP;

    disable_int(PS2MOUSE_IRQ);

    return (EOK);
}

static int mse_done(unsigned unitno)
{
    if (unitno) {
        return (ENXIO);
    }

    flags_mse = DRV_STATUS_DONE;
    return (EOK);
}

static int mse_read (void *buf, unsigned bytes, unsigned unitno)
{
    return (ENXIO);
}

static unsigned mse_status (unsigned unitno)
{
    if (!unitno) {
        return (flags_mse);
    }

    return (0);
}

static short mse_poll (poll_table_t *table)
{
    short ret = 0;

    if (!cbuffer_is_empty(&mse_rx_cbuf)) {
        ret |= (POLLIN | POLLRDNORM);
    }

    poll_wait(&wq_mse, table);

    return (ret);
}

char_driver_t i8042_mouse_drv = {
	.cmn = {
		.name      = "mouse",
		.init_fn   = mse_init,
		.start_fn  = mse_start,
		.stop_fn   = mse_stop,
		.done_fn   = mse_done,
		.ioctrl_fn = NULL,
		.status_fn = mse_status,
		.poll_fn   = mse_poll
	},
    .write_fn       = NULL,
    .read_fn        = mse_read,
    .write_multi_fn = NULL,
    .read_multi_fn  = NULL    
};
