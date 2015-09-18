include ../../build/makefiles/makefile.master

OBJS = vga_tty.o vga_tty_bcopy.o vga_tty_bset.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I../../include -I../../include/libs -I../../platforms

all: objdirs $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
