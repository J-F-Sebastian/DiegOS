;
; DiegOS Operating System source code
;
; Copyright (C) 2012 - 2020 Diego Gallizioli
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.
;
; This boot loader will work only on a High Density Diskette,
; formatted as 1.44 MByte disk (2880 sectors, 512 bytes each).
;
; DiegOS BPB boot sector
;
; Memory map at boot time and run time
;
;   +-----------------------+
;   |       0x00000000      |
;   |          ....         |
;   |          ....         |   Real Mode interrupt vectors (256*4 Bytes)
;   |          ....         |
;   +-----------------------+
;   |       0x00000400      |   BIOS Data Area (256 Bytes)
;   +-----------------------+
;   |       0x00000500      |   UNUSED (256 Bytes)
;   +-----------------------+
;   |       0x00000600      |
;   |          ....         |
;   |          ....         |
;   |          ....         |   IDT (256*8 Bytes)
;   |          ....         |
;   |          ....         |
;   |          ....         |
;   |          ....         |
;   +-----------------------+
;   |       0x00000E00      |   GDT (8*8 Bytes)
;   +-----------------------+
;   |       0x00000E40      |   IDTR, GDTR (2*8 Bytes)
;   +-----------------------+
;   |       0x00000E50      |
;   |          ....         |
;   |          ....         |
;   |          ....         |   FREE FOR 16-BIT STACK (25008 Bytes)
;   |          ....         |
;   |          ....         |
;   |          ....         |
;   +-----------------------+
;   |       0x00007000      |   16-BIT STACK START (GROWS TO LOWER ADDRESSES)
;   |          ....         |   UNUSED (3072 Bytes)
;   +-----------------------+
;   |       0x00007C00      |   BOOT CODE ENTRY point (16-BIT REAL MODE)
;   |          ....         |   (512 Bytes)
;   +-----------------------+
;   |       0x00007E00      |
;   |          ....         |
;   |          ....         |
;
;     ~~~~~~~~~~~~~~~~~~~~~     FREE FOR 32-BIT STACK (DiegOS init ONLY !!!)
;                               (29184 Bytes)
;   |          ....         |
;   |          ....         |
;   +-----------------------+
;   |       0x0000F000      |   32-BIT STACK START (GROWS TO LOWER ADDRESSES)
;   |          ....         |   UNUSED (4096 Bytes)
;   +-----------------------+
;   |       0x00010000      |   DiegOS ENTRY POINT (32-BIT PROTECTED MODE)
;   |          ....         |
;   |          ....         |
;     ~~~~~~~~~~~~~~~~~~~~~     FREE FOR 32-BIT CODE (575 KBytes)
;   |          ....         |
;   |          ....         |
;   +-----------------------+
;   |       0x0009FC00      |   Extended BIOS Data Area (1 KByte)
;   |          ....         |
;   +-----------------------+
;   |       0x000A0000      |
;   |          ....         |
;   |          ....         |
;     ~~~~~~~~~~~~~~~~~~~~~     VGA VIDEO MEMORY (128 KBytes)
;   |          ....         |
;   |          ....         |
;   +-----------------------+
;   |       0x000C0000      |
;   |          ....         |   VIDEO BIOS  (32 KBytes)
;   |          ....         |
;   |          ....         |
;   +-----------------------+
;   |       0x000C8000      |
;   |          ....         |   ROM AND MAPPED HARDWARE (160 KBytes)
;   |          ....         |
;   |          ....         |
;   +-----------------------+
;   |       0x000F0000      |
;   |          ....         |   BIOS    (64 KBytes)
;   |          ....         |
;   |          ....         |
;   +-----------------------+
;   |       0x00100000      |
;   |                       |
;     ~~~~~~~~~~~~~~~~~~~~~
;     ~~~~~~~~~~~~~~~~~~~~~     DiegOS HEAP, FREE MEMORY
;     ~~~~~~~~~~~~~~~~~~~~~
;

[ORG 0x7C00]
[BITS 16]

BYTESPERSEC	EQU 512
RAM_START	EQU 0x0600
STACK16_START	EQU 0x7000
STACK32_START	EQU 0x0000F000
DIEGOS_START	EQU 0x00010000

