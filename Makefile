# Makefile
.PHONY: clean all BootLoader Disk.img Utility

all: BootLoader Kernel32 Kernel64 Disk.img

BootLoader:
	@echo 
	@echo =============== Build Boot Loader ===============
	@echo
	make -C 00.BootLoader
	@echo
	@echo =============== Build Complete ===============
	@echo

Kernel32:
	@echo 
	@echo =============== Build 32bit Kernel ===============
	@echo
	make -C 01.Kernel32
	@echo 
	@echo =============== Build Complete ===============
	@echo

Kernel64:
	@echo 
	@echo ============== Build 64bit Kernel ===============
	@echo 
	make -C 02.Kernel64
	@echo 
	@echo =============== Build Complete ===============
	@echo 

Disk.img: 00.BootLoader/BootLoader.bin 01.Kernel32/Kernel32.bin 02.Kernel64/Kernel64.bin
	@echo
	@echo =============== Disk Image Build Start ===============
	@echo
	python3 04.Utility/00.ImageMaker/ImageMaker.py $^
	@echo
	@echo =============== All Build Complete ===============
	@echo

Utility:
	@echo 
	@echo =========== Utility Build Start ===========
	@echo 
	make -C 04.Utility
	@echo 
	@echo =========== Utility Build Complete ===========
	@echo 

clean:
	make -C 00.BootLoader clean
	make -C 01.Kernel32 clean
	make -C 02.Kernel64 clean
	make -C 04.Utility clean
	rm -f Disk.img HDD.img