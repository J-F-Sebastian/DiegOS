ENTRY(ARM_start)

/* The size of the single stack used by the application */
IRQ_STACK_SIZE = 512;
FIQ_STACK_SIZE = 0;
SVC_STACK_SIZE = 512;
ABT_STACK_SIZE = 128;
UND_STACK_SIZE = 64;
SYS_STACK_SIZE = 512;

SECTIONS
     {
       .text 0x0 : 
	{
		_text_start = .; *(.text) *(.rodata*); KEEP(.text.*) _text_end = .;
	}
       .data :
	{
		__stack_start__ = .;

		. += IRQ_STACK_SIZE;
		__irq_stack_start__ = .;

		. += FIQ_STACK_SIZE;
	        __fiq_stack_start__ = .;

		. += SVC_STACK_SIZE;
		__svc_stack_start__ = .;

		. += ABT_STACK_SIZE;
		__abt_stack_start__ = .;

		. += UND_STACK_SIZE;
		__und_stack_start__ = .;

		. += SYS_STACK_SIZE;
		__sys_stack_start__ = .;

		__stack_end__ = .;
	
		_data_start = .;
		*(.data*)
		__init_array_start = .;
		*(.init_array)
		*(.init_array.*)
		__init_array_end = .;
		_data_end = .;
	}
       .bss (NOLOAD):
	{
		_bss_start = .; *(.bss) *(COMMON); _bss_end = .;
	}
     }
