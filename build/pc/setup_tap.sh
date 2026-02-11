#!/usr/bin/bash

sudo ip tuntap add dev tap0 mode tap user diego
sudo ip addr add 192.168.10.1/24 dev tap0
sudo ip link set tap0 up

