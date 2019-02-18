PREFIX=/usr/bin
CC=$(PREFIX)/arm-linux-gnueabihf-gcc-6
LD=$(PREFIX)/arm-linux-gnueabihf-ld

CFLAGS=-Wall-g
OBJS= DRAM_test.o 2837mmu.o
 
DRAM_test : $(OBJS)
	$(CC) -o $@ $^

.PHONY: clean
clean :
	rm -f *.o
