export CPU = ia32
export PLATFORM = pc

include ../makefiles/makefile.master
include ../makefiles/makefile.$(CPU)

STATICLIST=../../kernel/$(OBJPREFIX)_diegos.a

ARLIST=-l:../../applications/console/$(OBJPREFIX)_app.a \
	-l:../../applications/demo/$(OBJPREFIX)_app.a \
	-l:../../kernel/libs/$(OBJPREFIX)_libkernel.a \
	-l:../../kernel/libs/pci/$(OBJPREFIX)_libpci.a \
	-l:../../kernel/libs/unistd/$(OBJPREFIX)_libunistd.a \
	-l:../../kernel/$(OBJPREFIX)_kernel.a \
	-l:../../platforms/$(PLATFORM)/$(PLATFORM).a \
	-l:../../platforms/$(CPU)/$(CPU).a \
	-l:../../drivers/$(OBJPREFIX)_drivers.a \
	-l:../../libs/$(OBJPREFIX)_diegos_lib.a \
	-l:../../libs/libc/$(OBJPREFIX)_libc.a \
	-l:../../networking/netw/$(OBJPREFIX)_diegos_netw.a \
	-l:../../networking/ip/$(OBJPREFIX)_ip.a
   
all: bootsect.bin 
	cd ../../platforms/$(CPU) && make all
	cd ../../platforms/$(PLATFORM) && make all
	cd ../../kernel && make all
	cd ../../libs && make all
	cd ../../drivers && make -f makefile.$(OBJPREFIX) all
	cd ../../networking && make all
	cd ../../applications/console && make all
	cd ../../applications/demo && make all
	ld -m elf_i386 -T diegos.ld -nostdlib -o DiegOS.elf $(STATICLIST) --start-group $(ARLIST) --end-group
	objcopy -S -O binary --set-section-flags .bss=alloc,load,contents DiegOS.elf DiegOS.bin
	cat bootsect.bin DiegOS.bin > img.bin
	dd if=img.bin of=bootflop.img ibs=2880b conv=sync

clean:
	cd ../../kernel && make clean
	cd ../../libs && make clean
	cd ../../platforms/$(CPU) && make clean
	cd ../../platforms/$(PLATFORM) && make clean
	cd ../../drivers && make -f makefile.$(OBJPREFIX) clean
	cd ../../networking && make clean
	cd ../../applications/console && make clean
	cd ../../applications/demo && make clean
	rm -f *.o *.bin *.img *.elf

bootsect.bin:
	nasm -f bin -o $@ ../../boot/pc/boots32.s

