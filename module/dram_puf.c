/***********Header Files************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

/************ Variable Declaration *********/
#define INPUT_SIZE 1024*1024

#define INVERT_NEUMANN 0

#define PROCFS_MAX_SIZE		RAM_TESTSIZE
#define PROCFS_NAME 		"neumann_puf"


char * create_neumann(char *startAddr, int inputLen, int *outputSize);
void read_ram(unsigned int * start, unsigned int offset);

char *puf;
int pufSize;

static struct proc_dir_entry *ent;

static ssize_t mywrite(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
	printk( KERN_DEBUG "write handler\n");
	return -1;
}

static ssize_t myread(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
    int total_len = 0;
    printk( KERN_DEBUG " read from proc");

    if(*ppos > 0 || count < pufSize)
		return 0;
    
    if(copy_to_user(ubuf, puf, pufSize))
        return -EFAULT;

    total_len = pufSize;

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
    
    puf = create_neumann(phys_to_virt(0x10000000), INPUT_SIZE, &pufSize);
    
    printk(KERN_NOTICE "Generated PUF with size %i\n", pufSize);
    
    return 0;
}

#define LEFT_MASK 0x80
#define LEFT_PAIR 0xc0
#define RIGHT_PAIR 0x30

char * create_neumann(char *startAddr, int inputLen, int *outputSize) {

    int i, bitpair, copied_bits, output_pos;
    unsigned char curr_pair;
    char *outputAddr;

    *outputSize = INPUT_SIZE / 4;
    outputAddr = kmalloc(*outputSize, GFP_KERNEL);

    output_pos = 0;
    copied_bits = 0;

    for(i = 0; i < inputLen; i++) {

        for( bitpair = 0; bitpair < 4; bitpair++) {
            // Copy current byte, shift to current pair, mask
            curr_pair = *(startAddr + i);
            curr_pair = curr_pair << (bitpair * 2);
            curr_pair = curr_pair & LEFT_PAIR;

#ifdef INVERT_NEUMANN
            // Copy if not 00 or 01
            if ( (curr_pair == 0xc0 || curr_pair == 0x00 ) ) {
#else
            if ( !(curr_pair == 0xc0 || curr_pair == 0x00 ) ) {
#endif
                *(outputAddr + output_pos) = *(outputAddr + output_pos) | (curr_pair >> 7);

                // If 8 bits copied, move on, otherwise shift
                if(++copied_bits % 8 == 0) {
                    output_pos++;
        
                    if(*outputSize < (output_pos)) {
                        *outputSize = output_pos + 24;
                        outputAddr = krealloc(outputAddr, *outputSize, GFP_KERNEL);

                        if(outputAddr == NULL) {
                            printk("Could not realloc space");
                            return 0;
                        }
                    }

                    *(outputAddr+output_pos) = 0x00;
                } else {
                    *(outputAddr + output_pos) = *(outputAddr + output_pos) << 1;
                }
            }
        }
    }
    
    outputAddr = krealloc(outputAddr, copied_bits / 8, GFP_KERNEL);
    pufSize = copied_bits / 8;

    return outputAddr;
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

        
    startAddr = start;
    for( offset = 0; offset < testsize; offset++ ) {         // Loop to read data from memory
           currentDRAMAddr = ( start + offset );
           result = *( currentDRAMAddr );                                          
               printk(KERN_NOTICE "0x%xx : 0x%x\n", (unsigned int) currentDRAMAddr, result );
               startAddr = currentDRAMAddr;
         
    }
}

void thread_cleanup(void) {
    printk(KERN_ALERT "Module Removed\n");
    proc_remove(ent); 
    kfree(puf);
    return;
}

MODULE_LICENSE("GPL");   
module_init(thread_init);
module_exit(thread_cleanup);

MODULE_AUTHOR("PANDABOARD Rowhammer-BASED DRAM PUF");
MODULE_DESCRIPTION("Pandaboard Rowhammer-based DRAM PUF kernel module for changing DRAM refresh rate and reading/writing tp PUF memory locations");

