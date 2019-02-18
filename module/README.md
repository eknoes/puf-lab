# How to compile
*Modified version of https://www.raspberrypi.org/documentation/linux/kernel/building.md*

    git clone https://github.com/raspberrypi/tools ~/Projekte/rbpi-tools
    git clone --depth=1 https://github.com/raspberrypi/linux ~/Projekte/rbpi-linux

Afterwards, modify the Makefile to point to those directories.

    RBPITOOLS=${HOME}/Projekte/rbpi-tools
    RBPIKERNEL=${HOME}/Projekte/rbpi-linux

To build the kernel module, you first have to build the kernel sources.
Therefore, cd to RBPI-TOOLS.

    KERNEL=kernel7
    make ARCH=arm CROSS_COMPILE=${RBPITOOLS}/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf- bcm2709_defconfig
    make ARCH=arm CROSS_COMPILE=${RBPITOOLS}/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf- zImage modules dtbs

To install the compiled kernel, follow [this](https://www.raspberrypi.org/documentation/linux/kernel/building.md#choosing_sources) steps from "Install directly onto the SD card".
