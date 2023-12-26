#!/bin/sh

make
# qemu-system-x86_64 --nographic -L . -m 64 -M pc -drive format=raw,file=./Disk.img
qemu-system-x86_64 -display curses -L . -m 64 -M pc -drive format=raw,file=./Disk.img