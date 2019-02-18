// --------------------------------------------------------------------
//
//   Title     :  2837mmu.h
//             :
//   Library   :
//             :
//   Developers:  Kevin Hsu
//             :
//   Purpose   :  RaspberryPi 3 "B" BCM2837 peripheral address mapping
//             :
//   Limitation:
//             :
//   Note      :
//             :
// --------------------------------------------------------------------
//   modification history :
// --------------------------------------------------------------------
//   Version| mod. date: |
//   V1.0   | 03-Dec-2017 | Programming Start

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "2837mmu.h"

#include <unistd.h>

void setup_io( struct bcm2837_peripheral *ptr ) {                                    // Set up memory region to access peripheral
   if ( ( ptr-> mem_fd = open("/dev/mem", O_RDWR|O_SYNC)  ) < 0 ) {  // Open dev/mem
      printf("can't open /dev/mem \n");
      exit (-1);
   }

   if ( (ptr-> mem = malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL ) { // Allocate map block
      printf("allocation error \n");
      exit (-1);
   }

   if ( (unsigned long)ptr-> mem % PAGE_SIZE )                                        // Make suer pointer is at 4K boundary
       ptr-> mem += PAGE_SIZE - ( (unsigned long)ptr-> mem % PAGE_SIZE );

   ptr-> map = (unsigned char *)mmap(                                                     // Map mmap
      (caddr_t)ptr->mem,
      BLOCK_SIZE,
      PROT_READ|PROT_WRITE,
      MAP_SHARED|MAP_FIXED,
      ptr-> mem_fd,
      ptr-> phi_addr
   );

   if ((long)ptr-> map < 0) { 
      printf( "mmap error %d\n", (int)ptr-> map );
      exit (-1);
   }

   ptr->addr = (volatile unsigned *)ptr-> map;                                          //Get the returned pointer

} // End of setup_io 

void release_io(struct bcm2837_peripheral *ptr) {                                // Release mmap
    munmap(ptr->map, BLOCK_SIZE);
    close(ptr->mem_fd);
} // End of release_io
