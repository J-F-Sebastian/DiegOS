include ../../build/makefiles/makefile.master

OBJS = vga.o vga_txt.o vga_palettes.o vga_fonts.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I../../include -I../../include/libs -I../../platforms

all: objdirs $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
