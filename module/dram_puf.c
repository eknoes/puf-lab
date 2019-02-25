
/***
The below code tests for 60 secs for running a loop continuously
In the code, we can execute it for 60 secs


Performs Rowhammering Only



***/


/***********Header Files************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>

/************ Variable Declaration *********/
#define RAM_TESTSIZE 1024 // Test Range = (RAM_TESTSIZE x 4) Byte
#define PHYS_DRAM_ADDR 0xc0000000

void read_ram(unsigned int * start, unsigned int offset);


int thread_init (void) {   
    printk(KERN_NOTICE "Physical 0x30000000");   
    read_ram(phys_to_virt(0x30000000), RAM_TESTSIZE);
    printk(KERN_NOTICE "Physical 0x00000000");   
    read_ram(phys_to_virt(0x00000000), RAM_TESTSIZE);
    return 0;
}

void read_ram(unsigned int * start, unsigned int testsize) {
    unsigned int offset;
    unsigned int *currentDRAMAddr;
    unsigned int result;
    
    for( offset = 0; offset < testsize; offset++ ) {         // Loop to read data from memory
           currentDRAMAddr = ( start + offset );
           result = *( currentDRAMAddr );                                          
           printk(KERN_NOTICE "Address: 0x%x : Value: 0x%x\n", (unsigned int) currentDRAMAddr, result );
    }
}

void thread_cleanup(void) {
  printk(KERN_ALERT "Module Removed\n");
 
 return;
}

MODULE_LICENSE("GPL");   
module_init(thread_init);
module_exit(thread_cleanup);

MODULE_AUTHOR("PANDABOARD Rowhammer-BASED DRAM PUF");
MODULE_DESCRIPTION("Pandaboard Rowhammer-based DRAM PUF kernel module for changing DRAM refresh rate and reading/writing tp PUF memory locations");

