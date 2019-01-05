export CPU = ia32
export PLATFORM = pc

include ../makefiles/makefile.master
include ../makefiles/makefile.$(CPU)

STATICLIST=../../kernel/$(OBJPREFIX)_diegos.a

ARLIST=../../applications/console/$(OBJPREFIX)_app.a \
	../../applications/demo/$(OBJPREFIX)_app.a \
	../../kernel/libs/$(OBJPREFIX)_libkernel.a \
	../../kernel/libs/pci/$(OBJPREFIX)_libpci.a \
	../../kernel/libs/unistd/$(OBJPREFIX)_libunistd.a \
	../../kernel/$(OBJPREFIX)_kernel.a \
	../../platforms/$(PLATFORM)/$(PLATFORM).a \
	../../platforms/$(CPU)/$(CPU).a \
	../../drivers/$(OBJPREFIX)_drivers.a \
	../../libs/$(OBJPREFIX)_diegos_lib.a \
	../../libs/libc/$(OBJPREFIX)_libc.a
   
all: bootsect.bin 
	cd ../../platforms/$(CPU) && make all
	cd ../../platforms/$(PLATFORM) && make all
	cd ../../kernel && make all
	cd ../../libs && make all
	cd ../../drivers && make -f makefile.$(OBJPREFIX) all
	cd ../../applications/console && make all
	cd ../../applications/demo && make all
	$(LD) -static -m elf_i386 -T diegos.ld -nostdlib -o DiegOS.elf $(STATICLIST) --start-group $(ARLIST) --end-group
	objcopy -S DiegOS.elf DiegOS.stripped.elf
	objcopy -O binary DiegOS.stripped.elf DiegOS.bin
	cat bootsect.bin DiegOS.bin > img.bin
	dd if=img.bin of=bootflop.img ibs=2880b conv=sync

clean:
	cd ../../kernel && make clean
	cd ../../libs && make clean
	cd ../../platforms/$(CPU) && make clean
	cd ../../platforms/$(PLATFORM) && make clean
	cd ../../drivers && make -f makefile.$(OBJPREFIX) clean
	cd ../../applications/console && make clean
	cd ../../applications/demo && make clean
	rm -f *.o *.bin *.img *.elf

bootsect.bin:
	nasm -f bin -o $@ ../../boot/pc/boots32.s

