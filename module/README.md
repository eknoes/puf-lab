# Kernel module dram_puf for the Raspberry Pi
This kernel module should provide a PUF derivated from the DRAM of the
Raspberry Pi in the location `/proc/dram_puf`.

## Concept
The PUF is calculated by reading a memory range starting at `BASE_ADDR` to
`BASE_ADDR + INPUT_SIZE`.  As our experimental results showed a bias towards
the sequence `01010101`, two correction algorithms are applied to the read
memory.

1. An inverted variation of the Von Neumann correction was applied. Here the
   bit pairs `01` and `10` where discarded, `00` converted to `0` and `11` to
   `1`.
2. The second time the Von Neumann correction is applied in its original form.

Details on the Von Neumann Correction can be found
[here](https://link.springer.com/content/pdf/10.1007%2F978-3-642-21040-2_12.pdf).
A basic implementation was attempted in the ["neumann" directory](../neumann)
of this repository.

## Cross-Compilation
*Modified version of https://www.raspberrypi.org/documentation/linux/kernel/building.md*

```bash
git clone https://github.com/raspberrypi/tools $HOME/Projekte/rbpi-tools
git clone --depth=1 https://github.com/raspberrypi/linux $HOME/Projekte/rbpi-linux
git clone --depth=1 git@github.com:raspberrypi/firmware.git $HOME/Projekte/rbpi-firmware
```

Afterwards, modify the Makefile to point to those directories.

```bash
RBPITOOLS=$HOME/Projekte/rbpi-tools
RBPIKERNEL=$HOME/Projekte/rbpi-linux
RBPIFIRMWARE=$HOME/Projekte/rbpi-firmware
```

To build the kernel module, you first have to build the kernel sources.

```bash
cd $RBPITOOLS
KERNEL=kernel7
make ARCH=arm CROSS_COMPILE=$RBPITOOLS/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf- bcm2709_defconfig
make ARCH=arm CROSS_COMPILE=$RBPITOOLS/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf- zImage modules dtbs
```

Now, you have a compiled kernel.

## Install Kernel
*This step can usually be skipped*
*It is expected that /dev/mmcblk0 is the microSD card*

```bash
cd $RBPIKERNEL
mkdir mnt
mkdir mnt/fat32
mkdir mnt/ext4
sudo mount /dev/mmcblk0p1 mnt/fat32
sudo mount /dev/mmcblk0p2 mnt/ext4

sudo make -j6 ARCH=arm CROSS_COMPILE=$RBPITOOLS/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf- INSTALL_MOD_PATH=mnt/ext4 modules_install
export KERNEL=kernel7

sudo cp mnt/fat32/$KERNEL.img mnt/fat32/$KERNEL-backup.img
sudo cp arch/arm/boot/zImage mnt/fat32/$KERNEL.img
sudo cp arch/arm/boot/dts/*.dtb mnt/fat32/
sudo cp arch/arm/boot/dts/overlays/*.dtb* mnt/fat32/overlays/
sudo cp arch/arm/boot/dts/overlays/README mnt/fat32/overlays/
sudo umount mnt/fat32
sudo umount mnt/ext4
```

## Build kernel module
Just `make` it.

## Installation
Move the file comiled kernel module `dram_puf.ko` to the Raspberry Pi and load
it with the command `insmod dram_puf.ko`. You can check the output via `dmesg`.

To load the module persistently at system startup, copy the module to
`/lib/modules/$(uname -r)/kernel/drivers/staging/dram_puf` and insert the name
into `/etc/modules` to load it at boot time. Afterwards, run `sudo depmod`.

## Usage
* **Logging of memory range:** When the flag `LOG_RAM` is set, the initial
  input is logged to kernel log
* **Location of PUF:** `/proc/dram_puf`


## Backup
### Results
This reduces the size from 1MB to around 14KB, but when testing with `rng-test`
from [rng-tools](https://wiki.archlinux.org/index.php/Rng-tools) it succeeds as
"random".

We discovered small differences on different reads of the memory range by our
kernel module.  They lead to high differences of the generated PUFs, as we do
not yet apply any form of error correction. Since the differences in the
original memory also resulted in coll This should be done on the initial input.
