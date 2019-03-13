# README
The kernel module should provide a DRAM based PUF in `/proc/dram_puf`.
This PUF is calculated by reading the memory range starting at `BASE_ADDR` to `BASE_ADDR + INPUT_SIZE`.
As our experimental results showed a bias, we apply the Von Neumann Transformation twice, where the first iteration uses an inverted Von Neumann.
This reduces the size from 1MB to around 14KB, but when testing with `rng-test` from [rng-tools](https://wiki.archlinux.org/index.php/Rng-tools) it succeeds as "random".

We discovered small changes in the memory range read by our kernel module.
They lead to high differences of the generated PUFs, as we do not yet apply any error correction.
This should be done on the initial input.

## Logging of memory range
When the flag `LOG_RAM` is set, the initial input is logged to kernel log.

## Von Neumann Transformation
Is basically used to unbias input for pseudo-random number generation, it is described in 2.2 [here](https://link.springer.com/content/pdf/10.1007%2F978-3-642-21040-2_12.pdf).

Before we implemented it in the kernel module, we wrote a basic version which can be found in the directory `neumann`.

# How to compile
*Modified version of https://www.raspberrypi.org/documentation/linux/kernel/building.md*

    git clone https://github.com/raspberrypi/tools ~/Projekte/rbpi-tools
    git clone --depth=1 https://github.com/raspberrypi/linux ~/Projekte/rbpi-linux
    git clone --depth=1 git@github.com:raspberrypi/firmware.git ~/Projekte/rbpi-firmware

Afterwards, modify the Makefile to point to those directories.

    RBPITOOLS=${HOME}/Projekte/rbpi-tools
    RBPIKERNEL=${HOME}/Projekte/rbpi-linux
    RBPIFIRMWARE=${HOME}/Projekte/rbpi-firmware

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

# Experimental Results
Experimental results for Raspberry PI, which addresse can be read

## Address space
* Physical 0x30000000 works
* Physical 0xc0000000 does not work
* Virtual 0xc0000000 does not work
* Physical 0x00000000 works
* Physical 0x00000000 -> 0x000000FF looks like SDRAM Register, as from 0x000000FF on it, contents are 0x55555555
