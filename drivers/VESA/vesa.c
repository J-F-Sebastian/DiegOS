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

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include <ia32/ports.h>
#include <pc/int86.h>
#include <diegos/devices.h>
#include <errno.h>
#include <endian.h>

#include "vesa.h"
#include "vesa_private.h"
#include "vga_fonts.h"

/* Frame buffer pointers */
static volatile uint8_t *frame_buffer = NULL;
static volatile uint32_t *frame_buffer32 = NULL;
/* Font pointer */
//static const unsigned char *g_font = NULL;
static struct VBEInfoBlock vesa_info;
static struct ModeInfoBlock vesa_gmode;
static uint16_t *vesa_mode_list = NULL;
static const uint8_t *g_font = NULL;
static unsigned W,H,max_offset;

static inline unsigned compute_offset (unsigned x, unsigned y)
{
	return y*vesa_gmode.LinBytesPerScanLine+x;
}

static inline unsigned compute_len (int a, int b)
{
	if (a > b) return (unsigned)(a - b + 1);
	else return (b - a + 1);
}

static int vesa_init (unsigned unitno)
{
    regs16_t regs;
    uint16_t *ptr;
    unsigned modecount = 0;

    if (unitno)
    {
        return ENXIO;
    }

	strncpy(vesa_info.signature, "VBE2", sizeof(vesa_info.signature));    
    regs.ax = 0x4F00;
    prot_to_seg_ofs(&vesa_info, &regs.es, &regs.di);    
    int86(0x10, &regs);

    if (0x004F != regs.ax) 
    {
		return ENXIO;
    }
            
    kdrvprintf("VESA Version %d.%d Video memory %u MBytes\n",               
               vesa_info.version >> 8,
               vesa_info.version & 0xFF,
               vesa_info.videomem64k >> 4);
    kdrvprintf("OEM: %s - Vendor: %s - Product: %s\n",
               real_to_prot(vesa_info.OEMnameptr),
               real_to_prot(vesa_info.vendornameptr),
	           real_to_prot(vesa_info.productnameptr));
	kdrvprintf("Capabilities: %x\n", vesa_info.capabilities);
	
	if ((vesa_info.version >> 8) < 3) return ENOTSUP;           
	ptr = real_to_prot(vesa_info.VESAmodesptr);
    while (*ptr++ != 0xFFFFU)
    {
		modecount++;		
	}
	
	if (modecount)
	{
		modecount++;
		vesa_mode_list = malloc(modecount*sizeof(uint16_t));
		if (!vesa_mode_list) return ENXIO;
		ptr = real_to_prot(vesa_info.VESAmodesptr);
        memcpy(vesa_mode_list, ptr, modecount*sizeof(uint16_t));
	}
    return (EOK);
}

static void put_pixel (point_t pix, unsigned c)
{
    unsigned off = compute_offset(pix.x, pix.y);
     
    if (off < max_offset) frame_buffer[off] = (uint8_t)c;
}

static void horiz_line (point_t a, unsigned len, unsigned c)
{
    unsigned offset;
        
    if (!len) return;

    offset = compute_offset(a.x, a.y);
    
    c &= 0xFF;
    c |= c << 8;
    c |= c << 16;
         
    while ((offset & 3) && len--)
    {
        frame_buffer[offset++] = (uint8_t)c;
	}
	offset >>= 2;
    while (len > 4) 
    {
		frame_buffer32[offset] = c;
		len -= 4;
		offset++;
	}
	offset <<= 2;
	while (len--)
	{
		frame_buffer[offset++] = (uint8_t)c;
	}
}

static void vert_line (point_t a, unsigned len, unsigned c)
{
    unsigned offset;
    
    if (!len) return;

    offset = compute_offset(a.x, a.y);
            
    while (len--) 
    {
		frame_buffer[offset] = (uint8_t)c;    
		offset += vesa_gmode.LinBytesPerScanLine;
	}
}

static void line (point_t a, point_t b, unsigned width, unsigned color)
{		
    if (a.y == b.y) 
    {	
        horiz_line((a.x > b.x) ? b : a, compute_len(a.x, b.x), color);
    } 
    else if (a.x == b.x) 
    {
        vert_line((a.y > b.y) ? b : a, compute_len(a.y, b.y), color);
    }
}

