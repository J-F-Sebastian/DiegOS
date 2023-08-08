include ../../build/makefiles/makefile.master

OBJS = NandLoader.o wb_uart.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

OBJSTART = diegos_init.o

OBJSTARTO = $(addprefix $(OBJPREFIX)/, $(OBJSTART))

CFLAGS+= -I../../include -I../../platforms/ARM/N9H26 -D__UPLL_240__

all: $(OBJS)

clean:
	rm -rf $(OBJS)
