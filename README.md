# PUF on the Raspberry Pi
*Privacy Enhancing Technologies Seminar WS18/19*

In this repository the goal was pursued, to extract a PUF (Physical Uncloneable
Function) froma Raspberry Pi and provide it in a form to be later used in a
proof-of-concept SSO implementation.

## Repository Index
* **[module](module):** The kernel module implementation providing a PUF.
  Further information in the directory.
* **[test-results](test-results):** Test results during different development
  stages.
* **[neumann](neumann):** Initial implementation of the Von Neumann correction
  algorithm.

## Approach
We pursued the goal to extract a PUF from the values read from unused DRAM of
the Raspberry Pi during startup (by using a kernel module).

Other approaches such as getting a PUF from SRAM (cache) or through
deactivating the refresh of the DRAM and getting a PUF from how the values in
the selected DRAM region changed were not feasible. This is the reason, as
because of the closed source firmware and not much information being available
about the exact hardware of the Pi, we were unable to find hardware registers
that could be used to achieve such goals.

## Current State
We discovered at least one memory section which was similar on each reboot, but
different between Pis (starting at `0x10000000`).

Using the current version of this module the size of a chosen memory section
will be reduced from 1MB to around 14KB in an attempt to reduce bias, but this
way, when testing with `rng-test` from
[rng-tools](https://wiki.archlinux.org/index.php/Rng-tools) it succeeds as
"random".

We discovered small differences on different reads of the memory range by our
kernel module. They lead to high differences of the generated PUFs, as we do
not yet apply any form of error correction.
As such the module provides a random number generator and could be usable as a
PUF in further iterations.


## Useful
* Reading `/proc/iomem` to get physical memory addresse ranges [1](https://superuser.com/questions/480451/what-kind-of-memory-addresses-are-the-ones-shown-by-proc-ioports-and-proc-iomem)

## Contributors
* Sönke Huster ([eknoes](github.com/eknoes))
* Fabian Hinz ([naibaf0](github.com/naibaf0))
* Martin Weber and Maximilian Fries from https://github.com/MokaMokiMoke/PrivTechSSO

## Related Work
PUF based SSO (Single Sign-On) (https://github.com/MokaMokiMoke/PrivTechSSO)
