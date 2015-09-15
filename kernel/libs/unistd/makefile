include ../../../build/makefiles/makefile.master

OBJS = lseek.o open.o read.o write.o fdescr.o sleep.o fcntl.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

CFLAGS+= -I../../../include -I../../../include/libs

all: objdirs $(OBJSO)
	$(AR) $(OBJPREFIX)_libunistd.a $(OBJSO)

clean:
	rm -rf $(OBJSO) *.a
