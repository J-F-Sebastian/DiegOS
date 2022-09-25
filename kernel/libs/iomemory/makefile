include ../../../build/makefiles/makefile.master

OBJS = iomalloc.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

CFLAGS+= -I../../../include -I../../../include/libs -I../../../platforms \
         -I ../../

all: objdirs $(OBJSO)
	$(AR) $(OBJPREFIX)_libiomemory.a $(OBJSO)

clean:
	rm -rf $(OBJSO) *.a
