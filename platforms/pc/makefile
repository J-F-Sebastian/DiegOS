include ../../build/makefiles/makefile.master

OBJS = platform_init.o i8259.o 

#OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I../../include -I../../include/libs -I../ia32 

all: $(OBJS)
	$(AR) $(PLATFORM).a $(OBJS)
	
clean:
	rm -rf $(OBJS) *.a
