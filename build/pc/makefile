#
# DiegOS Operating System source code
#
# Copyright (C) 2012 - 2022 Diego Gallizioli
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>
#

include makefile.config
include ../makefiles/makefile.master
include ../makefiles/makefile.$(CPU)
include ../makefiles/makefile.$(PLATFORM)

STATICLIST=../../kernel/$(OBJPREFIX)_diegos.a

ARLIST=../../applications/console/$(OBJPREFIX)_app.a \
	../../applications/demo/$(OBJPREFIX)_app.a \
	../../kernel/libs/$(OBJPREFIX)_libkernel.a \
	../../kernel/libs/time/$(OBJPREFIX)_libkerneltime.a \
	../../kernel/libs/pci/$(OBJPREFIX)_libpci.a \
	../../kernel/libs/unistd/$(OBJPREFIX)_libunistd.a \
	../../kernel/libs/iomemory/$(OBJPREFIX)_libiomemory.a \
	../../kernel/$(OBJPREFIX)_kernel.a \
	../../platforms/$(PLATFORM)/$(PLATFORM).a \
	../../platforms/$(CPU)/$(CPU).a \
	../../drivers/$(OBJPREFIX)_drivers.a \
	../../libs/$(OBJPREFIX)_diegos_lib.a \
	../../libs/libc/$(OBJPREFIX)_libc.a \
	../../fat/$(OBJPREFIX)_fat.a
   
all: mbr.bin bootsect.bin
	cd ../../platforms/$(CPU) && make all
	cd ../../platforms/$(PLATFORM) && make all
	cd ../../kernel && make all
	cd ../../libs && make all
	cd ../../drivers && make -f makefile.$(OBJPREFIX) all
	cd ../../fat && make all
	cd ../../applications/console && make all
	cd ../../applications/demo && make all
	$(LD) --gc-sections -static -m elf_i386 -T diegos.ld -nostdlib -o DiegOS.elf $(STATICLIST) --start-group $(ARLIST) --end-group
	objcopy -S DiegOS.elf DiegOS.stripped.elf
	objcopy -O binary DiegOS.stripped.elf DiegOS.bin
	./mkimage -b mbr.bin -s DiegOS.bin -d bootimage.img
	./mkimage -f -b bootsect.bin -s DiegOS.bin -d bootfloppy.img

clean:
	cd ../../kernel && make clean
	cd ../../libs && make clean
	cd ../../platforms/$(CPU) && make clean
	cd ../../platforms/$(PLATFORM) && make clean
	cd ../../drivers && make -f makefile.$(OBJPREFIX) clean
	cd ../../fat && make clean
	cd ../../applications/console && make clean
	cd ../../applications/demo && make clean
	rm -f *.o *.bin *.img *.elf

mbr.bin:
	nasm -f bin -o $@ ../../boot/pc/bootmbr32.s

bootsect.bin:
	nasm -f bin -o $@ ../../boot/pc/bootsector32.s
