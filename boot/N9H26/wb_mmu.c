/****************************************************************************
 * @file     wb_mmu.c
 * @version  V3.00
 * @brief    N9H26 series SYS driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *     wb_mmu.c
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     This file implement mmu functions.
 *
  * HISTORY
 *     2008-06-25  Ver 1.0 draft by Min-Nan Cheng
 *
 * REMARK
 *     None
 **************************************************************************/
#include <stdint.h>
#include <types_common.h>
#include "wblib.h"

#define  _CoarsePageSize    64	//MB

typedef struct _coarse_table {
	unsigned int page[256];
} _CTable;

unsigned int _mmuSectionTable[4096] __attribute__((aligned(0x4000)));

static BOOL _IsInitMMUTable = FALSE;

//extern INT32 sysGetSdramSizebyMB(void);
extern void sysSetupCP15(unsigned int);

void sysSetupCP15(unsigned int addr)
{
	register int reg1 = 1, reg0 = 0x40000000;
	reg0 = addr;
	__asm volatile ("MOV     %0, %1                \n"	// _mmuSectionTable
			"MCR     p15, #0, %0, c2, c0, #0  \n"	// write translation table base register c2
			"MOV     %0, #0x40000000   \n" "MCR     p15, #0, %0, c3, c0, #0  \n"	// domain access control register c3
			"MRC     p15, #0, %0, c1, c0, #0 \n"	// read control register c1
			"ORR     %0, %0, #0x1000 \n"	// enable I cache bit
			"ORR     %0, %0, #0x5     \n"	// enable D cache and MMU bits
			"MCR     p15, #0, %0, c1, c0, #0  \n"	// write control register c1
			::"r" (reg1), "r"(reg0):"memory");
}

int sysSetMMUMappingMethod(int mode)
{
	return 0;

}				/* end sysSetMMUMappingMethod */

int sysInitMMUTable(int cache_mode)
{
	int i;
	if (_IsInitMMUTable == FALSE) {
		for (i = 0; i < 64; i++) {
			_mmuSectionTable[i] = (i << 20) | 0xDFE;	/* Cacheable, Bufferable */
		}
		for (i = 64; i < 2048; i++) {
			_mmuSectionTable[i] = (i << 20) | 0xDF2;	/* Non-acheable, Non-Bufferable */
		}
		for (i = 2048; i < 2112; i++) {
			_mmuSectionTable[i] = ((i - 2048) << 20) | 0xDF2;	/* Non-acheable, Non-Bufferable */
		}
		for (i = 2112; i < 4096; i++) {
			_mmuSectionTable[i] = (i << 20) | 0xDF2;	/* Non-acheable, Non-Bufferable */
		}
		_IsInitMMUTable = TRUE;
	}

	//moved here by cmn [2007/01/27]
	//set CP15 registers
	sysSetupCP15((unsigned int)_mmuSectionTable);

	return 0;

}				/* end sysInitMMUTable */

extern char __heap_start__;
extern char __heap_end__;
unsigned char *HeapSize = 0;
unsigned char *_sbrk(int incr)
{
	//extern char _Heap_Begin; // Defined by the linker.
	//extern char _Heap_Limit; // Defined by the linker.

	static char *current_heap_end = 0;
	char *current_block_address;

	if (current_heap_end == 0) {
		current_heap_end = &__heap_start__;
	}

	current_block_address = current_heap_end;

	// Need to align heap to word boundary, else will get
	// hard faults on Cortex-M0. So we assume that heap starts on
	// word boundary, hence make sure we always add a multiple of
	// 4 to it.
	incr = (incr + 3) & (~3);	// align value to 4
	if (current_heap_end + incr > &__heap_end__) {
		// Some of the libstdc++-v3 tests rely upon detecting
		// out of memory errors, so do not abort here.
#if 0
		extern void abort(void);

		_write(1, "_sbrk: Heap and stack collision\n", 32);

		abort();
#else
		// Heap has overflowed
		// errno = ENOMEM;
		return (unsigned char *)-1;
#endif
	}

	current_heap_end += incr;

	return (unsigned char *)current_block_address;
}