static void rect (point_t a, point_t b, unsigned wide, unsigned color)
{    
    point_t c,d;
    unsigned width = compute_len(a.x, b.x);
    unsigned height = compute_len(a.y, b.y);

    if (width < 2*wide) return;
    if (height < 2*wide) return;
    if (!wide) return;

	/*
	 * a --------------- c
	 * |				 | 
	 * |                 |
	 * d --------------- b
	 */ 
    c.y = a.y;
    c.x = b.x;
    d.y = b.y;
    d.x = a.x;

	while (wide--)
	{
		horiz_line(a, width, color);
        horiz_line(d, width, color);
        vert_line(a, height, color);
        vert_line(c, height, color);
        width -= 2;
        height -= 2;
        a.x++;
        a.y++;
        d.x++;
        d.y--; 
        c.x--;
        c.y++;               
	}   
}

static void fillrect (point_t a, point_t b, unsigned c)
{
    unsigned width = compute_len(a.x, b.x);
    unsigned height = compute_len(a.y, b.y);

    while (height--) {
        horiz_line(a, width, c);
        a.y++;
    }
}

/*
 * It is implicitely assumed that fonts are rendered real-time as well as H,
 * the font width, has a fixed value of 8
 * */
static void vesa_write_text (point_t a, unsigned c, const char *str)
{
	const unsigned char *fontptr;
	volatile uint8_t *fb = frame_buffer;
	unsigned loop, scan, i = 0;	
	unsigned char cursor;

    if (!str || !str[0]) return;    
    
    while (str[i]) 
    {
		fb = frame_buffer + compute_offset(a.x, a.y);
		fontptr = g_font + (unsigned)str[i]*H;
		for (scan = 0; scan < H; scan++, fb += vesa_gmode.XResolution)
		{		
			cursor = 1;
			loop = W;
			while (cursor)
			{
				loop--;
				if (cursor & fontptr[scan]) fb[loop] = (uint8_t)c;
				cursor += cursor;			
			}
		}
		i++;
		a.x += W;
	}
}

static void vesa_write_bgtext (point_t a,
                               unsigned bg,
                               unsigned c,
                               const char *str)
{
	const unsigned char *fontptr;
	volatile uint8_t *fb = frame_buffer;
	unsigned loop, scan, i = 0;	
	unsigned char cursor;
		
    if (!str || !str[0]) return;
        
    while (str[i]) 
    {
		fb = frame_buffer + compute_offset(a.x, a.y);
		fontptr = g_font + (unsigned)str[i]*H;
		for (scan = 0; scan < H; scan++, fb += vesa_gmode.XResolution)
		{		
			cursor = 1;
			loop = W;
			while (cursor)
			{
				loop--;
				if (cursor & fontptr[scan]) fb[loop] = (uint8_t)c;
				else fb[loop] = (uint8_t)bg;				
				cursor += cursor;			
			}						
		}
		i++;
		a.x += W;
	}
}

static void load_palette (unsigned index, uint8_t rgb[])
{   
#if 0		
	regs16_t regs; 
	uint8_t *ptr = (uint8_t *)real_buffer();
	
	if (index > 255) return;
	

	regs.ax = 0x4F09;
	regs.bx = 2;
	regs.cx = 1;
	regs.dx = (uint16_t)index;
	ptr[0] = rgb[0];
	ptr[1] = rgb[1];
	ptr[2] = rgb[2];
	ptr[3] = 0;	
    prot_to_seg_ofs(ptr, &regs.es, &regs.di);    
    int86(0x10, &regs);    
#else
	out_byte(0x3c8, index+8);
    out_byte(0x3c9,rgb[0]);
    out_byte(0x3c9,rgb[1]);
    out_byte(0x3c9,rgb[2]);
#endif   
}

static void load_all_palette (uint8_t rgb[])
{
	regs16_t regs; 
	uint8_t *ptr = (uint8_t *)real_buffer();
		
	regs.ax = 0x4F09;
	regs.bx = 0;
	regs.cx = 256;
	regs.dx = 0;
	memcpy(ptr, rgb, 256*3);
    prot_to_seg_ofs(ptr, &regs.es, &regs.di);    
    int86(0x10, &regs);    
}

