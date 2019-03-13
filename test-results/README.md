# Experimental Results
We have different results from the different steps taken, that we used to
compare the contents of different memory ranges and later on the different
approaches for the puf.  Just the first step was taken on different devices, as
the later approaches based on these results.

## 01-find-range
The first step was logging different memory ranges to kernel log to find an
appropiate one for a potential puf.

### Possible Address space
To locate the DRAM, we used some trial-and-error for different ranges, based on
the [chip documentation](../docs/BCM2835-ARM-Peripherals.pdf)

* Physical 0x30000000 works
* Physical 0xc0000000 does not work
* Virtual 0xc0000000 does not work
* Physical 0x00000000 works
* Physical 0x00000000 -> 0x000000FF looks like SDRAM Register, as from 0x000000FF on it, contents are 0x55555555
 
### Possible Ranges
Now we read the content of the memory on different ranges and different devices
several times after boot, to see which could be used as PUF

* Base Addr: 0x00000000: Same (Device level, register + X)
* Base Addr: 0x00e00000: Really different (Application Data?)
* Base Addr: 0x10000000: Same (device level), Quite Similar (Both PIs)
* Base Addr: 0x20000000: Same (device level), Similarr (Both PIs)
* Base Addr: 0x30000000: Really different
 

## 02-inv-neumann-puf
We implemented the Von Neumann correction algorithm. We inverted it and
compared after several boot, where we took a short break and a long break
between the boots.

## 03-final-inverted-normal-neumann-puf
As after inverting it, we had a strong bias for `11`, we apply the common Von
Neumann correction algorithm afterwards.

## File Tree
```
├── pi1
│   └── 01-find-range
│       ├── addr1.txt
│       └── addr2.txt
├── pi2
│   ├── 01-find-range
│   │   ├── addr1_pi2.txt
│   │   ├── addr2_pi2.txt
│   │   ├── addr3_pi2.txt
│   │   ├── addr4_pi2.txt
│   │   ├── addr5_pi2.txt
│   │   ├── addr6_pi2.txt
│   │   ├── addr_blank.txt
│   │   ├── addresses_1.txt
│   │   └── addresses_2.txt
│   ├── 02-inv-neumann-puf
│   │   ├── test1.txt
│   │   ├── test2.txt
│   │   ├── test3.txt
│   │   ├── test4.txt
│   │   ├── test5.txt
│   │   └── test6.txt
│   ├── 03-final-inverted-normal-neumann-puf
│   │   ├── test1.txt
│   │   ├── test2.txt
│   │   └── test3.txt
│   └── 03-final-inverted-normal-neumann-puf-binary
│       ├── test1.bin
│       ├── test1_binary_repr.txt
│       ├── test1_bits_only.txt
│       ├── test2.bin
│       ├── test2_binary_repr.txt
│       └── test2_bits_only.txt

```
