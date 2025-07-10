include $(WSROOT)/build/makefiles/makefile.master

OBJS = vga_tty.o vga_tty_bcopy.o vga_tty_bset.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
all: objdirs $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
