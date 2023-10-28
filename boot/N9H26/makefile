include ../../build/makefiles/makefile.master

OBJS = NandDrv.o NandLoader.o wb_uart.o wb_config.o wb_cache.o Startup_GCC.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

OBJSTART = diegos_init.o

OBJSTARTO = $(addprefix $(OBJPREFIX)/, $(OBJSTART))

CFLAGS+= -I../../include -I../../platforms/ARM/N9H26 -D__UPLL_240__

all: $(OBJS)
	$(LN) -T gcc_arm_SRAM_Loader.ld -nostdlib -o bootN9H26.elf $(OBJS)
	$(OBJCPY) -S -O binary bootN9H26.elf bootN9H26.bin

clean:
	rm -rf $(OBJS)
