obj-m += dram_puf.o

CCPREFIX=${RBPITOOLS}/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-
ccflags-y+=-Wall -g -lbcm_host -I${RBPIFIRMWARE}/opt/vc/include -I/usr/include  -L${RBPIFIRMWARE}/opt/vc/lib

all:
	make ARCH=arm CROSS_COMPILE=${CCPREFIX} -C ${RBPIKERNEL} M=$(PWD) modules

clean:
	make -C ${RBPIKERNEL} M=$(PWD) clean
