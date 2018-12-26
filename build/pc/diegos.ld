ENTRY(DiegOS)

SECTIONS
     {
       .text 0x10000 : 
	{
		_btext = .; *(.text) *(.rodata*); KEEP(.text.*) _etext = .;
	}
       .data :
	{ 
		_bdata = .; *(.data*); _edata = .;
	}
       .bss :
	{ 
		_bbss = .; *(.bss*); _ebss = .;
	}
	.init.array :
	{
		__init_array_start = .;
		*(.init_array)
		*(.init_array.*)
		__init_array_end = .;
	}
     }