static void store_palette (unsigned index, uint8_t rgb[])
{
}

static void store_all_palette (uint8_t rgb[])
{
}

static void draw_sprite(point_t a,
                        uint8_t sprite[],
                        unsigned W,
                        unsigned H,
                        unsigned color)
{
}

static void show (void)
{
    /* 
     * No op
     */
}

static void set_font (uint8_t fonts[], unsigned w, unsigned h)
{
    /* Right now we don't use fonts[], next dev round please :-) */  
    if ((8 == w) && (8 == h)) {
        g_font = vga_get_font(VGA_FONTS_CP437_8x8);
        W = w;
        H = h;
    }
    else if ((8 == w) && (16 == h)) {
        g_font = vga_get_font(VGA_FONTS_CP437_8x16);
        W = w;
        H = h;
    }
    else {
        g_font = vga_get_font(VGA_FONTS_CP437_8x16);
        W = 8;
        H = 16;
    }  
}

static unsigned vesa_status (unsigned unitno)
{
    return (DRV_STATUS_RUN | DRV_IS_GFX_UI);
}

static int set_resolution (unsigned W, unsigned H, unsigned depth, int loose)
{	
	uint16_t *cursor = vesa_mode_list;
	uint16_t mode;
	regs16_t regs;
	
	if (depth != 8) return ENXIO;
		
	while (*cursor != 0xFFFF)
	{
		mode = *cursor++ & 0x1FF;
		mode |= (1 << 14);
		regs.ax = 0x4F01;
		/* Request for linear frame buffer support */
		regs.cx = mode;
		prot_to_seg_ofs(&vesa_gmode, &regs.es, &regs.di);    
        int86(0x10, &regs);
        if (regs.ax != 0x004F) continue;
        if (0x81 != (vesa_gmode.ModeAttributes & 0x81)) continue;
        if (vesa_gmode.BitsPerPixel != depth) continue;
        if (vesa_gmode.MemoryModel != 0x04) continue;
        if ((vesa_gmode.XResolution != W) || 
            (vesa_gmode.YResolution != H)) continue;
        /* 
         * OK go for it! 
         */     
        regs.ax = 0x4F02;		
		regs.bx = mode;			 
        int86(0x10, &regs);            
        if (regs.ax != 0x004F) return ENOTSUP;
        frame_buffer = (uint8_t *)vesa_gmode.PhysAddress;
        frame_buffer32 = (uint32_t *)vesa_gmode.PhysAddress;
        max_offset = vesa_gmode.LinBytesPerScanLine*vesa_gmode.YResolution;
        /*
         * DAC can be set to 8 bit per pixel
         */
      //  if (vesa_info.capabilities & 1)
      //  {
		//	regs.ax = 0x4F08;
		//	regs.bx = 0x0800;
		//	int86(0x10, &regs);
		//	assert((regs.bx & 0xFF00) == 0x0800);
		//}
        return EOK;            
	}		
	return ENOTSUP;
}

/*
 * This has been placed at the end so we do not need to
 * forward static declarations etc. etc.
 * Moreover, it is an initialization, no more than this...
 */
grafics_driver_t vesa_drv = {
    .cmn = {
        .name = "vesa",
        .init_fn = vesa_init,
        .start_fn = driver_def_ok,
        .stop_fn = driver_def_ok,
        .done_fn = driver_def_ok,
        .status_fn = vesa_status,
        .ioctrl_fn = NULL,
        .poll_fn = NULL
    },
    .put_pixel_fn = put_pixel,
    .line_fn = line,
    .rectangle_fn = rect,
    .filled_rectangle_fn = fillrect,
    .write_text_fn = vesa_write_text,
    .write_text_bg_fn = vesa_write_bgtext,
    .load_palette_fn = load_palette,
    .load_all_palette_fn = load_all_palette,
    .store_palette_fn = store_palette,
    .store_all_palette_fn = store_all_palette,
    .draw_sprite_fn = draw_sprite,
    .show_fn = show,
    .set_font_fn = set_font,
    .set_res_fn = set_resolution
};

