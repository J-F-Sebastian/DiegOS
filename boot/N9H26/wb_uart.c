/****************************************************************************
 * @file     wb_uart.c
 * @version  V3.00
 * @brief    N9H26 series SYS driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/****************************************************************************
* FILENAME
*   wb_uart.c
*
* VERSION
*   1.0
*
* DESCRIPTION
*   The UART related function of Nuvoton ARM9 MCU
*
* HISTORY
*   2008-06-25  Ver 1.0 draft by Min-Nan Cheng
*
* REMARK
*   None
 **************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <types_common.h>
#include "wblib.h"

PFN_SYS_UART_CALLBACK(pfnUartIntHandlerTable)[2][2] = { 0 };

//#define vaStart(list, param) list = (char *)((int)&param + sizeof(param))
//#define vaArg(list, type) ((type *)(list += sizeof(type)))[-1]

/* Global variables */
BOOL volatile _sys_bIsUARTInitial = FALSE;
BOOL volatile _sys_bIsUseUARTInt = TRUE;
uint32_t _sys_uUARTClockRate = EXTERNAL_CRYSTAL_CLOCK;
//uint32_t
// UART_BA = UART0_BA;

#define RX_ARRAY_NUM 100
uint8_t uart_rx[RX_ARRAY_NUM] = { 0 };

void _PutChar_f(uint8_t ucCh);
uint32_t volatile rx_cnt = 0;

#define sysTxBufReadNextOne()   (((_sys_uUartTxHead+1)==UART_BUFFSIZE)? (uint32_t)NULL: _sys_uUartTxHead+1)
#define sysTxBufWriteNextOne()  (((_sys_uUartTxTail+1)==UART_BUFFSIZE)? (uint32_t)NULL: _sys_uUartTxTail+1)
#define UART_BUFFSIZE   256
uint8_t _sys_ucUartTxBuf[UART_BUFFSIZE];
uint32_t volatile _sys_uUartTxHead, _sys_uUartTxTail;
void *_sys_pvOldUartVect;
uint32_t u32UartPort = 0x100;

int32_t i32UsedPort = 0;
void sysUartPort(uint32_t u32Port)
{
	u32UartPort = (u32Port & 0x1) * 0x100;
	//Nornal Speed UART
	outp32(REG_CLKDIV3, (inp32(REG_CLKDIV3) & (~(UART1_N1 | UART1_S | UART1_N0))));
	outp32(REG_GPAFUN1, (inp32(REG_GPAFUN1) & ~(MF_GPA10 | MF_GPA11)) | 0x3300);
	outp32(REG_APBCLK, inp32(REG_APBCLK) | UART1_CKE);

	i32UsedPort = u32Port;
}

void sysUartInstallcallback(uint32_t u32IntType, PFN_SYS_UART_CALLBACK pfnCallback)
{
	if (u32IntType > 1)
		return;
	if (u32IntType == 0) {
		pfnUartIntHandlerTable[i32UsedPort][0] = (PFN_SYS_UART_CALLBACK) (pfnCallback);
	} else if (u32IntType == 1) {
		pfnUartIntHandlerTable[i32UsedPort][1] = (PFN_SYS_UART_CALLBACK) (pfnCallback);
	}
}

