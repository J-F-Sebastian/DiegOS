include $(WSROOT)/build/makefiles/makefile.master

OBJS = vesa.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
all: objdirs $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
