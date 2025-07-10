include $(WSROOT)/build/makefiles/makefile.master

OBJS = local_loop.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
all: objdirs $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
