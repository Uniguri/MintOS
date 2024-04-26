#!/bin/sh

qemu-system-x86_64 \
    -L . -m 64M -M pc \
    -drive file=./HDD.img,format=raw,index=0,media=disk \
    -blockdev driver=file,node-name=f0,filename=./Disk.img \
    -device floppy,drive=f0 $1 $2 $3