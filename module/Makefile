obj-m += dram_puf.o

CCPREFIX=${RBPITOOLS}/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-

all:
	make ARCH=arm CROSS_COMPILE=${CCPREFIX} -C ${RBPIKERNEL} M=$(PWD) modules

clean:
	make -C ${RBPIKERNEL} M=$(PWD) clean
