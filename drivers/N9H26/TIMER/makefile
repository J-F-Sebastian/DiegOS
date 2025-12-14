include ../../../build/makefiles/makefile.master

OBJS = N9H26_TIMER.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I../../../include -I../../../include/libs -I../../../platforms

all: objdirs $(OBJSO)
	
clean:
	rm -rf $(OBJSO) 
