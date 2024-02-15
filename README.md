# MintOS
MintOS is 64Bit Multicore OS by @kkamagui.
Check out https://github.com/kkamagui/mint64os-examples/.

# Differences
## Changes
- I write ImageMaker with python3 (check 04.Utility/00.ImageMaker)
- I implement ModeSwitch not only with assembly but also with C (check 01.Kernel32/Source/ModeSwitch.c and 01.Kernel32/Source/ModeSwitchASM.asm)
## Not written
- I did not write code enabling A20 Gate.
- I did not write code checking memory size.