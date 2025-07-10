include $(WSROOT)/build/makefiles/makefile.master

OBJS = iomalloc.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

all: objdirs $(OBJSO)
	$(AR) $(OBJPREFIX)_libiomemory.a $(OBJSO)

clean:
	rm -rf $(OBJSO) *.a
