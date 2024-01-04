#!/bin/sh

make
# qemu-system-x86_64 --nographic -L . -m 64 -M pc -drive format=raw,file=./Disk.img $1 $2 $3
# qemu-system-x86_64 -display curses -L . -m 64 -M pc -drive format=raw,file=./Disk.img $1 $2 $3
qemu-system-x86_64 -display curses -L . -m 64 -M pc  -blockdev driver=file,node-name=f0,filename=./Disk.img -device floppy,drive=f0  $1 $2 $3