void sysUartISR()
{

	uint32_t volatile regIIR, i;
	uint32_t volatile regIER, u32EnableInt;
	//sysprintf("UART ISR\n");
	regIIR = inpb(REG_UART_ISR + u32UartPort);
	regIER = inpb(REG_UART_IER + u32UartPort);
	u32EnableInt = regIER & regIIR;
	if (u32EnableInt & THRE_IF) {
		// buffer empty
		if (_sys_uUartTxHead == _sys_uUartTxTail) {
			//Disable interrupt if no any request!
			outpb((REG_UART_IER + u32UartPort),
			      inp32(REG_UART_IER + u32UartPort) & (~THRE_IEN));
		} else {
			//Transmit data
			for (i = 0; i < 8; i++) {
				outpb(REG_UART_THR + u32UartPort,
				      _sys_ucUartTxBuf[_sys_uUartTxHead]);
				_sys_uUartTxHead = sysTxBufReadNextOne();
				if (_sys_uUartTxHead == _sys_uUartTxTail)	// buffer empty
					break;
			}
		}
	}
	if (u32EnableInt & RDA_IF) {
		uint32_t u32Count;
		u32Count = (inpw(REG_UART_FSR + u32UartPort) & Rx_Pointer) >> 8;
#if 0
		sysprintf("\nRx data available %d bytes\n", u32Count);
#endif
		for (i = 0; i < u32Count; i++) {
			if (rx_cnt == RX_ARRAY_NUM) {
				rx_cnt = 0;
			}
			uart_rx[rx_cnt] = (inpb(REG_UART_RBR + u32UartPort));
#if 0
			//debug _PutChar_f(uart_rx[rx_cnt + i]);
#endif
			rx_cnt++;
		}
#if 0
		sysprintf("\nRx array index %d\n", rx_cnt);
#else
		//callback to uplayer(bufptr, len);

		if (pfnUartIntHandlerTable[i32UsedPort][0] != 0)
			pfnUartIntHandlerTable[i32UsedPort][0] (&(uart_rx[0]), u32Count);
		rx_cnt = 0;
#endif

	}
	if (u32EnableInt & Tout_IF)	//½ÓÊÕ×´Ì¬·ÖÎö
	{
		uint32_t u32Count;
		u32Count = (inpw(REG_UART_FSR + u32UartPort) & Rx_Pointer) >> 8;
#if 0
		sysprintf("\nRx data time out %d bytes\n", u32Count);
#endif
		for (i = 0; i < u32Count; i++) {
			if (rx_cnt == RX_ARRAY_NUM) {
				rx_cnt = 0;
			}
			uart_rx[rx_cnt] = (inpb(REG_UART_RBR + u32UartPort));
#if 0
			_PutChar_f(uart_rx[rx_cnt + i]);
#endif
			rx_cnt++;

		}
#if 0
		sysprintf("\nRx array index %d\n", rx_cnt);
#else
		//callback to uplayer(bufptr, len);
		if (pfnUartIntHandlerTable[i32UsedPort][1] != 0)
			pfnUartIntHandlerTable[i32UsedPort][1] (&(uart_rx[0]), u32Count);
		rx_cnt = 0;
#endif
	}
	//sysprintf("\n");
}

static void sysSetBaudRate(uint32_t uBaudRate)
{
	uint32_t _mBaudValue;

	/* First, compute the baudrate divisor. */
	// mode 3
	_mBaudValue = (_sys_uUARTClockRate / uBaudRate) - 2;
	outpw(REG_UART_BAUD + u32UartPort, ((0x30 << 24) | _mBaudValue));
}

void sysUartEnableInt(int32_t eIntType)
{
	if (eIntType == UART_INT_RDA)
		outp32(REG_UART_IER + u32UartPort, inp32(REG_UART_IER + u32UartPort) | RDA_IEN);
	else if (eIntType == UART_INT_RDTO)
		outp32(REG_UART_IER + u32UartPort,
		       inp32(REG_UART_IER + u32UartPort) | RTO_IEN | Time_out_EN);
	else if (eIntType == UART_INT_NONE)
		outp32(REG_UART_IER + u32UartPort, 0x0);
}

int32_t sysInitializeUART(WB_UART_T * uart)
{
	/* Enable UART multi-function pins */
	//outpw(REG_PINFUN, inpw(REG_PINFUN) | 0x80);
	//outpw(REG_GPAFUN, inpw(REG_GPAFUN) | 0x00F00000); //Normal UART pin function
	static BOOL bIsResetFIFO = FALSE;
	int32_t i32Ret = 0;

//    if(uart->uart_no == 0)
	sysUartPort(uart->uart_no);
//  else
//      sysUartPort(uart->uart_no);
	/* Reset the TX/RX FIFOs */
	if (bIsResetFIFO == FALSE) {
		outpw(REG_UART_FCR + u32UartPort, 0x07);
		bIsResetFIFO = TRUE;
	}
	/* Setup reference clock */
	_sys_uUARTClockRate = uart->uiFreq;

	/* Setup baud rate */
	sysSetBaudRate(uart->uiBaudrate);

	/* Set the modem control register. Set DTR, RTS to output to LOW,
	   and set INT output pin to normal operating mode */
	//outpb(UART_MCR, (WB_DTR_Low | WB_RTS_Low | WB_MODEM_En));

	/* Setup parity, data bits, and stop bits */
	outpw(REG_UART_LCR + u32UartPort, (uart->uiParity | uart->uiDataBits | uart->uiStopBits));

	/* Timeout if more than ??? bits xfer time */
	outpw(REG_UART_TOR + u32UartPort, 0x80 + 0x20);

	/* Setup Fifo trigger level and enable FIFO */
	outpw(REG_UART_FCR + u32UartPort, (uart->uiRxTriggerLevel << 4) | 0x01);

	/* Enable HUART interrupt Only (Receive Data Available Interrupt & RX Time out Interrupt) */

	/* Timeout if more than ??? bits xfer time */
	outpw(REG_UART_TOR + u32UartPort, 0x7F);

	// hook UART interrupt service routine
//    if (u32UartPort)
	{
		//==1 NORMAL UART
		_sys_uUartTxHead = _sys_uUartTxTail = (uint32_t) NULL;
		_sys_pvOldUartVect = sysInstallISR(IRQ_LEVEL_1, IRQ_UART, (void *)sysUartISR);
		sysEnableInterrupt(IRQ_UART);
	}
/*
    else
    {
        //==0 High SPEED
        _sys_uUartTxHead = _sys_uUartTxTail = (uint32_t)NULL;
        _sys_pvOldUartVect = sysInstallISR(IRQ_LEVEL_1, IRQ_HUART, (PVOID)sysUartISR);
        sysEnableInterrupt(IRQ_HUART);
    }
*/
	_sys_bIsUARTInitial = TRUE;

	i32Ret = Successful;

//exit_sysInitializeUART:

	return i32Ret;
}

