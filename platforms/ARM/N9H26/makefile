include $(WSROOT)/build/makefiles/makefile.master

OBJS = processor_init.o platform_init.o memory.o interrupts.o vectors.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS += -I$(WSROOT)/drivers/N9H26

all: objdirs $(OBJSO)
	$(AR) $(PLATFORM).a $(OBJSO)
	
clean: objclean
	rm -rf $(OBJSO) *.a
