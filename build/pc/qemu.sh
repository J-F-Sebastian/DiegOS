#!/bin/sh
#qemu-system-i386 -cpu pentium2 -m 16 -net nic,model=ne2k_pci -vga std -fda bootfloppy.img -boot a -serial telnet:127.0.0.1:10000,server,nowait -monitor stdio -S
#qemu-system-i386 -cpu pentium2 -m 32 -vga std -fda bootfloppy.img -boot a -serial telnet:127.0.0.1:10000,server,nowait -monitor stdio -S
#sudo qemu-system-i386 -machine q35 -cpu pentium2 -m 16 -vga std -net nic,model=rtl8139 -fda bootflop.img -boot a -serial telnet:127.0.0.1:10000,server,nowait -monitor stdio -S
qemu-system-i386 -cpu pentium2 -m 16 -netdev tap,id=mynet0,ifname=tap0,script=no,downscript=no -device rtl8139,netdev=mynet0,mac=52:54:00:12:34:56 -vga std -fda bootfloppy.img -boot a -serial telnet:127.0.0.1:10000,server,nowait -monitor stdio -S
