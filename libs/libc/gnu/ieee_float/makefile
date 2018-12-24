include ../../../../build/makefiles/makefile.master

OBJS = fphook.o frexp.o ldexp.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I../../../../include 

all: objdirs $(OBJSO)
	$(AR) ../../$(OBJPREFIX)_libc.a $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
