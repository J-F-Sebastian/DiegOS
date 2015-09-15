include ../../../build/makefiles/makefile.master

OBJS = pci_lib.o pci_ia32.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

CFLAGS+= -I../../../include -I../../../include/libs -I../../../platforms \
         -I ../../

all: objdirs $(OBJSO)
	$(AR) $(OBJPREFIX)_libpci.a $(OBJSO)

clean:
	rm -rf $(OBJSO) *.a
