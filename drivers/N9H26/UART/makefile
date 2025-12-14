include ../../../build/makefiles/makefile.master

OBJS = wb_uart.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
#CFLAGS+= -I../../../include -I../../../include/libs -I../../../platforms

all: objdirs $(OBJSO)
	
clean:
	rm -rf $(OBJSO) 
