include $(WSROOT)/build/makefiles/makefile.master

OBJS = i8042.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

all: objdirs $(OBJSO)

clean:
	rm -rf $(OBJSO) *.a
