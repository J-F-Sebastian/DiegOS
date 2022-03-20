;
; DiegOS Operating System source code
;
; Copyright (C) 2012 - 2015 Diego Gallizioli
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
;
;DiegOS boot sector
;
;This is a simplified boot sector, mainly used under VirtualBox
;The assumptions are:
;
; 1) Very little error checking, AX is dumped at the end of the
;    error string if reading from floppy fails
; 2) System boots from floppy disk #1 (disk 0)
; 3) Floppy disk is 1.44MB format, 80 tracks, 18 sectors, 2 heads,
;    512 bytes per sector
; 4) System loader is at a fixed location on disk, size cannot
;    exceed 576KBytes
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
;   |          ....         |   DiegOS BOOT VARIABLES (4096 Bytes)
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

IMG_SIZE	EQU 512; in sectors, 512 bytes per sector. This value MUST NOT
                   ; be greater than 1152, or your image will be written to
                   ; the VGA memory...

RAM_START	EQU 0x0600
STACK16_START	EQU 0x7000
STACK32_START	EQU 0x0000F000
DIEGOS_VARS 	EQU 0x0000F000
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
LINEAR_CODE_SEL 	    EQU 1*8
LINEAR_DATA_SEL		    EQU 2*8
LINEAR_PROTO_CODE_LO	EQU 0x000000A0
LINEAR_PROTO_DATA_LO 	EQU 0x0000FFFF
LINEAR_PROTO_CODE_HI 	EQU 0x00C09B00
LINEAR_PROTO_DATA_HI 	EQU 0x00CF9200

; Protection Enable Bit in CR0, no paging
PE_BIT EQU 1
CD_BIT EQU 1<<30
NW_BIT EQU 1<<29 

BootLoader:
CLD
; Init DS,SS to have a temporary stack and data access
; Increment counters
XOR	AX, AX
XOR	BX, BX
XOR	CX, CX
XOR	DX, DX

MOV	DS, AX
MOV	ES, AX
MOV	SS, AX
MOV	SP, STACK16_START
MOV	BP, SP

MOV	SI, HelloString
CALL	PrintString

;reset first disk drive
MOV	AH, 0x00
MOV	DL, 0x00
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
MOV	BX, WORD DIEGOS_START>>4
MOV	ES, BX
MOV BP, BX
XOR	BX, BX
MOV SI, WORD IMG_SIZE

;1.44 MB Disk Format support 18 sectors per track,
; or 0x12. the first sector is used by boot code.
; Function 0x02 of INT 0x13, read data from disk
;MOV	AH, 0x02
;sectors to be read, first round not in loop
;MOV	AL, 0x01
MOV     AX, 0x0201
;starting cylinder/track
;MOV	CH, 0x00
;starting sector
;MOV	CL, 0x02
MOV     CX, 0x0002
;starting head
;MOV	DH, 0x00
;1st disk unit
;MOV	DL, 0x00
XOR     DX, DX

readOneSector:

INT	0x13
JC	Error

DEC SI
JZ  StartDiegOS

ADD BX, 512
JNC NoNewSegment

;MOV AL, 0x23 ; Print # every 64 KBytes
;CALL PrintChar
;XOR BX, BX
ADD BP, WORD DIEGOS_START>>4
MOV ES, BP
NoNewSegment:

MOV	AX, 0x0201
INC CL
CMP CL, 0x12
JNA readOneSector

MOV CL, 1
ADD CH, DH
XOR	DH, 0x01

JMP readOneSector

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

StartDiegOS:
;stop disk motor
MOV	AL, 0x00
MOV DX, 0x3F2
OUT DX, AL

;Clear interrupts
MOV SI, LoadString
CALL    PrintString
CLI

;Enable Gate A20
CALL    WaitForKeyboard
MOV     AL, 0xD1
OUT     0x64, AL
CALL    WaitForKeyboard
MOV     AL, 0xDF
OUT     0x60, AL
CALL    WaitForKeyboard

;Detect free memory
XOR     ECX, ECX
XOR     EDX, EDX
MOV     AX, 0xE801
INT     0x15		; request upper memory size
JC      Error
CMP     AH, 0x86		; unsupported function
JE      Error
CMP     AH, 0x80		; invalid command
JE      Error
JCXZ    useEAX		; was the CX result invalid?
MOV     EAX, ECX
MOV     EBX, EDX
useEAX:
; EAX = number of contiguous Kb, 1M to 16M
; EBX = contiguous 64Kb pages above 16M
SHL     EAX, 10
SHL     EBX, 16
ADD     EAX, EBX
MOV     EBX, DWORD DIEGOS_VARS
MOV     DWORD [EBX], 0x00100000
MOV     DWORD [EBX + 4], EAX

