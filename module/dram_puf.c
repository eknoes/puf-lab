
/***
The below code tests for 60 secs for running a loop continuously
In the code, we can execute it for 60 secs


Performs Rowhammering Only



***/


/***********Header Files************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/io.h>
#include <linux/uaccess.h>

/************ Variable Declaration *********/
#define RAM_TESTSIZE 64 // Test Range = (RAM_TESTSIZE x 4) Byte
#define PHYS_DRAM_ADDR 0xc0000000

#define PROCFS_MAX_SIZE		RAM_TESTSIZE
#define PROCFS_NAME 		"puf-memory-map"


void read_ram(unsigned int * start, unsigned int offset);
int ram_to_buf(unsigned int * start, unsigned int testsize, char * buffer);

static struct proc_dir_entry *ent;

static ssize_t mywrite(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
	printk( KERN_DEBUG "write handler\n");
	return -1;
}

static ssize_t myread(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
	char buf[PROCFS_MAX_SIZE * 16];
    int len = 0;
    int i = 0;
    int total_len = 0;

    if(*ppos > 0 || count < PROCFS_MAX_SIZE)
		return 0;
    
    //len = sprintf(buf, "Here comes the PUF Value");

    for(i = 0; i < 100; i++) {        
        len = ram_to_buf(phys_to_virt(0x00000000), RAM_TESTSIZE, buf);
       
        if(copy_to_user(ubuf + total_len,buf,len))
            return -EFAULT;

        total_len += len;
        
    }

    *ppos = total_len;
    return total_len;
}

static struct file_operations myops = 
{
	.owner = THIS_MODULE,
	.read = myread,
	.write = mywrite,
};


int thread_init (void) {   
	ent = proc_create(PROCFS_NAME, 0660, NULL, &myops);
/*  printk(KERN_NOTICE "Physical 0x30000000");   
  read_ram(phys_to_virt(0x30000000), RAM_TESTSIZE);
    printk(KERN_NOTICE "Physical 0x00000000");   
    read_ram(phys_to_virt(0x00000000), RAM_TESTSIZE);
    printk(KERN_NOTICE "Physical 0x10000000");   
    read_ram(phys_to_virt(0x10000000), RAM_TESTSIZE);
    printk(KERN_NOTICE "Physical 0x20000000");   
    read_ram(phys_to_virt(0x20000000), RAM_TESTSIZE);*/
    return 0;
}

int ram_to_buf(unsigned int * start, unsigned int testsize, char * buffer) {
    unsigned int offset;
    unsigned int *currentDRAMAddr;
    unsigned int *startAddr;
    unsigned int result;
    unsigned int last_result;
    unsigned int len = 0;
        
    startAddr = start;
    for( offset = 0; offset < testsize; offset++ ) {         // Loop to read data from memory
           currentDRAMAddr = ( start + offset );
           result = *( currentDRAMAddr );                                          
           if(last_result != result) {
               last_result = result;
               len += sprintf(buffer + len, "0x%x +%d: 0x%x\n", (unsigned int) startAddr, (unsigned int) (currentDRAMAddr - startAddr), result );
               startAddr = currentDRAMAddr;
           } 
    }

    return len;
}


void read_ram(unsigned int * start, unsigned int testsize) {
    unsigned int offset;
    unsigned int *currentDRAMAddr;
    unsigned int *startAddr;
    unsigned int result;
    unsigned int last_result;

        
    startAddr = start;
    for( offset = 0; offset < testsize; offset++ ) {         // Loop to read data from memory
           currentDRAMAddr = ( start + offset );
           result = *( currentDRAMAddr );                                          
           if(last_result != result) {
               last_result = result;
               printk(KERN_NOTICE "Address: 0x%x to 0x%x : Value: 0x%x\n", (unsigned int) startAddr, (unsigned int) currentDRAMAddr, result );
               startAddr = currentDRAMAddr;
           } 
    }
}

void thread_cleanup(void) {
  printk(KERN_ALERT "Module Removed\n");
	proc_remove(ent); 
 return;
}

MODULE_LICENSE("GPL");   
module_init(thread_init);
module_exit(thread_cleanup);

MODULE_AUTHOR("PANDABOARD Rowhammer-BASED DRAM PUF");
MODULE_DESCRIPTION("Pandaboard Rowhammer-based DRAM PUF kernel module for changing DRAM refresh rate and reading/writing tp PUF memory locations");

