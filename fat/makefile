include ../build/makefiles/makefile.master

OBJS = disk_access.o fat.o fat_format.o fat_internals.o fat_names.o fat_print.o mbr.o bpb.o
OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

CFLAGS += -I../include

all: objdirs $(OBJSO)
	$(AR) $(OBJPREFIX)_fat.a $(OBJSO)

clean:
	rm -rf $(OBJSO) *.a
