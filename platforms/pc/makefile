include ../../build/makefiles/makefile.master

OBJS = platform_init.o i8259.o memory.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I../../include -I../../include/libs -I../ia32 

all: objdirs $(OBJSO)
	nasm -g -f elf -o $(OBJPREFIX)/int86.o int86.s
	$(AR) $(PLATFORM).a $(OBJSO) $(OBJPREFIX)/int86.o
	
clean: objclean
	rm -rf $(OBJPREFIX)/int86.o
