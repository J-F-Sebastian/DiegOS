ENTRY(DiegOS)

SECTIONS
     {
       .text 0x10000 : { 
		*(.text);
		*(.rodata*);
	    }
       .data : { 
		*(.data*); 
	    }
       .bss : { 
		*(.bss*);
	    }
     }
