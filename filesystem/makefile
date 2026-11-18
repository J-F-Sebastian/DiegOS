include $(WSROOT)/build/makefiles/makefile.master

OBJS = fs.o fs_dump.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

all: objdirs $(OBJSO)
	$(AR) $(OBJPREFIX)_fs.a $(OBJSO)

clean:
	rm -rf $(OBJSO) *.a
