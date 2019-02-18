# How to compile
*Modified version of https://www.raspberrypi.org/documentation/linux/kernel/building.md*

    git clone https://github.com/raspberrypi/tools ~/Projekte/rbpi-tools
    git clone --depth=1 https://github.com/raspberrypi/linux ~/Projekte/rbpi-linux

Afterwards, modify the Makefile to point to those directories.

    RBPI-TOOLS=${HOME}/Projekte/rbpi-tools
    RBPI-KERNEL=${HOME}/Projekte/rbpi-linux
