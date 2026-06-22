#include <errno.h>
#include <platform/BIOS.h>
#include <platform/int86.h>

int INT13_read_drive_param(const uint8_t drv,
			   uint8_t *drv_type, uint16_t *C, uint8_t *H, uint8_t *S)
{
	regs16_t regs;

	/* AH 0x08 */
	regs.ax = 0x0800;
	/* DL drive */
	regs.dx = drv;
	/* ES:DI set to 0000H:0000H to guard again BIOS bugs */
	regs.es = regs.di = 0;
	int86(0x13, &regs);

	/* If carry flag is set the an error was detected */
	if (regs.flags & 1) {
		return (EIO);
	}

	*drv_type = regs.bx & 0xFF;
	*S = regs.cx & 0x3F;
	*C = ((regs.cx & 0xC0) << 2) | (regs.cx >> 8);
	*H = regs.dx >> 8;

	return (EOK);
}