; Descriptors at beginning of RAM
IDT_DESC	EQU RAM_START
; IDT size is 8 bytes x 256 interrupts = 2048 bytes, 0x800
GDT_DESC	EQU IDT_DESC + 0x0800
; GDT entries: 3 descriptors for NULL, CODE, DATA
; FLAT model, max priority
; GDT has room for a total of 8 descriptors
IDTR_START	EQU GDT_DESC + 0x40 + 2
GDTR_START	EQU IDTR_START + 8

; equates for building GDT
LINEAR_CODE_SEL 	EQU 1*8
LINEAR_DATA_SEL		EQU 2*8
LINEAR_PROTO_CODE_LO	EQU 0x000000A0
LINEAR_PROTO_DATA_LO 	EQU 0x0000FFFF
LINEAR_PROTO_CODE_HI 	EQU 0x00C09B00
LINEAR_PROTO_DATA_HI 	EQU 0x00CF9200

; Protection Enable Bit in CR0, no paging
PE_BIT EQU 1
CD_BIT EQU 1<<30
NW_BIT EQU 1<<29

; BPB - BIOS Parameter Block
; Leave void, other tools must fill in proper data.
BPB:
RESB	62

BootLoader:
CLD
; Init DS,SS to have a temporary stack and data access
; Increment counters
XOR	AX, AX
MOV	DS, AX
MOV	SS, AX
MOV	SP, STACK16_START
MOV	BP, SP

MOV	SI, HelloString
CALL	PrintString

STI

;reset first disk drive
MOV	AH, 0x00
;MOV	DL, 0x00
INT	0x13
JC	Error

;CH = low eight bits of maximum cylinder number
;CL = maximum sector number (bits 5-0)
;high two bits of maximum cylinder number (bits 7-6)
;DH = maximum head number
;AH = 02h
;AL = number of sectors to read (must be nonzero)
;CH = low eight bits of cylinder number
;CL = sector number 1-63 (bits 0-5)
;high two bits of cylinder (bits 6-7, hard disk only)
;DH = head number
;DL = drive number (bit 7 set for hard disk)
;ES:BX -> data buffer

; set destination buffer
; NOTE: the buffer is 64KBytes limited !!!
MOV	BX, BPB 
MOV 	SI, WORD [BX + 0xE]
DEC	SI
MOV	AX, WORD DIEGOS_START>>4
MOV	ES, AX
MOV 	BP, AX

;1.44 MB Disk Format support 18 sectors per track,
; or 0x12. the first sector is used by boot code.
; Function 0x02 of INT 0x13, read data from disk
;AH	function 0x02
;AL	sectors to be read, first round not in loop
MOV     AX, 0x0201
;CH	starting cylinder/track
;CL	starting sector
MOV     CX, 0x0002
;DH	starting head
;DL	1st disk unit
MOV	DH, 0x00
XOR	BX, BX

readOneSector:

INT	0x13
JC	Error

SUB 	SI, 1
JZ  	StartDiegOS

PUSH	BX
CALL	PrintDot
POP	BX

ADD 	BX, BYTESPERSEC 
JNC 	NoNewSegment

ADD 	BP, WORD DIEGOS_START>>4
MOV 	ES, BP
NoNewSegment:

MOV	AX, 0x0201
INC 	CL
CMP 	CL, 0x12
JNA	readOneSector

MOV 	CL, 1
ADD 	CH, DH
XOR	DH, 0x01

JMP readOneSector

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

StartDiegOS:
MOV	SI, StartString
CALL	PrintString
CALL	StopMotor
;Clear interrupts
CLI

;Enable Gate A20
MOV	AX, 0x2401
INT	0x15
JNC	A20Enabled

CALL	A20Enable

A20Enabled:
;Detect free memory
;MOV     AX, 0xE801
;INT     0x15		; request upper memory size
;JC      Error
;CMP	AX, 0x0
;JNE    	useAX		; was the EAX result invalid?
;MOV     AX, CX
;MOV     BX, DX
;useAX:
; AX = number of contiguous Kb, 1M to 16M
; BX = contiguous 64Kb pages above 16M
;SHL     EAX, 10
;SHL     EBX, 16
;ADD     EAX, EBX
;MOV     DWORD [DIEGOS_VARS], 0x00100000
;MOV     DWORD [DIEGOS_VARS + 4], EAX

