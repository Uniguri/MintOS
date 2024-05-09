# MintOS

MintOS is 64Bit Multicore OS by @kkamagui.
Check out <https://github.com/kkamagui/mint64os-examples/>.

## How to run

1. Install `gcc-multilib`, `nasm`, `objcopy` and `python3`.
2. Type `make`
3. Execute `make_qemu_hdd` with size of HDD.
4. Execute `run_qemu`.

## Differences

I cannot check all differences. T^T..

## Issue

1. Sometime SS register is zero after ireq on kISRTimer.
2. If HDD is bigger than 127G, cannot detect size of HDD.