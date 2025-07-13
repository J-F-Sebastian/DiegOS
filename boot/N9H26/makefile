include ../../build/makefiles/makefile.master

OBJS = NandLoader.o NandDrv.o wb_uart.o wb_config.o wb_cache.o wb_aic.o wb_mmu.o Startup_GCC.o \
       strlen.o memcpy.o memset.o

CFLAGS+= -I../../include -I../../platforms/ARM/N9H26 -D__UPLL_240__

all: $(OBJS)
	$(LN) -T gcc_arm_SRAM_Loader.ld -nostdlib --gc-sections -o bootN9H26.elf $(OBJS) ../../platforms/ARM/armv5te/armv5te_N9H26.a
	$(OBJCPY) -S -O binary bootN9H26.elf bootN9H26.bin

clean:
	rm -rf $(OBJS) *.elf *.bin
