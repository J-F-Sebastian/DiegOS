include $(WSROOT)/build/makefiles/makefile.master

OBJS = fdc.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

all: objdirs $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
