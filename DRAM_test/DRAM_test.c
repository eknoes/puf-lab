//  RAM Read/Write Test Program for Raspberry-Pi 3
//  20-Dec-2017
//  Kevin Hsu
// --------------------------------------------------------------------
//
//   Title     :  DRAM_test.c
//             :
//   Library   :
//             :
//   Developers:  Kevin Hsu
//             :
//   Purpose   :  RaspberryPi 3 "B" DRAM testing
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
//   V1.1   | 18-Feb-2019 | Fix Error Detection

#include <stdio.h>
#include <stdlib.h>
#include "RPI3.h"
#include "2837mmu.h"
#include <unistd.h>

#define RAM_TESTSIZE               1024                 // Test Range = (RAM_TESTSIZE x 4) Byte
#define TEST_PATTERN1             0x99999999    // Test pattern 1, set the even bits
#define TEST_PATTERN2             0x55555555    // Test pattern 2, set the odd bits 

unsigned int RAM_test(unsigned int *ptr, unsigned int pattern);   // RAM write/read function
struct bcm2837_peripheral dram = { RAM_TESTBASE };                // Declare DRAM mmap structure

int main(int argc, char **argv) { 

  setup_io( &dram );                                                                              // Init DRAM mmap

  if ( !(RAM_test(dram.addr, TEST_PATTERN1)) || !(RAM_test(dram.addr, TEST_PATTERN2)) )
           printf( "RAM test fails!! \n" );
  else
           printf( "RAM test done successfully \n" );

  release_io( &dram );                                                                            // Close DRAM mmap
  
  return 0;

} // End of main

unsigned int RAM_test ( unsigned int *ptr, unsigned int pattern ) {    // RAM write/read function

  unsigned int *currentDRAMAdd = NULL;
  unsigned int *mappedDRAMAdd =NULL;
  unsigned int offset = 0;
  unsigned int result = 0;
  unsigned int errorCount= 0;

  for( offset = 0; offset < RAM_TESTSIZE ; offset++ ) {         // Loop to write pre-defined pattern into memory
      currentDRAMAdd = ( ptr + offset );
      *( currentDRAMAdd ) = 0x0;                                              // Clear memory
      *( currentDRAMAdd ) = pattern;                                        // Write Pattern data to memory
  } // Write process

  for( offset = 0; offset < RAM_TESTSIZE ; offset++ ) {         // Loop to read data from memory
       currentDRAMAdd = ( ptr + offset );
       result = *( currentDRAMAdd );                                          
       printf( "Address: 0x%x : Value: 0x%x\n", ( ptr+offset ), result );
       if ( result != pattern ) {                                                       // Result comparing
             printf("Error Found! Offset = %x\n", offset);
             errorCount++;
       }
  } // Read process

  if (errorCount == 0)
    return 1;
  else
    return 0;

} // End of RAM write/read function
