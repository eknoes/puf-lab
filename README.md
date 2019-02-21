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
