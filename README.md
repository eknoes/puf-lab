PUF on the Raspberry Pi
=======================

There are multiple ways a PUF on a Raspberry PI could be achieved:

1. Read SRAM (Static RAM, Cache) at earliest possible opportunity. If nothing
has been written yet the random init values should provide a PUF
2. Read DRAM at earliest possible opportunity
  * If the bits are still random and not initialized/overwritten this can be a
    PUF
3. Disable Refresh in DRAM at runtime (via kernel module), write e.g. only 1s
in a selected spot of some bits, wait a while and check which bits have flipped

#General discoveries

* Reading and writing from DRAM is possible (see *DRAM_test*)
* Physical address space is 0x30000000 to 0x3effffff (0x3f000000 is the
beginning of peripheral device addresses)
* Virtual address space is 0xc0000000 (as confirmed with the
`bcm_host_get_sdram_address()` function from the pi)

#Current issues
* Cross Compiling, even a kernel module is possible however we couldn't get it to work with the
`bcm_host.h` header. This is possibly the best way to proceed (don't use `bcm_host.h`)
* Compiling on the pi is problematic. Some discussions about how to do it are
still going on 3 years later. It should be possible though.

#Goals
* Reading from dram is possible. We need to see with what values we get it when
reading from a kernel module at runtime (so that we get unitialized values (not all 1s or 0s))
* The dram docu (see docs) describes the refresh of the RAM. It doesn't mention
a way to disable it entirely (as far as I read), but there are some ways to
disable part of it. We need to find out what the virtual address of these
registers is and if using these functions is feasible
* We coudl try to find a way to read the SRAM
