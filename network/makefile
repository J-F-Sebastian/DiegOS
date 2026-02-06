include $(WSROOT)/build/makefiles/makefile.master

OBJS = net_buffers.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

all: objdirs $(OBJSO)
	$(AR) $(OBJPREFIX)_network.a $(OBJSO)

clean:
	rm -rf $(OBJSO) *.a
