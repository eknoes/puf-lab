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

Now, you have a compiled kernel.

## Install Kernel
*This step can usually be skipped*
*It is expected that /dev/mmcblk0 is the microSD card*

    cd ${RBPIKERNEL}
    mkdir mnt
    mkdir mnt/fat32
    mkdir mnt/ext4
    sudo mount /dev/mmcblk0p1 mnt/fat32
    sudo mount /dev/mmcblk0p2 mnt/ext4

    sudo make -j6 ARCH=arm CROSS_COMPILE=${RBPITOOLS}/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf- INSTALL_MOD_PATH=mnt/ext4 modules_install
    export KERNEL=kernel7

    sudo cp mnt/fat32/$KERNEL.img mnt/fat32/$KERNEL-backup.img
    sudo cp arch/arm/boot/zImage mnt/fat32/$KERNEL.img
    sudo cp arch/arm/boot/dts/*.dtb mnt/fat32/
    sudo cp arch/arm/boot/dts/overlays/*.dtb* mnt/fat32/overlays/
    sudo cp arch/arm/boot/dts/overlays/README mnt/fat32/overlays/
    sudo umount mnt/fat32
    sudo umount mnt/ext4

## Build kernel module
Just `make` it.

## Install kernel module
Move the file `dram_puf.ko` to the raspberry and load it with `insmod dram_puf.ko`. You can check the output via `dmesg`.