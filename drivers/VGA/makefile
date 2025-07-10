include $(WSROOT)/build/makefiles/makefile.master

OBJS = vga.o vga_txt.o vga_palettes.o vga_fonts.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
all: objdirs $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
