include ../../build/makefiles/makefile.master

OBJS = platform_init.o i8259.o memory.o

#OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I../../include -I../../include/libs -I../ia32 

all: $(OBJS)
	nasm -g -f elf -o int86.o int86.s
	$(AR) $(PLATFORM).a $(OBJS)	int86.o
	
clean:
	rm -rf $(OBJS) *.a int86.o
