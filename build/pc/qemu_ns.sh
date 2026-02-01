#!/bin/sh
#qemu-system-i386 -cpu pentium2 -m 16 -net nic,model=rtl8139 -vga std -fda bootfloppy.img -boot a -serial telnet:127.0.0.1:10000,server,nowait -monitor stdio
#qemu-system-i386 -cpu pentium2 -m 16 -netdev tap,id=tap0 -device ne2k_isa,netdev=tap0 -vga std -fda bootfloppy.img -boot a -serial telnet:127.0.0.1:10000,server,nowait -monitor stdio
#qemu-system-i386 -cpu pentium2 -m 16 -vga std -fda bootfloppy.img -boot a -serial telnet:127.0.0.1:10000,server,nowait -monitor stdio
qemu-system-i386 -cpu pentium2 -m 16 -netdev tap,id=mynet0,ifname=tap0,script=no,downscript=no -device rtl8139,netdev=mynet0,mac=52:54:00:12:34:56 -vga std -fda bootfloppy.img -boot a -serial telnet:127.0.0.1:10000,server,nowait -monitor stdio