void _PutChar_f(uint8_t ucCh)
{
	if (_sys_bIsUseUARTInt == TRUE) {
		while (sysTxBufWriteNextOne() == _sys_uUartTxHead) ;	// buffer full

		_sys_ucUartTxBuf[_sys_uUartTxTail] = ucCh;
		_sys_uUartTxTail = sysTxBufWriteNextOne();

		if (ucCh == '\n') {
			while (sysTxBufWriteNextOne() == _sys_uUartTxHead) ;	// buffer full

			_sys_ucUartTxBuf[_sys_uUartTxTail] = '\r';
			_sys_uUartTxTail = sysTxBufWriteNextOne();
		}

		if (!(inpw(REG_UART_IER + u32UartPort) & 0x02))
			outpw(REG_UART_IER + u32UartPort, 0x02);
	} else {
		/* Wait until the transmitter buffer is empty */
		while (!(inpw(REG_UART_FSR + u32UartPort) & 0x400000)) ;
		/* Transmit the character */
		outpb(REG_UART_THR + u32UartPort, ucCh);

		if (ucCh == '\n') {
			/* Wait until the transmitter buffer is empty */
			while (!(inpw(REG_UART_FSR + u32UartPort) & 0x400000)) ;
			outpb(REG_UART_THR + u32UartPort, '\r');
		}
	}
}

void sysPutString(char *string)
{
	while (*string != '\0') {
		_PutChar_f(*string);
		string++;
	}
}

static void sysPutRepChar(char c, int count)
{
	while (count--)
		_PutChar_f(c);
}

static void sysPutStringReverse(char *s, int index)
{
	while ((index--) > 0)
		_PutChar_f(s[index]);
}

static void sysPutNumber(int value, int radix, int width, char fill)
{
	char buffer[40];
	int bi = 0;
	uint32_t uvalue;
	uint16_t digit;
	uint16_t left = FALSE;
	uint16_t negative = FALSE;

	if (fill == 0)
		fill = ' ';

	if (width < 0) {
		width = -width;
		left = TRUE;
	}

	if (width < 0 || width > 80)
		width = 0;

	if (radix < 0) {
		radix = -radix;
		if (value < 0) {
			negative = TRUE;
			value = -value;
		}
	}

	uvalue = value;

	do {
		if (radix != 16) {
			digit = uvalue % radix;
			uvalue = uvalue / radix;
		} else {
			digit = uvalue & 0xf;
			uvalue = uvalue >> 4;
		}
		buffer[bi] = digit + ((digit <= 9) ? '0' : ('A' - 10));
		bi++;

		if (uvalue != 0) {
			if ((radix == 10)
			    && ((bi == 3) || (bi == 7) || (bi == 11) | (bi == 15))) {
				buffer[bi++] = ',';
			}
		}
	}
	while (uvalue != 0);

	if (negative) {
		buffer[bi] = '-';
		bi += 1;
	}

	if (width <= bi)
		sysPutStringReverse(buffer, bi);
	else {
		width -= bi;
		if (!left)
			sysPutRepChar(fill, width);
		sysPutStringReverse(buffer, bi);
		if (left)
			sysPutRepChar(fill, width);
	}
}

