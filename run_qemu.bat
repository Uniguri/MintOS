@echo off

set qemu="C:\Program Files\qemu\qemu-system-x86_64.exe"

%qemu% -L . -m 64M -M pc -blockdev driver=file,node-name=f0,filename=./Disk.img -device floppy,drive=f0 %1 %2 %3