;Assign segments for proper kernel start
XOR	AX, AX
MOV	ES, AX

; IDT is initialized by kernel
; Setup GDT
MOV 	EBX, DWORD GDT_DESC

; First GDT entry is NULL
MOV 	DWORD [EBX], 0
MOV 	DWORD [EBX + 4], 0

ADD	EBX, 8
; Second GDT entry is for FLAT MEMORY CODE
MOV 	DWORD [EBX], LINEAR_PROTO_CODE_LO
MOV 	DWORD [EBX + 4], LINEAR_PROTO_CODE_HI

ADD	EBX, 8
; Third GDT entry is for FLAT MEMORY DATA
MOV 	DWORD [EBX], DWORD LINEAR_PROTO_DATA_LO
MOV 	DWORD [EBX + 4], DWORD LINEAR_PROTO_DATA_HI

; Setup IDTR
MOV	    EAX, DWORD IDT_DESC
MOV	    EBX, DWORD IDTR_START
MOV	    WORD [EBX], (256 * 8) - 1
MOV 	DWORD [EBX + 2], EAX

; Setup GDTR
MOV	    EAX, DWORD GDT_DESC
MOV	    EBX, DWORD GDTR_START
MOV	    WORD [EBX], (8 * 8) - 1
MOV 	DWORD [EBX + 2], EAX

; execute a 32 bit LGDT
o32	LGDT [GDTR_START]

; enter protected mode
MOV 	EBX, CR0
OR 	    EBX, PE_BIT
AND     EBX, ~CD_BIT
AND		EBX, ~NW_BIT
MOV 	CR0, EBX

; clear prefetch queue, sets CS
JMP 	DWORD LINEAR_CODE_SEL:CLEAR_LABEL

[BITS 32]

CLEAR_LABEL:
; make All segs address 4G of linear memory
MOV	    ECX, DWORD LINEAR_DATA_SEL
MOV	    DS, CX
MOV 	ES, CX
MOV	    FS, CX
MOV	    GS, CX

;init stack
MOV	    EDI, DWORD STACK32_START
MOV	    SS,  CX
MOV	    ESP, EDI
MOV	    EBP, EDI

; execute a 32 bit LIDT
LIDT 	[IDTR_START]

JMP	    DIEGOS_START

[BITS 16]

WaitForKeyboard:
IN      AL, 0x64
TEST    AL, 2
JNZ     WaitForKeyboard
RET

Error:
;stop disk motor
MOV	AL, 0x00
MOV DX, 0x3F2
OUT DX, AL

PUSH	SI
PUSH	AX
MOV	SI, ErrorString
CALL	PrintString

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ENABLE THE TRACES ONLY FOR DEBUGGING      ;
; IT SHOULD NOT BE REALLY USEFUL ONCE THE   ;
; LOADER IS RUNNING FINE                    ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;MOV	AL, 'A'
;MOV	AH, 0x0E
;MOV	BH, 0x00
;MOV	BL, 0x07
;INT	0x10
;MOV AL, 'X'
;INT 0x10
;MOV AL, ':'
;INT 0x10

;trace error codes
;POP	CX
;MOV	AL, CH
;ADD	AL, 'A'
;INT	0x10

;MOV	AL, CL
;ADD	AL, 'A'
;INT	0x10
;;;;;;;;;;;;;;;;;;;;;;;;;

;MOV	AL, ' '
;INT	0x10
;MOV	AL, 'S'
;INT	0x10
;MOV AL, 'I'
;INT 0x10
;MOV AL, ':'
;INT 0x10

;POP	CX
;MOV	AL, CH
;ADD	AL, 'A'
;INT	0x10

;MOV	AL, CL
;ADD	AL, 'A'
;INT	0x10

HLT

JMP	$

PrintChar:
MOV	AH, 0x0E
;MOV	BH, 0x00
;MOV	BL, 0x07
MOV BX, 0x0007
INT	0x10
RET

PrintString:
Next:
MOV	AL, [SI]
INC	SI
OR	AL, AL
JZ	Exit
CALL	PrintChar
JMP	Next
Exit:
RET

HelloString db 0xDB,0xDB,0xDD,'DiegOS Boot Loader v1.0', 0x0A, 0x0D, 0x00
ErrorString db 0x0A, 0x0D, ' ERROR !!!', 0x00
LoadString db 'Loading...', 0x0A, 0x0D, 0x00

TIMES	510 - ($ - $$) db 0xdf
DW	0xAA55
