#!/bin/sh

qemu-system-x86_64 -display curses \
    -L . -m 64 -M pc \
    -blockdev driver=file,node-name=f0,filename=./Disk.img \
    -device floppy,drive=f0 $1 $2 $3