static char *FormatItem(char *f, int a)
{
	char c;
	unsigned fieldwidth = 0;
	int leftjust = FALSE;
	int radix = 0;
	char fill = ' ';

	if (*f == '0')
		fill = '0';

	while ((c = *f++) != 0) {
		if (c >= '0' && c <= '9') {
			fieldwidth = (fieldwidth * 10) + (c - '0');
		} else
			switch (c) {
			case '\000':
				return (--f);
			case '%':
				_PutChar_f('%');
				return (f);
			case '-':
				leftjust = TRUE;
				break;
			case 'c':
				{
					if (leftjust)
						_PutChar_f(a & 0x7f);

					if (fieldwidth > 0)
						sysPutRepChar(fill, fieldwidth - 1);

					if (!leftjust)
						_PutChar_f(a & 0x7f);
					return (f);
				}
			case 's':
				{
					if (leftjust)
						sysPutString((char *)a);

					if (fieldwidth > strlen((char *)a))
						sysPutRepChar(fill, fieldwidth - strlen((char *)a));

					if (!leftjust)
						sysPutString((char *)a);
					return (f);
				}
			case 'd':
			case 'i':
				radix = -10;
				break;
			case 'u':
				radix = 10;
				break;
			case 'x':
				radix = 16;
				break;
			case 'X':
				radix = 16;
				break;
			case 'o':
				radix = 8;
				break;
			default:
				radix = 3;
				break;	/* unknown switch! */
			}
		if (radix)
			break;
	}

	if (leftjust)
		fieldwidth = -fieldwidth;

	sysPutNumber(a, radix, fieldwidth, fill);

	return (f);
}

void sysPrintf(char *pcStr, ...)
{
	WB_UART_T uart;
	va_list argP;

	_sys_bIsUseUARTInt = TRUE;
	if (!_sys_bIsUARTInitial) {
		sysUartPort(1);
		uart.uart_no = WB_UART_1;
		uart.uiFreq = sysGetExternalClock();
		uart.uiBaudrate = 115200;
		uart.uiDataBits = WB_DATA_BITS_8;
		uart.uiStopBits = WB_STOP_BITS_1;
		uart.uiParity = WB_PARITY_NONE;
		uart.uiRxTriggerLevel = LEVEL_1_BYTE;
		sysInitializeUART(&uart);
	}

	va_start(argP, pcStr);	/* point at the end of the format string */
	while (*pcStr) {
		/* this works because args are all ints */
		if (*pcStr == '%')
			pcStr = FormatItem(pcStr + 1, va_arg(argP, int));
		else
			_PutChar_f(*pcStr++);
	}
}

void sysprintf(char *pcStr, ...)
{
	WB_UART_T uart;
	va_list argP;

	_sys_bIsUseUARTInt = FALSE;
	if (!_sys_bIsUARTInitial) {
		//Default use external clock 12MHz as source clock.
		sysUartPort(1);
		uart.uart_no = WB_UART_1;
		uart.uiFreq = sysGetExternalClock();
		uart.uiBaudrate = 115200;
		uart.uiDataBits = WB_DATA_BITS_8;
		uart.uiStopBits = WB_STOP_BITS_1;
		uart.uiParity = WB_PARITY_NONE;
		uart.uiRxTriggerLevel = LEVEL_1_BYTE;
		sysInitializeUART(&uart);
	}

	va_start(argP, pcStr);	/* point at the end of the format string */
	while (*pcStr) {
		/* this works because args are all ints */
		if (*pcStr == '%')
			pcStr = FormatItem(pcStr + 1, va_arg(argP, int));
		else
			_PutChar_f(*pcStr++);
	}
}

char sysGetChar()
{
	while (1) {
		if (inpw(REG_UART_ISR + u32UartPort) & 0x01)
			return (inpb(REG_UART_RBR + u32UartPort));
	}
}

void sysPutChar(char ucCh)
{
	/* Wait until the transmitter buffer is empty */
	while (!(inpw(REG_UART_FSR + u32UartPort) & 0x400000)) ;
	/* Transmit the character */
	outpb(REG_UART_THR + u32UartPort, ucCh);
}

void sysUartTransfer(int8_t * pu8buf, uint32_t u32Len)
{
	do {
		if ((inp32(REG_UART_FSR + u32UartPort) & Tx_Full) == 0) {
			outpb(REG_UART_THR + u32UartPort, *pu8buf++);
			u32Len = u32Len - 1;
		}
	}
	while (u32Len != 0);
}
