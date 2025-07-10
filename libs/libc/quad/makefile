include $(WSROOT)/build/makefiles/makefile.master

OBJS = umoddi3.o udivdi3.o qdivrem.o 

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
all: objdirs $(OBJSO)
	$(AR) ../$(OBJPREFIX)_libc.a $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
