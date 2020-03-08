#!/bin/sh
#sudo qemu-system-i386 -cpu pentium2 -m 16 -netdev tap,id=tap0 -device rtl8139,netdev=tap0 -vga std -fda bootflop.img -boot a -serial telnet:127.0.0.1:10000,server,nowait -monitor stdio -S
sudo qemu-system-i386 -cpu pentium2 -m 32 -vga std -fda bootimage.img -boot a -serial telnet:127.0.0.1:10000,server,nowait -monitor stdio -S
#sudo qemu-system-i386 -machine q35 -cpu pentium2 -m 16 -vga std -net nic,model=rtl8139 -fda bootflop.img -boot a -serial telnet:127.0.0.1:10000,server,nowait -monitor stdio -S