;Assign segments for proper kernel start
XOR	AX, AX
MOV	ES, AX

; IDT is initialized by kernel
; Setup GDT
; First GDT entry is NULL
MOV 	DWORD [GDT_DESC], 0
MOV 	DWORD [GDT_DESC + 4], 0

; Second GDT entry is for FLAT MEMORY CODE
MOV 	DWORD [GDT_DESC + 8], LINEAR_PROTO_CODE_LO
MOV 	DWORD [GDT_DESC + 12], LINEAR_PROTO_CODE_HI

; Third GDT entry is for FLAT MEMORY DATA
MOV 	DWORD [GDT_DESC + 16], DWORD LINEAR_PROTO_DATA_LO
MOV 	DWORD [GDT_DESC + 20], DWORD LINEAR_PROTO_DATA_HI

; Setup IDTR
MOV	WORD  [IDTR_START], (256 * 8) - 1
MOV 	DWORD [IDTR_START + 2], DWORD IDT_DESC


; Setup GDTR
MOV	WORD  [GDTR_START], (8 * 8) - 1
MOV 	DWORD [GDTR_START + 2], DWORD GDT_DESC

; execute a 32 bit LGDT
o32	LGDT [GDTR_START]

; enter protected mode
MOV 	EBX, CR0
OR	EBX, PE_BIT
AND     EBX, ~CD_BIT
AND	EBX, ~NW_BIT
MOV 	CR0, EBX

; clear prefetch queue, sets CS
JMP 	DWORD LINEAR_CODE_SEL:CLEAR_LABEL

[BITS 32]

CLEAR_LABEL:
; make All segs address 4G of linear memory
MOV	ECX, DWORD LINEAR_DATA_SEL
MOV	DS, CX
MOV 	ES, CX
MOV	FS, CX
MOV	GS, CX

;init stack
MOV	EDI, DWORD STACK32_START
MOV	SS,  CX
MOV	ESP, EDI
MOV	EBP, EDI

; execute a 32 bit LIDT
LIDT 	[IDTR_START]
; START THE SYSTEM!
JMP	DIEGOS_START

[BITS 16]

Error:
MOV	SI, ErrorString
CALL	PrintString
CALL	StopMotor
HLT
JMP	$

; StopMotor
; Stop the boot disk motor
; This is expected for FLOPPY DISK
; Registers used (to be saved by the caller)
; AL
StopMotor:
;stop disk motor
MOV	AL, 0x00
MOV	DX, 0x3F2
OUT 	DX, AL
RET

; PrintString
; Print a null terminated string to display
; Registers used (to be saved by the caller)
; AX
; BX
PrintString:
MOV	AH, 0x0E
MOV	BX, 0x0007
PrintNext:
LODSB
CMP	AL, 0x00
JE	PrintQuit
INT	0x10
JMP	PrintNext
PrintQuit:
RET

; PrintDot
; Print a green dot to display
; Registers used (to be saved by the caller)
; AX
; BX
PrintDot:
; Print a dot for every sector read in memory
MOV	AX, 0x0EFE
MOV	BX, 0x0007
INT	0x10
RET

A20Enable:
MOV	AL, 0xD1
OUT	0x64, AL
call	empty_8042
MOV	AL, 0xDF
OUT	0x64, AL
CALL	empty_8042
RET

empty_8042:
MOV	CX, 0x100
LOOP	$
IN	AL, 0x64
TEST	AL, 0x02
JNZ	empty_8042
RET

HelloString 	db 0xDB,0xDB,0xDD,'DiegOS Boot Loader 2.0', 0x0D, 0x0A, 0x00
ErrorString	db 'Error', 0x00
StartString     db 0x0D, 0x0A, 'Starting ...', 0x0D, 0x0A, 0x00
TIMES 510 - ($ - $$) db 0xdf

DW	0xAA55
