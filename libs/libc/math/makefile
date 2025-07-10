include $(WSROOT)/build/makefiles/makefile.master

OBJS = infinity.o 

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
all: objdirs $(OBJSO)
	$(AR) ../$(OBJPREFIX)_libc.a $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
