include $(WSROOT)/build/makefiles/makefile.master

OBJS = lseek.o open.o read.o write.o fdescr.o sleep.o fcntl.o\
       access.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

all: objdirs $(OBJSO)
	$(AR) $(OBJPREFIX)_libunistd.a $(OBJSO)

clean:
	rm -rf $(OBJSO) *.a
