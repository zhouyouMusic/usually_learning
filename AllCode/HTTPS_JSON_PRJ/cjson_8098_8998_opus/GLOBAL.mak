export SDK_PATH=
export OUT_PATH=$(PWD)

#export CROSS_COMPLER=/home/lan/Documents/SDK_NAND_SPEECH/01交叉编译器/mipsel-gcc472-glibc216-mips32/bin/mips-linux-gnu-
export CROSS_COMPLER=/home/zhouy/CodePRJ/R_16_ALL/code/tina/prebuilt/gcc/linux-x86/arm/toolchain-sunxi/toolchain/bin/arm-openwrt-linux-
export CC=$(CROSS_COMPLER)gcc
export AR=$(CROSS_COMPLER)ar

CFLAGS=-O3
