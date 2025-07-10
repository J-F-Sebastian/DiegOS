include $(WSROOT)/build/makefiles/makefile.master

OBJS = kernel_time.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

all: objdirs $(OBJSO)
	$(AR) $(OBJPREFIX)_libkerneltime.a $(OBJSO)

clean:
	rm -rf $(OBJSO) *.a
