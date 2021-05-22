include ../../../build/makefiles/makefile.master

OBJS = kernel_time.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

CFLAGS+= -I../../../include -I../../../include/libs

all: objdirs $(OBJSO)
	$(AR) $(OBJPREFIX)_libkerneltime.a $(OBJSO)

clean:
	rm -rf $(OBJSO) *.a
