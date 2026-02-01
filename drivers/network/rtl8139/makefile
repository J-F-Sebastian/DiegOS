include $(WSROOT)/build/makefiles/makefile.master

OBJS = rtl8139.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

all: objdirs $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
