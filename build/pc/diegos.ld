ENTRY(DiegOS)

SECTIONS
     {
       .text 0x110000 :
	{
		_text_start = .; *(.text) *(.rodata*); KEEP(.text.*) _text_end = .;
	}
       .data :
	{ 
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
