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
#define BASE_ADDR 0x10000000 // Physical Address for PUF generation
#define PROCFS_NAME "dram_puf"

#define LOG_RAM 0

char * run_neumann(char *start_addr, int input_len, int *output_size, int inverted);
void log_ram_content(unsigned int * start, unsigned int length);

char *puf;
int puf_size;

static struct proc_dir_entry *entry;

static ssize_t read_puf(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
    if(*ppos > 0 || count < puf_size)
		return 0;
    
    if(copy_to_user(ubuf, puf, puf_size))
        return -EFAULT;

    *ppos = puf_size;

    return puf_size;
}


static struct file_operations proc_operations = 
{
	.owner = THIS_MODULE,
	.read = read_puf,
};

int puf_init (void) {   
    char *tmp_puf;
    int size;

#if LOG_RAM 
    printk(KERN_NOTICE "Log RAM content");
    log_ram_content(phys_to_virt(BASE_ADDR), INPUT_SIZE);
#endif
    
    entry = proc_create(PROCFS_NAME, 0660, NULL, &proc_operations);
    
    // Run inverted Neumann to remove 0101b bias
    tmp_puf = run_neumann(phys_to_virt(BASE_ADDR), INPUT_SIZE, &size, true);
    printk(KERN_NOTICE "Generated first PUF iteration with size %i\n", size);

    // Run Neumann to remove 1111b bias
    puf = run_neumann(tmp_puf, size, &puf_size, false);
    printk(KERN_NOTICE "Generated final PUF iteration with size %i\n", puf_size);

    kfree(tmp_puf);

    return 0;
}

#define LEFT_MASK 0x80 // leftmost bit
#define LEFT_PAIR 0xc0 // leftmost bit pair
#define RIGHT_PAIR 0x30 // second bit pair from the left

char * run_neumann(char *start_addr, int input_len, int *output_size, int inverted) {
    int i, bitpair, copied_bits, output_pos;
    unsigned char curr_pair;
    char *output_addr;

    // Expect reduction to quarter size
    *output_size = INPUT_SIZE / 4;
    output_addr = kmalloc(*output_size, GFP_KERNEL);

    output_pos = 0;
    copied_bits = 0;

    for(i = 0; i < input_len; i++) {

        for( bitpair = 0; bitpair < 4; bitpair++) {
            // Copy current byte, shift to current pair, mask to get most left pair
            curr_pair = *(start_addr + i);
            curr_pair = curr_pair << (bitpair * 2);
            curr_pair = curr_pair & LEFT_PAIR;


            // Actual Von Neumann Algorithm
            // Copy if not 00 or 11, discard otherwise
            if ( ( inverted && (curr_pair == 0xc0 || curr_pair == 0x00 ) ) || (!inverted && !(curr_pair == 0xc0 || curr_pair == 0x00 ) )) {
                *(output_addr + output_pos) = *(output_addr + output_pos) | (curr_pair >> 7); // Copy bit to the rightmost output byte
                
                // If 8 bits copied, move on to next output byte, otherwise shift one bit
                if(++copied_bits % 8 == 0) {
                    output_pos++;
                    
                    // If running out of output space, realloc
                    if(*output_size < (output_pos)) {
                        *output_size = output_pos + 24;
                        output_addr = krealloc(output_addr, *output_size, GFP_KERNEL);

                        if(output_addr == NULL) {
                            printk("Could not realloc space");
                            return 0;
                        }
                    }

                    *(output_addr+output_pos) = 0x00;
                } else {
                    *(output_addr + output_pos) = *(output_addr + output_pos) << 1;
                }
            }
        }
    }
    
    // Reallocate to discard unused output space
    output_addr = krealloc(output_addr, copied_bits / 8, GFP_KERNEL);
    puf_size = copied_bits / 8;

    return output_addr;
}

// Loop memory and log contents to kernel log
void log_ram_content(unsigned int * start, unsigned int length) {
    unsigned int offset;
    unsigned int *current_addr;
    unsigned int result;

    for( offset = 0; offset < length; offset++ ) {
           current_addr = ( start + offset );
           result = *( current_addr );                                          
           printk(KERN_NOTICE "0x%xx : 0x%x\n", (unsigned int) current_addr, result );
    }
}

void puf_cleanup(void) {
    printk(KERN_ALERT "Module Removed\n");
    proc_remove(entry); 
    kfree(puf);
    return;
}

MODULE_LICENSE("GPL");   
module_init(puf_init);
module_exit(puf_cleanup);

MODULE_AUTHOR("Soenke Huster, Fabian Hinz");
MODULE_DESCRIPTION("Creates a PUF after initialization out of DRAM and applying transformation for high entropy");

