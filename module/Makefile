obj-m += dram_puf.o

RBPI-TOOLS=/home/soenke/Projekte/rbpi-tools
RBPI-KERNEL=/home/soenke/Projekte/rbpi-linux



CCPREFIX=${RBPI-TOOLS}/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-

all:
	make ARCH=arm CROSS_COMPILE=${CCPREFIX} -C ${RBPI-KERNEL} M=$(PWD) modules

clean:
	make -C ${RBPI-KERNEL} M=$(PWD) clean
